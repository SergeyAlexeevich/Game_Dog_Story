#pragma once

//#define THREAD_POOL // есть мысли сделать в пуле

#include <functional>
#include <chrono>
#include <future>
#include <optional>
#include <mutex>

#include <boost/json.hpp>
#include <unordered_map>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/http.hpp>
#include <boost/signals2.hpp>

#ifdef THREAD_POOL
    #include <boost/asio/thread_pool.hpp>
#endif

#include "../models/geometry_primitives.h"
#include "../models/game.h"
#include "../models/loot_generator.h"
#include "../time_management/ticker.h"
#include "../physics/collision_detector.h"
#include "game_manager.h"
#include "../domain_models/player.h"
#include "../database/use_cases_impl.h"
#include "../database/database_connection_settings.h"
#include "../database/postgres.h"

// Предварительное объявление RestoreManager
namespace data_persistence {
    class BackupRestoreManager;
}

namespace app{

    // Коэфициент перевода миллесекунд в секунды
    const double MILLISECONDS_IN_SECOND = 1000.0;

    using namespace std::literals;
    namespace json = boost::json;
    namespace net = boost::asio;
    namespace http = boost::beast::http;
    namespace sig = boost::signals2;
    using milliseconds = std::chrono::milliseconds;
    using std::pair;
    using std::optional;
    using TimeType = std::optional<double>;
    using StatusMessage = pair<http::status         // Статус-код
                            ,std::string>;          // body

    class Application{
    public:
    using AppStrand = net::strand<net::io_context::executor_type>;
    using SerializeSignal = sig::signal<void(const app::GameManager& manager, double current_time)>;
    using RestoreSignal = sig::signal<void(app::GameManager& manager, const model::Game& game)>;
    using SavedGame = std::function<void(GameManager&)>;
    
        explicit Application(net::io_context& ioc, AppStrand strand, model::Game&& game, TimeType auto_update_interval
                                , TimeType save_interval, const db_conn_settings::DbConnectrioSettings& db_settings) 
                : ioc_{ioc}
                , strand_{std::make_shared<AppStrand>(strand)}
                , game_{std::move(game)}
                , auto_update_interval_{auto_update_interval}
                , game_manager_{game_}
                , loot_generator_{loot_gen::LootGenerator{
                    std::chrono::milliseconds{static_cast<uint64_t>(game_.GetGameSetting().default_period * MILLISECONDS_IN_SECOND)}
                    , game_.GetGameSetting().default_probability}}
                , save_interval_{save_interval}
                , data_base_{db_settings}
                , use_cases_{data_base_.GetPlayerRecords()} {
                // Если интервал задан, то включаем автоматическое обновление времени
                if(auto_update_interval_){
                    ticker_ = std::make_shared<time_m::Ticker>(
                        *strand_
                        ,std::chrono::milliseconds{static_cast<uint64_t>(auto_update_interval_.value())}
                        , [this](const std::chrono::milliseconds& time_period) mutable{
                            double delta_time = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(time_period).count();
                            this->UpdateGameSessions(delta_time);
                        }
                    );

                    ticker_->Start();
                }
        }
        Application(const Application& other) = delete;
        Application(Application&& other) = delete;
        Application& operator = (const Application& other) = delete;
        Application& operator = (Application&& other) = delete;
        ~Application() = default;

        StatusMessage JoinGame(const std::string& dog_name, const std::string& map_id);

        const model::Game& GetGame()const noexcept;
        StatusMessage GetPlayerList(const std::string& token);
        StatusMessage GetGameState(const std::string& token);
        StatusMessage GetRecords(std::optional<size_t> offset, std::optional<size_t> limit);
        SerializeSignal& GetSerializeSignal() noexcept;
        RestoreSignal& GetRestoreSignal() noexcept;

        StatusMessage CharacterMoveManagement(const std::string& token, const std::string& move_character);

        StatusMessage UpdateGameManager(TimeType manual_update_interval_ = std::nullopt);

        void SetRestoreGameManager(GameManager&& manager_rest);
        void SetSaveNeeded(bool auto_save_needed);
        void SetSavedGame(const SavedGame& save);

        void EmitSerializeSignal();
        void EmitRestoreSignal();

        GameManager& GetManager() noexcept{
            return game_manager_;
        }

    private:
        void AddLostObject(double delta_time, model::GameSession& session);
        void ControlPlayersInGame(const std::vector<std::string>& tokens);
        bool IsRemovePlayer(PlayerPtr player, TimeType time, model::Velocity start_velocity);
        StatusMessage UpdateGameSessions(double delta_time);
        void UpdatePlayerPositions(double delta_time, model::GameSession& session);

    private:
    SavedGame save_game_;
    net::io_context& ioc_;
    std::shared_ptr<AppStrand> strand_;
    model::Game game_;
    TimeType auto_update_interval_;
    GameManager game_manager_;
    std::shared_ptr<time_m::Ticker> ticker_;
    std::optional<loot_gen::LootGenerator> loot_generator_;
    TimeType save_interval_;
    bool auto_save_needed_ = false;
    postgres::Database data_base_;
    db_storage::UseCasesImpl use_cases_;
    // Создаем сигнал для сериализации
    SerializeSignal serialize_signal_;
    // Создаем сигнал для восстановления
    RestoreSignal restore_signal_;
    };

} // app