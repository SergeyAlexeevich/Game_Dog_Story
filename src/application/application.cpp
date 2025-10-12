#include "application.h"

#include <stdexcept>
#include <iostream>
#include <future>

#include "../models/geometry_primitives.h"
#include "utils.h"
#include "collision_manager.h"
#include "../logging/logger.h"
#include "../database/database_invariants.h"

namespace app {

    using NearestPointType = std::pair<int,         // id карты
                                model::Position>;  // координаты точки

        StatusMessage Application::JoinGame(const std::string &dog_name, const std::string &map_id) {
            StatusMessage result;

            net::dispatch(*strand_,[this, &result, dog_name, map_id](){
                if(!game_.FindMap(model::Map::Id{map_id})){
                    json::object message{
                    {"code"s, "mapNotFound"s},
                    {"message"s, "Map not found"s}  
                    };

                    result = pair{http::status::not_found, json::serialize(message)}; // 404
                    return;
                }

                // проверяем полученное имя на пустоту
                if(dog_name.empty() || std::all_of( // проверяем не состоит ли имя только из пробельных символов
                                                dog_name.begin()
                                                , dog_name.end()
                                                , [](char c) {
                                                        return std::isspace(static_cast<unsigned char>(c));
                                                }
                                            )
                                        ){

                    json::object message{
                    {"code"s, "invalidArgument"s},
                    {"message"s, "Invalid name"s}  
                    }; 

                    result = pair{http::status::bad_request, json::serialize(message)}; //400
                    return;
                }

                auto new_player = game_manager_.AddPlayer(map_id, dog_name);

                if(!new_player){
                    json::object message{
                    {"code"s, "mapNotFound"s},
                    {"message"s, "Map not found"s}  
                    };

                    result = pair{http::status::not_found, json::serialize(message)}; //404
                    return;
                }

                result = pair{http::status::ok, json::serialize( // 200
                                    json::object{
                                            {"authToken"s, new_player->GetToken()},
                                            {"playerId"s, new_player->GetDogId()}
                                    }

                                )
                        };
            });
        return result;
    }

        const model::Game& Application::GetGame() const noexcept {            
            return game_;
        }

        StatusMessage Application::GetPlayerList(const std::string& token) {
            auto player_in_session = game_manager_.FindPlayerByToken(token);

            if(!player_in_session){
                json::object message{
                  {"code"s, "unknownToken"s},
                  {"message"s, "Player token has not been found"s}  
                };

                return pair{http::status::unauthorized, json::serialize(message)}; // 401
            }

            auto player_list_in_session = player_in_session->GetDogListInCurrentSession();
            json::object result;

            for(const auto& [key, value] : player_list_in_session){
                json::object dog_data{
                    {"name"s, value->GetName() }
                };

                result.emplace(std::to_string(key),dog_data);
            }

            return {http::status::ok, json::serialize(result)}; //200
        }

