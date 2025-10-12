#pragma once

#include <unordered_map>
#include <unordered_set>

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include "../models/game_session.h"
#include "../models/game.h"
#include "dog_serialization.h"
#include "lost_object_serialization.h"
#include "geometry_primitives_serialization.h"

namespace serialization {
    
    using namespace std::literals;

    // GameSessionRepr (GameSessionRepresentation) - сериализованное представление класса GameSession
    class GameSessionRepr{
    public:
        GameSessionRepr() = default;
        explicit GameSessionRepr(const model::GameSession& session)
        : id_map_{*session.GetMapId()}
        , max_count_dog_{model::MAX_DOG_ON_MAP}
        , dogs_list_{}
        , lost_objects_{}
        , lost_items_positions_{session.GetLostItemsPosition().begin(), session.GetLostItemsPosition().end()}
        , id_session_{session.GetGameSessionId()}
        , dog_id_{0} {
            // Заполняем список собак
            for(const auto& [dog_id, dog] : session.GetDogsList()){
                dogs_list_.emplace(dog_id,DogRepr{*dog});
            }

            // Заполняем список объектов
            for(const auto& [obj_id, obj] : session.GetLostObjects()){
                lost_objects_.emplace(obj_id, LostObjectRepr{obj});
            }

        }

        // Восстанавливаем данные сессии
        [[nodiscard]] model::GameSession Restore(const model::Game& game) const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

        std::string GetMapId() const noexcept {
            return id_map_;
        }

    private:
    model::DogsList GetRestoreDogsList() const;
    model::LostObjectType GetRestoreLostObjectsList() const;
    std::unordered_set<model::Position, model::PositionHasher> GetRestoreLostItemPos() const;

    private:
        std::string id_map_;
        size_t max_count_dog_ = 0;
        std::unordered_map<size_t,        // dog_id
                            DogRepr> dogs_list_;
        std::unordered_map<size_t, LostObjectRepr> lost_objects_;
        std::unordered_set<model::Position, model::PositionHasher> lost_items_positions_;
        size_t id_session_ = 0;
        size_t dog_id_ = 0;
    };

    template <typename Archive>
    inline void GameSessionRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_map_;
        ar& max_count_dog_;
        ar& dogs_list_;
        ar& lost_objects_;
        ar& lost_items_positions_;
        ar& id_session_;
        ar& dog_id_;
    }

} // namespace serialization