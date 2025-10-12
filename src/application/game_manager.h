#pragma once 

#include <deque>
#include <optional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

#include "../models/game.h"
#include "../domain_models/player.h"
#include "utils.h"

namespace app {

    using std::pair;
    class Players;

class Player;

    namespace net = boost::asio;
    using DogPtr = std::shared_ptr<model::Dog>;
    using SessionPtr = std::shared_ptr<model::GameSession>;
    using PlayerPtr = std::shared_ptr<domain::Player>;
    using PlayerList = std::deque<PlayerPtr>;
    using ConstPlayerList = const std::deque<PlayerPtr>;
    using ListPlayerInSession = std::unordered_map<size_t // session_id
                                                    , PlayerList>;
    using DogsList = std::unordered_map<size_t // dog_id
                                            ,DogPtr>;
    using ConstDogsList = const std::unordered_map<size_t // dog_id
                                                    ,DogPtr>;
    using ConteinerByObj = std::vector<model::LostObject>;
    using GameSessionsType = std::unordered_map<std::pair <SessionStatus, model::Map::Id>
                                    , std::shared_ptr<model::GameSession>
                                    , GameSessionsHasher>;

    namespace detail{
        
        // Хешер, использующий только токен для основного списка игроков
        struct PlayerTokenHasher {
            std::size_t operator()(const PlayerPtr& player) const noexcept {
                return std::hash<std::string>{}(std::string{player->GetToken()});
            }
        };

        // Компаратор на основе токена
        struct PlayerTokenEqual {
            bool operator()(const PlayerPtr lhs, const PlayerPtr rhs) const noexcept {
                return lhs->GetToken() == rhs->GetToken();
            }
        };
        
        // Хешер, использующий id пса и id карты
        struct DogIdAndMapIdHasher {
            std::size_t operator()(const std::pair<size_t, size_t>& p) const {
                // Используем более надежный способ комбинирования хеш-значений
                std::size_t hash1 = std::hash<size_t>{}(p.first);
                std::size_t hash2 = std::hash<size_t>{}(p.second);
                
                // Комбинирование хеш-значений с использованием хорошего сдвига
                return hash1 ^ (hash2 << 16) ^ (hash2 >> 16);
            }
        };
    } // namespace detail

    using DogMapKey = std::pair<size_t,   // dog_id
                                size_t>;  // map_id
    using DogMapIndex = std::unordered_map<DogMapKey, PlayerPtr, detail::DogIdAndMapIdHasher>;
    using TokenIndex = std::unordered_map<std::string_view // token
                                                , PlayerPtr>;
    using PlayersList = std::unordered_set<PlayerPtr, detail::PlayerTokenHasher,detail::PlayerTokenEqual>;
    using AllGameSessionsList = std::unordered_map<size_t    // id сессии
                                        , std::shared_ptr<model::GameSession>>;

    class GameManager{
    public:
        explicit GameManager(const model::Game& game, bool is_restore = false)
            : game_(game)
            , is_restore_(is_restore) {
    }
        ~GameManager() = default;
        GameManager(const GameManager&) = default;
        GameManager& operator=(const GameManager&) = default;
        GameManager(GameManager&&) = default;
        GameManager& operator=(GameManager&& other) noexcept {
            if(this == &other) {
                return *this;
            }

            is_restore_ = other.is_restore_;
            players_ = std::move(other.players_);
            dog_id_and_map_id_to_players_ = std::move(other.dog_id_and_map_id_to_players_);
            token_to_player_ = std::move(other.token_to_player_);
            id_session_to_players_ = std::move(other.id_session_to_players_);
            game_session_ = std::move(other.game_session_);
            all_sessions_list_ = std::move(other.all_sessions_list_);
            id_session_ = other.id_session_;
            dog_id_ = other.dog_id_;

            return *this;
        }

        void RemovePlayer(std::string_view token);

        domain::Player* FindPlayerByToken(std::string_view token);
        domain::Player* FindPlayerByDogAndMapId(size_t dog_id, size_t map_id);

        std::pair<PlayerList,bool> GetPlayerBySessionList(const std::shared_ptr<domain::Player>& player);
        std::pair<PlayerList,bool> GetPlayerBySessionList(size_t session_id);
        std::optional<PlayerPtr> GetPlayerByTokenList(const std::string& token);
        const PlayersList& GetPlayers() const noexcept;
        GameSessionsType& GetGameSessions() noexcept;
        const GameSessionsType& GetGameSessions() const noexcept;
        const DogMapIndex& GetDogMapIndex() const noexcept;
        const TokenIndex& GetTokenIndex() const noexcept;
        const ListPlayerInSession& GetListPlayerInSession() const noexcept;
        const AllGameSessionsList& GetAllSessions() const noexcept;
        size_t GetCurrentValueSessionId() const noexcept;
        size_t GetDogId() const noexcept;
        PlayerPtr GetPlayer(std::string_view token);

        PlayerPtr AddPlayer(const std::string& id_map, const std::string& name_dog);
        void AddPlayerInPlayerList(PlayerPtr player);
        void AddDogMapIndexList(const DogMapKey& key, PlayerPtr player);
        void AddTokenIndex(std::string_view token, PlayerPtr player);
        void AddListPlayerInSessionList(size_t session_id, PlayerPtr player);
        void AddGameSessionInMaps(std::pair<SessionStatus, model::Map::Id> key
                                                , size_t session_id, std::shared_ptr<model::GameSession> session);

        void SetDogId(size_t dog_id);

        bool operator==(const GameManager& other) const;

        void RemovePlayer(PlayerPtr player);

    private:
        // Добавляем сессиию
        SessionPtr AddSession(const std::string& id_map);
        // Добавляем пса на карту
        pair<SessionPtr,DogPtr> AddDogToGame(const std::string& id_map, const std::string& name_dog);
        // Добавляем сессию в карту сессиий по типу (свободна для добавления игрока или нет) и id карты
        void AddGameSessionsType(std::pair <SessionStatus, model::Map::Id> key, std::shared_ptr <model::GameSession>);
        // Добавляем сессию в основной набор сессий
        std::shared_ptr<model::GameSession> AddAllGameSessionsList(size_t session_id, std::shared_ptr<model::GameSession> session);

    private:
        const model::Game& game_;
        bool is_restore_ = false;
        // Основной набор игроков с уникальной идентификацией по токену
        PlayersList players_;
        // Индекс для быстрого поиска игроков по комбинации ID собаки и карты
        DogMapIndex dog_id_and_map_id_to_players_;
        // Индекс для быстрого поиска игроков по токену
        TokenIndex token_to_player_;
        // Индекс для быстрого возврата игроков в сессии
        ListPlayerInSession id_session_to_players_;
        // Карта сессиий по типу (свободна для добавления игрока или нет) и id карты
        GameSessionsType game_session_;
        // Основную карту сессий
        AllGameSessionsList all_sessions_list_;
        size_t id_session_ = 0;
        size_t dog_id_ = 0;
    };

} // app