        StatusMessage Application::GetGameState(const std::string& token) {
            // Ищем игрока в игре по токену
            auto player_in_session = game_manager_.FindPlayerByToken(token); 

            // Проверяем что игрок существует в сессии
            if(!player_in_session){
                json::object message{
                  {"code"s, "unknownToken"s},
                  {"message"s, "Player token has not been found"s}  
                };

                return pair{http::status::unauthorized, json::serialize(message)}; //401
            }

            // Создаем объект для записи данных
            json::object data;
            // Добавляем информацию об игроках
            // Получаем список игроков в сессии (проверка существования такого списка не требуется, т.к. выше была 
            // проверка на существование такого токена в игре. А раз токен свществует, значит список хотяб из одного
            // игрока существует)
            auto players_in_session = game_manager_.GetPlayerBySessionList(player_in_session->GetGameSessionId()).first;

            for(const auto& player : players_in_session){
                json::object player_data{
                    {"pos"s, json::array{ 
                        player->GetDog()->GetCurrentPosition().x, player->GetDog()->GetCurrentPosition().y }
                    }
                    ,{"speed"s, json::array{ 
                        player->GetDog()->GetVelocity().vx, player->GetDog()->GetVelocity().vy}
                    }
                    ,{"dir"s, player->GetDog()->GetDirectionToString()}
                };

                // Добавляем информацию о рюкзаке и его содержимом
                json::array items;

                for(const auto& obj : player->GetObjInBag()){
                    items.emplace_back(
                        json::object{
                            {"id"s, obj.GetId()}
                            , {"type"s, obj.GetType()}
                        }
                    );
                }

                player_data.emplace("bag"s, items);
                player_data.emplace("score"s, player->GetScore());
                data.emplace(std::to_string(player->GetDogId()),player_data);
            }

            // Создаем результатирующий объект
            json::object result  = json::object{
                                        {"players"s,data}
                                    };

            // Очищаем объект с данными
            data.clear();
            // Добавляем информацию о потерянных предметах
            auto lost_objects = player_in_session->GetCurrentSession()->GetLostObjects();

            for(const auto& object : lost_objects){
                json::object lost_obj_data{
                    {"type"s,object.second.GetType()}
                    , {"pos"s, json::array{object.second.GetPosition().x, object.second.GetPosition().y} }
                };

                data.emplace(std::to_string(object.second.GetId()), lost_obj_data);
            }

            result.emplace("lostObjects"s, data);
            return {http::status::ok, json::serialize(result)}; //200
        }

        StatusMessage Application::GetRecords(std::optional<size_t> offset, std::optional<size_t> limit) {
            size_t start = db_invariants::DEFAULT_OFFSET;
            size_t number_row = db_invariants::DEFAULT_LIMIT;

            if(offset) {
                start = offset.value();
            }

            if(limit) {
                number_row = limit.value();
            }

            std::optional<std::vector<domain::PlayerRecord>> players_table = use_cases_.GetRecordsTable(start, number_row);

            json::array players_records_arr;
            for(const auto& player_record : players_table.value()) {
                players_records_arr.emplace_back(
                    json::object{
                        {"name"sv, player_record.GetName()}
                        , {"score"sv, player_record.GetScore()}
                        , {"playTime"sv, player_record.GetPlayTime()}
                    }
                );
            }

            return {http::status::ok, json::serialize(players_records_arr)};
        }

        StatusMessage Application::CharacterMoveManagement(const std::string &token, const std::string &move_character) {
            StatusMessage result;

            net::dispatch(*strand_,[this,&result,token, move_character](){
                auto player_in_session = game_manager_.FindPlayerByToken(token);

                // Проверяем наличие игрока в сессии
                if(!player_in_session){
                    json::object message{
                    {"code"s, "unknownToken"s},
                    {"message"s, "Player token has not been found"s}  
                    };

                    result = pair{http::status::unauthorized,json::serialize(message)}; //401
                    return;
                }

                auto move_charac = model::STRING_TO_DIRECTION.find(move_character);

                // Проверяем валидность направления движения
                if(move_charac == model::STRING_TO_DIRECTION.end()){
                    json::object message{
                    {"code"s, "unkninvalidArgumentownToken"s},
                    {"message"s, "Failed to parse action"s}  
                    };

                    result = pair{http::status::bad_request,json::serialize(message)}; // 400
                    return;
                }

                // Получаем скорость на карте
                auto speed_on_map = player_in_session->GetVelocityOnMap();

                // Проверяем направление
                if(move_charac->second == model::Direction::NONE){
                    // делаем сброс скорости, если персонаж стоит на месте Direction::NONE
                    player_in_session->GetDog()->ResetVelocity();
                } else {
                    // Устанавливаем направление
                    player_in_session->GetDog()->SetDirection(move_charac->second);
                    // Устанавливаем скорость
                    player_in_session->GetDog()->SetVelocity(speed_on_map);
                }

                result = {http::status::ok, json::serialize(json::object{})}; // 200
            });
            return result;
        }

