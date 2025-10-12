#pragma once

#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include "../application/game_manager.h"
#include "player_serialization.h"
#include "game_session_serialization.h"

namespace serialization {
    
    using namespace std::literals;

    struct SessionReprKey{
        SessionReprKey() = default;
        SessionReprKey(app::SessionStatus session_status, model::Map::Id map_id)
        : session_status{static_cast<size_t>(session_status)}
        , map_id{*map_id} {

        }

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& session_status;
            ar& map_id;
        }

        app::SessionStatus GetSessionStatus() const noexcept{
            return static_cast<app::SessionStatus>(session_status);
        }

        model::Map::Id GetMapId() const noexcept{
            return model::Map::Id{map_id};
        }

        bool operator==(const SessionReprKey& other) const {
            return session_status == other.session_status && map_id == other.map_id;
        }

        bool operator!=(const SessionReprKey& other) const {
            return !(*this == other);
        }

        bool operator<(const SessionReprKey& other) const {
            if (session_status != other.session_status) {
                return session_status < other.session_status;
            }
            
            return map_id < other.map_id;
        }

        size_t session_status = 0;
        std::string map_id;
    };

    struct GameSessionsHasher{ // по статусу и id карты
        size_t operator()(const SessionReprKey& value)const{
            // Получаем числовое значение статуса сессии
            const size_t status =value.session_status;
            // Получаем идентификатор карты
            size_t map_id_number = 0;

            for(auto ch : value.map_id){
                map_id_number += static_cast<size_t>(ch);
            }
                
            // Комбинируем хеши с использованием XOR и сдвигов
            return (map_id_number * 17) ^ (status * 31) ^ (map_id_number << 5);
        }
    };

    using PlayersList = std::deque<PlayerRepr>;
    using DogMapIndex = std::unordered_map<app::DogMapKey, PlayerRepr, app::detail::DogIdAndMapIdHasher>;
    using TokenIndex = std::unordered_map<std::string_view // token
                                                , PlayerRepr>;
    using ListPlayerInSession = std::unordered_map<size_t, PlayersList>;
    using AllGameSessionsList = std::unordered_map<size_t    // id сессии
                                        , GameSessionRepr>;
    using GameSessionsType = std::unordered_map<SessionReprKey
                                    , GameSessionRepr
                                    , GameSessionsHasher>;

    // GameManagerRepr (GameManagerRepresentation) - сериализованное представление класса GameManager
    class GameManagerRepr {
    public:
        GameManagerRepr() = default;
        explicit GameManagerRepr(const app::GameManager& manager) 
        : id_session_{manager.GetCurrentValueSessionId()}
        , dog_id_{manager.GetDogId()} {
            // Создаем лист с всеми игроками
            auto all_players_list = manager.GetPlayers();
            for(const auto& player : all_players_list){
                players_.emplace_back(PlayerRepr(*player));
            }
            
            // Создаем карту сессиий по типу
            auto game_session = manager.GetGameSessions();
            for(const auto& [key, shared_session] : game_session) {
                auto key_for_add = SessionReprKey(key.first, key.second);
                game_session_.emplace(key_for_add, GameSessionRepr(*shared_session));
            }

            // Создаем основной набор сессий
            auto all_session_list = manager.GetAllSessions();
            for(const auto& [key, session] : all_session_list){
                all_sessions_list_.emplace(key, GameSessionRepr(*session));
            }
        }

        // Восстанавливаем данные GameManager
        [[nodiscard]] app::GameManager Restore(const model::Game& game) const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

    private:
        void RestoreMapsSessions(app::GameManager& manager, const model::Game& game) const;

        void RestorePlayers(app::GameManager& manager) const;

    private:
        size_t id_session_;
        size_t dog_id_;
        // Основной набор игроков с уникальной идентификацией по токену
        PlayersList players_;
        // Карта сессиий по типу (свободна для добавления игрока или нет) и id карты
        GameSessionsType game_session_;
        // Основной набор сессий
        AllGameSessionsList all_sessions_list_;
    };

    template <typename Archive>
    inline void GameManagerRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_session_;
        ar& dog_id_;
        ar& players_;
        ar& game_session_;
        ar& all_sessions_list_;
    }

} // namespace serialization