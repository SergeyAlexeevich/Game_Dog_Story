#include "game_manager.h"

#include <boost/json.hpp>

#include "../logging/logger.h"

namespace app {

    using namespace std::literals;

    void GameManager::RemovePlayer(std::string_view token) {
        if(auto token_iter = token_to_player_.find(token); token_iter != token_to_player_.end()){
            PlayerPtr removed_player = token_iter->second;
            token_to_player_.erase(token);

#ifdef DEBAGER
  logger::LogEntryToConsole(boost::json::object{}, "Player successfully remove to the indexes token_to_player_"s);
#endif

            size_t dog_id = removed_player->GetDogId();
            size_t map_id = std::stoull(*removed_player->GetMapId());

            dog_id_and_map_id_to_players_.erase(std::pair{dog_id, map_id});

#ifdef DEBAGER
  logger::LogEntryToConsole(boost::json::object{}, 
    "Player successfully remove to the indexes dog_id_and_map_id_to_players_"s);
#endif

            players_.erase(removed_player);
            
#ifdef DEBAGER
  logger::LogEntryToConsole(boost::json::object{}, "Player successfully remove to the indexes players_"s);
#endif
        }
    }

    domain::Player* GameManager::FindPlayerByToken(std::string_view token) {
        if(auto iter_found_player = token_to_player_.find(token); iter_found_player != token_to_player_.end()){
            return iter_found_player->second.get();
        }

        return nullptr;
    }

    domain::Player *GameManager::FindPlayerByDogAndMapId(size_t dog_id, size_t map_id) {
        if(auto found_player = dog_id_and_map_id_to_players_.find(std::pair{dog_id,map_id}); 
                found_player != dog_id_and_map_id_to_players_.end()){
            return found_player->second.get();
        }

        return nullptr;
    }

    std::pair<PlayerList,bool> GameManager::GetPlayerBySessionList(const std::shared_ptr<domain::Player>& player) {
       auto iter = id_session_to_players_.find(player->GetGameSessionId());

       return iter != id_session_to_players_.end() ? pair{iter->second, true} : pair{PlayerList{}, false};
    }

    std::pair<PlayerList,bool> GameManager::GetPlayerBySessionList(size_t session_id) {
        auto iter = id_session_to_players_.find(session_id);

        return iter != id_session_to_players_.end() ? pair{iter->second, true} : pair{PlayerList{}, false};
    }

    std::optional<PlayerPtr> GameManager::GetPlayerByTokenList(const std::string &token) {
        auto iter_found_player = token_to_player_.find(token);

        if(iter_found_player == token_to_player_.end()){
            return std::nullopt;
        }

        return std::optional{iter_found_player->second};
    }

    const PlayersList& GameManager::GetPlayers() const noexcept {
        return players_;
    }

    const DogMapIndex& GameManager::GetDogMapIndex() const noexcept {
        return dog_id_and_map_id_to_players_;
    }

    const TokenIndex& GameManager::GetTokenIndex() const noexcept {
        return token_to_player_;
    }

    const ListPlayerInSession& GameManager::GetListPlayerInSession() const noexcept {
        return id_session_to_players_;
    }

    GameSessionsType& GameManager::GetGameSessions() noexcept  {
        return game_session_;
    }

    const GameSessionsType &GameManager::GetGameSessions() const noexcept {
        return game_session_;
    }

    const AllGameSessionsList& GameManager::GetAllSessions() const noexcept {
        return all_sessions_list_;
    }

    size_t GameManager::GetCurrentValueSessionId() const noexcept {
        return id_session_;
    }

    size_t GameManager::GetDogId() const noexcept {
        return dog_id_;
    }

    PlayerPtr GameManager::GetPlayer(std::string_view token) {
        if(auto iter = token_to_player_.find(token); iter != token_to_player_.end()) {
            return iter->second;
        }
        
        return nullptr;
    }

    void GameManager::AddPlayerInPlayerList(PlayerPtr player) {
        players_.emplace(player);
    }

    void GameManager::AddDogMapIndexList(const DogMapKey& key, PlayerPtr player) {
        dog_id_and_map_id_to_players_.emplace(key, player);
    }

    void GameManager::AddTokenIndex(std::string_view token, PlayerPtr player) {
        token_to_player_.emplace(token, player);
    }

    void GameManager::AddListPlayerInSessionList(size_t session_id, PlayerPtr player) {
        // Получаем итератор на искомую сессию
        auto it_id_session_to_players = id_session_to_players_.find(session_id);

        // Проверяем валидность итератора искомой сессии. Если итератор не валиден, то создаем новую пару
        if(it_id_session_to_players == id_session_to_players_.end()){
            id_session_to_players_.emplace(session_id, std::deque<PlayerPtr>{player});
            return;
        }

        // При валидном итераторе добавляем игрока в список игроков искомой сессии
        it_id_session_to_players->second.emplace_back(player);
    }

    void GameManager::AddGameSessionInMaps(std::pair<SessionStatus, model::Map::Id> key
                                                    , size_t session_id, std::shared_ptr <model::GameSession> session) {
            auto temp_shared_session = AddAllGameSessionsList(session_id, session);
            AddGameSessionsType(key, temp_shared_session);
    }