        StatusMessage Application::UpdateGameManager(TimeType manual_update_interval_) {
            // Если запущен автотаймер и выдался запрос на ручное изменение времени
            if(auto_update_interval_ && manual_update_interval_){
                json::object result  = json::object{
                            {"code"s, "badRequest"s},
                            {"message"s, "Invalid endpoint"s}  
                        };

                return {http::status::bad_request, json::serialize(result)}; // 400
            }

            // Если при ручном режиме задания периода течения времени не задан период
            if(!manual_update_interval_) {
                throw std::logic_error("error: no time interval is specified when using the manual time interval change mode"s);
            }

            return UpdateGameSessions(manual_update_interval_.value());
        }

        void Application::SetRestoreGameManager(GameManager&& manager_rest) {
            game_manager_ = std::move(manager_rest);
        }

        void Application::SetSaveNeeded(bool auto_save_needed) {
            auto_save_needed_ = auto_save_needed;
        }

        void Application::SetSavedGame(const SavedGame& save) {
            save_game_ = save;
        }

        bool Application::IsRemovePlayer(PlayerPtr player, TimeType time, model::Velocity start_velocity) {
            // Обновляем время игрока
            player->CorrectTimeInGame(time.value());

            // Если игрок неактивен
             if(start_velocity == model::Velocity{0,0}) {
                // Передаем новое значение периода
                player->SetInactivity(time.value());
                // Получаем время в течении которого игрок был не активен
                auto time_inactivity_player = player->GetInactivityTime();
                
                // Получаем время в течении которого допустимо отсутствие активности игрока
                auto inactivity_time_out = game_.GetGameSetting().dog_retirement_time;

                // Если время в течении которого игрок был неактивен превышает допустимое время 
                //  нахождения в игре в неактивном состоянии
                if(std::chrono::duration<double, std::milli>(time_inactivity_player.value()).count() >= inactivity_time_out) {
                    return true;
                }

                return false;
            }

            // Устанавливаем состояние игрока из неактивного в активное
            player->SetActivity();

            return false;
        }

        void Application::EmitSerializeSignal() {
            save_game_(game_manager_);
        }

        void Application::EmitRestoreSignal() {
            restore_signal_(game_manager_, game_);
        }

        Application::SerializeSignal& Application::GetSerializeSignal() noexcept {
            return serialize_signal_;
        }

        Application::RestoreSignal& Application::GetRestoreSignal() noexcept {
            return restore_signal_;
        }

        StatusMessage Application::UpdateGameSessions(double delta_time) {
            StatusMessage result;
            net::dispatch(*strand_, [this, &result, delta_time]() {
                // Создаем обработчик коллизий (столкновений)
                CollisionManager collision_manager(game_, game_manager_);

                // Перебираем все запущенные сессии
                for(auto& [_, session] : game_manager_.GetAllSessions()) {
                    // Добавляем предметы на карту сессии
                    AddLostObject(delta_time, *session);
                    // Обновляем позицию игрока
                    UpdatePlayerPositions(delta_time, *session);
                    // Запускаем обработчик коллизий
                    collision_manager.HandlerCollision(*session);

                } // for(auto& [_, session] : game_.GetAllGameSessions())

                // Если установлен флаг необходимости авто сохранения
                if(auto_save_needed_) {
                    EmitSerializeSignal();
                }
                result = {http::status::ok, json::serialize(json::object{})}; // 200
            });
            return result;
        }

