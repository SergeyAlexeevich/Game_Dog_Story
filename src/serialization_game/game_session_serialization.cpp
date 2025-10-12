#include "game_session_serialization.h"

namespace serialization {

    [[nodiscard]] model::GameSession GameSessionRepr::Restore(const model::Game& game) const {
        if(auto map = game.FindMap(model::Map::Id{id_map_}); map){
            model::GameSession session{*map, id_session_, true};
            session.Restore(GetRestoreDogsList(), GetRestoreLostObjectsList(), GetRestoreLostItemPos());

            return session;    
        }    

        throw std::logic_error(
                "Failed to restore game session. Map not found. "s
                "Session ID: "s + std::to_string(id_session_) + 
                ", Map ID: "s + id_map_ + 
                ", Dog ID: "s + std::to_string(dog_id_)
            );
    }

    model::DogsList GameSessionRepr::GetRestoreDogsList() const {
        model::DogsList result;

        for(const auto& [key,dog] : dogs_list_) {
            result.emplace(key, std::make_shared<model::Dog>(dog.Restore()));
        }

        return result;
    }

    model::LostObjectType GameSessionRepr::GetRestoreLostObjectsList() const {
        model::LostObjectType result;
        
        for(const auto& [key, obj] : lost_objects_){
            result.emplace(key, obj.Restore());
        }

        return result;
    }

    std::unordered_set<model::Position, model::PositionHasher> GameSessionRepr::GetRestoreLostItemPos() const {
        std::unordered_set<model::Position, model::PositionHasher> result;
        
        for(const model::Position& pos : lost_items_positions_){
            result.emplace(pos);
        }

        return result;
    }

} // namespace serialization