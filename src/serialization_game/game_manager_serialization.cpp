#include "game_manager_serialization.h"

namespace serialization {

    [[nodiscard]] app::GameManager GameManagerRepr::Restore(const model::Game& game) const {
        // Создаем объект GameManager
        app::GameManager manager_rest(game);
        manager_rest.SetDogId(dog_id_);
        try {
        // Сначала восстанавливаем сессии
        RestoreMapsSessions(manager_rest, game);
        // Затем восстанавливаем игроков
        RestorePlayers(manager_rest);

        return manager_rest;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error during restoration: " + std::string(e.what()));
        }
    }

    void GameManagerRepr::RestoreMapsSessions(app::GameManager &manager, const model::Game &game) const {
        // Восстанавливаем основную карту с сессиями
        app::AllGameSessionsList all_sessions_list;
        for(const auto& [key, session] : all_sessions_list_) {
            
            all_sessions_list.emplace(key, std::make_shared<model::GameSession>(session.Restore(game)));
        }

        // Восстанавливаем карты с сессиями GameManager
        for(const auto& [key, session] : game_session_) {
            // Получаем восстановлеваемую сессию
            auto game_session = session.Restore(game);
            auto game_session_rest = all_sessions_list.find(game_session.GetGameSessionId());

            if(game_session_rest == all_sessions_list.end()){
                throw std::runtime_error("Session with ID "s + std::to_string(id_session_) + " not found"s 
                                            + "\nRecovery is not possible!"s);
            }

            // Добавляем в карты сессию
            manager.AddGameSessionInMaps(
                std::make_pair<app::SessionStatus, model::Map::Id>(key.GetSessionStatus(), key.GetMapId())
                , game_session_rest->first
                , game_session_rest->second
            );
        }
    }

    void GameManagerRepr::RestorePlayers(app::GameManager &manager) const {
        // Получаем карту с сессиями
        const app::AllGameSessionsList& map_session = manager.GetAllSessions();

        // Восстанавливаем все контейнеры с игроками
        for(const auto& player : players_) {
            // Восстанавливаем игрока
            app::PlayerPtr player_rest = std::make_shared<domain::Player>(player.Restore(map_session));
            // Восстанавливаем основную коллекцию игроков (players_)
            manager.AddPlayerInPlayerList(player_rest);
            // Восстанавливаем карту для поиска игроков по токену (token_to_player_)
            manager.AddTokenIndex(player_rest->GetToken(), player_rest); 
            // Восстанавливаем карту быстрого поиска игроков по id пса и id карты
            manager.AddDogMapIndexList(std::make_pair(player_rest->GetDogId(), app::Stoi(*player_rest->GetMapId()))
                                    , player_rest);
            // Восстанавливаем для быстрого возврата игроков в сессии
            manager.AddListPlayerInSessionList(player_rest->GetCurrentSession()->GetGameSessionId(), player_rest);
        }
    }

} // namespace serialization