    void GameManager::SetDogId(size_t dog_id) {
        dog_id_ = dog_id;
    }

    bool GameManager::operator==(const GameManager& other) const {
        return is_restore_ == other.is_restore_
                && players_ == other.players_
                && token_to_player_ == other.token_to_player_
                && id_session_to_players_ == other.id_session_to_players_
                && all_sessions_list_ == other.all_sessions_list_;
    }

    void GameManager::RemovePlayer(PlayerPtr player) {
        auto player_to_remote = player;
        // Получаем текущую сессию
        auto current_session = player_to_remote->GetCurrentSession();
        // Удаляем пса из сессии
        current_session->RemoteDog(player_to_remote->GetDogId());
        // Удаляем игрока из карты быстрого поиска по id пса и id карты
        dog_id_and_map_id_to_players_.erase(std::pair{player_to_remote->GetDogId(), app::Stoi(*current_session->GetMapId())});
        // Удаляем игрока из карты быстрого поиска по токену
        token_to_player_.erase(player_to_remote->GetToken());
        // Удаляем из карты быстрого возврата игроков в сессии
        auto& players_list_in_session = id_session_to_players_.at(current_session->GetGameSessionId());

        // Ищем игрока в сесии
        for(auto it = players_list_in_session.begin(); it != players_list_in_session.end(); ++it){
            if(player_to_remote->GetToken() == (*it)->GetToken()) {
                players_list_in_session.erase(it);
                break;
            }
        }
        // Удаляем из основной коллекции игроков
        players_.erase(player_to_remote);
    }

    void GameManager::AddGameSessionsType(std::pair<SessionStatus, model::Map::Id> key, std::shared_ptr <model::GameSession> session) {
        game_session_.emplace(key, session);
    }

    std::shared_ptr<model::GameSession> GameManager::AddAllGameSessionsList(size_t session_id
                                                                                    , std::shared_ptr <model::GameSession> session) {
        return all_sessions_list_.emplace(session_id, session).first->second;
    }

    PlayerPtr GameManager::AddPlayer(const std::string &id_map, const std::string &name_dog) {
        // создаем игровую сессиию с псом на выбранной карте
        std::pair<SessionPtr,DogPtr> result = AddDogToGame(id_map, name_dog);

        if(!result.first || !result.second){
            return nullptr;
        }

        // добавляем игрока в основной набор
        auto pair = players_.emplace(std::make_shared<domain::Player>(result.first, result.second, result.first->GetMap().GetBagCapacity()));

#ifdef DEBAGER
    logger::LogEntryToConsole(boost::json::object{}, "Player successfully added to the indexes players_"s);
#endif

        PlayerPtr added_player = *pair.first;

        // добавляем игрока в индекс для быстрого поиска игроков по комбинации ID собаки и карты
        token_to_player_.emplace(added_player->GetToken(), added_player);

#ifdef DEBAGER
    logger::LogEntryToConsole(boost::json::object{}, "Player successfully added to the indexes token_to_player_"s);
#endif
            // добавляем игрока в индекс для быстрого поиска игроков по токену
            dog_id_and_map_id_to_players_.emplace(
                std::pair{added_player->GetDogId(), Stoi(id_map)} , added_player
            );

#ifdef DEBAGER
    logger::LogEntryToConsole(boost::json::object{}, "Player successfully added to the indexes dog_id_and_map_id_to_players_"s);
#endif

            auto id_session = added_player->GetGameSessionId();

            AddListPlayerInSessionList(id_session, added_player);
            
        return added_player;
    }

    SessionPtr GameManager::AddSession(const std::string &id_map)
    {
        if(auto found_map = game_.FindMap(model::Map::Id{id_map}); found_map){
            auto id = found_map->GetId();
            size_t id_session = id_session_++;
            auto added_session = all_sessions_list_.emplace(id_session 
                                            , std::make_shared<model::GameSession>(*found_map, id_session)
            );

            if (!added_session.second) {
                throw std::runtime_error("Failed to add session in all_sessions_list_");
            }

            auto result = game_session_.emplace(
                                            std::pair{FREE, id}
                                            , added_session.first->second
            );

            if (!result.second) {
                throw std::runtime_error("Failed to add session in game_session_");
            }

            return result.first->second;
        }
        
        return nullptr;
    }

    pair<SessionPtr,DogPtr> GameManager::AddDogToGame(const std::string& id_map, const std::string& name_dog) {
        if(game_session_.empty()){
            AddSession(id_map);
        }
        
        size_t dog_id = dog_id_++;

        if(auto found_session = game_session_.find(pair{FREE, model::Map::Id{id_map}}); found_session != game_session_.end()){
            if(auto added_dog = found_session->second->AddDogOnMap(name_dog, dog_id)) {
                return pair{found_session->second, added_dog};   
            }
        }

        SessionPtr new_session = AddSession(id_map);

        if(!new_session){
            return pair{nullptr, nullptr};
        }

        DogPtr current_dog = new_session->AddDogOnMap(name_dog, dog_id);
        
        return pair{new_session,current_dog};
    }

} // app