        void Application::UpdatePlayerPositions(double delta_time, model::GameSession &session) {
            // Создаем вектор с токенами игроков, которых необходимо удалить из-за превышения допустимого времени неактивности
            std::vector<std::string> token_player_for_remove;
            // Расчет перемещения в текущей карте
            for(auto [_, dog] : session.GetDogsList()) {
                // Получаем текущую скорость игрока
                auto start_velocity = dog->GetVelocity();
                // Получаем направление движениия
                model::Direction current_derect = dog->GetDirection();
                // Получаем id карты для поиска игрока
                size_t map_id = Stoi(*session.GetMapId());
                // Ищем игрока
                auto player = game_manager_.FindPlayerByDogAndMapId(static_cast<size_t>(dog->GetId()), map_id);
                // Получаем текущую позицию
                model::Position current_pos = dog->GetCurrentPosition();
                // Получаем время в секундах( перемов из миллисекунд(дельту) в секунды)
                double time = delta_time/1000.0;
                // Вычисляем новую позицию
                model::Position new_pos = model::Position{
                                                current_pos.x + dog->GetVelocity().vx * time
                                                , current_pos.y + dog->GetVelocity().vy * time
                                            };
                // Получаем карту дорог
                auto roads_map = session.GetMapRoads();
                // Получаем id дороги на которой находится пес
                auto old_road_id = player->GetRoadId();

                // Проверяем валидность id
                if(!old_road_id) {
                    throw std::logic_error("Current player has no valid road ID assigned");
                }

                // Проверяем валидность новой позиции
                auto road_id = roads_map.IsValidPosition(new_pos, old_road_id);

                // Если новая позиция отвутствует на ценральной оси дорог
                if(!road_id){
                    // Получаем ближайшую позицию
                    NearestPointType nearest_point = roads_map.FindNearestPoint(old_road_id, current_pos
                                                                                            , current_derect);
                    dog->SetPosition(nearest_point.second);
                    dog->ResetVelocity();
                    player->SetRoadId(nearest_point.first);
                } else {
                    dog->SetPosition(new_pos);
                    player->SetRoadId(road_id.value());
                }
                // Получаем shared_ptr<Player>
                auto control_player = game_manager_.GetPlayer(player->GetToken());

                // Проверяем активность игрока
                if(IsRemovePlayer(control_player, delta_time, start_velocity)) {
                    // Добавляем токен игрока, которого необходимо удалить
                    token_player_for_remove.emplace_back(std::string(player->GetToken()));
                }
            } // for(auto [_, dog] : session->GetDogsList())

            // Проверяем активность игроков и при отсутствии активности в течении
            //   допустимого периода - удаляем неактивных, сохранив их достижения в БД
            ControlPlayersInGame(token_player_for_remove);
        }

        void Application::AddLostObject(double delta_time, model::GameSession &session) {
            // Получаем количество объектов в сессии
            unsigned curent_count_obj = static_cast<unsigned>(session.GetLostObjects().size());
            // Получаем количество игроков в сессии
            unsigned curent_count_players = static_cast<unsigned>(session.GetDogsList().size());
            // Получем период в миллисекундах
            std::chrono::milliseconds delta_t{static_cast<int64_t>(delta_time)};
            // Получаем необходимое кол-во объектов для добавления на карту
            auto count_object = loot_generator_.value().Generate(delta_t,curent_count_obj, curent_count_players);

            // Добавляем необходимое кол-во объектов на карту
            session.AddLostObjects(count_object);
        }

    void Application::ControlPlayersInGame(const std::vector<std::string>& tokens) {
        std::vector<domain::PlayerRecord> player_to_record;
        for(auto it = tokens.begin(); it != tokens.end(); ++it) {
            // Получаем игрока по токену
            auto player = game_manager_.GetPlayer(*it);
            // Проверяем есть ли игрок с таким токеном
            if(!player){
                continue;
            }
            // Создаем объект игрока для записи в таблицу
            domain::PlayerRecord player_record{player->GetName(), player->GetScore()
                                                                , player->GetTimeInGame().count()};
            // Добавляем игрока в вектор для записи
            player_to_record.emplace_back(player_record);
            // Удаляем пользователя из игры
            game_manager_.RemovePlayer(player);
        }
        // Сохраняем данные в таблицу
        use_cases_.AddPlayerRecords(player_to_record);
    }

} // app

