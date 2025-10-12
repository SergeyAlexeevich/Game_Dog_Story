#include "collision_manager.h"

#include "utils.h"

#include "../logging/logger.h"

namespace app {

    using namespace std::literals;

    // Обработчик коллизий
    void CollisionManager::HandlerCollision(model::GameSession& session) {
        // Добавляем все офисы бюро находок в индекс
        AddOfficesInItems(session.GetMap().GetOffices());
        // Добавляем все потерянные предметы в индекс
        AddObjectsInItems(session.GetLostObjects());
        // Добавляем всех соискателей в индекс
        AddGatherer(session.GetDogsList());
        // Получаем список всех событий
        auto events = collision_detector::FindGatherEvents(provider_);
        // Обрабатываем события
        RequestEvent(session, events);
        // Очищаем все индексы в провайдере
        ResetProvider();
    }

    // Добавляем все офисы бюро находок в индекс
    void CollisionManager::AddOfficesInItems(const std::vector<model::Office>& offices) {
        for(const auto& office : offices){
            collision_detector::Item item{
                .position = { static_cast<double>(office.GetPosition().x)
                            , static_cast<double>(office.GetPosition().y)
                }
                , .width = game_.GetGameSetting().default_office_width
                , .type = collision_detector::BASE
            };

            provider_.AddItem(Stoi(*office.GetId()),item);        
        }
    }

    // Добавляем все потерянные предметы в индекс
    void CollisionManager::AddObjectsInItems(const std::unordered_map<size_t, model::LostObject> &lost_object) {
        for(const auto& object : lost_object){
            collision_detector::Item item{
                .position = { static_cast<double>(object.second.GetPosition().x)
                            , static_cast<double>(object.second.GetPosition().y)
                }
                , .width = game_.GetGameSetting().default_office_width
            };
                
            provider_.AddItem(object.second.GetId(),item);
        }
    }

    // Добавляем всех соискателей в индекс
    void CollisionManager::AddGatherer(const std::unordered_map<uint64_t, std::shared_ptr<model::Dog>> &gatherer) {
        for(const auto& dog : gatherer){
            collision_detector::Gatherer gather{
                .start_pos = {dog.second->GetOldPosition().x
                            , dog.second->GetOldPosition().y
                }
                , .end_pos = {dog.second->GetCurrentPosition().x
                            , dog.second->GetCurrentPosition().y
                }
                , .width = game_.GetGameSetting().default_player_width
            };

            provider_.AddGatherer(dog.first, gather);
        }
    }

    // Очищаем все индексы в провайдере
    void CollisionManager::ResetProvider() noexcept {
        provider_.FullResetLists();
    }

    // Обрабатываем события
    void CollisionManager::RequestEvent(model::GameSession& session, std::vector<collision_detector::GatheringEvent>& events) {
        for(auto& event : events){
            // Находим игрока
            auto player = game_manager_.FindPlayerByDogAndMapId(event.gatherer_id, Stoi(*session.GetMapId()));
            // Проверяем что это событие - сбор потерянного предмета
            if(event.actor == collision_detector::Actor::MOVE_BAG){
                // Проверяем есть ли место в рюкзаке
                if(!player->IsBagFull()){
                    auto item = session.GetLostObj(event.item_id);

                    // Проверяем есть ли такой предмет в сессии на карте
                    if(item){
                    // Перемещаем предмет в рюкзак
                    player->AddObjInBag(std::move(item.value()));
                    // Удаляем предмет из эндекса элементов сессии, чтобы больше его не добавлять
                    session.RemoveLostObj(event.item_id);
                    }
                }
                continue;
            }
            
            // Выгружаем найденные предметы на базе(бюро находок)
            auto items = player->ExtractObjectFromBag();
            // Начисляем очки за найденные предметы
            player->AddScore(items);
        }
    }

} //namespace app