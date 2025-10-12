#pragma once 

#include "../physics/collision_detector.h"

#include <unordered_map>

#include "game_manager.h"
#include "../models/game.h"
#include "../models/office.h"
#include "../models/lost_object.h"
#include "../models/dog.h"
#include "../models/game_session.h"

namespace app{

    class CollisionManager{
    public:
        CollisionManager(model::Game& game, GameManager& game_manager) 
        : game_{game}
        , game_manager_{game_manager} { 
        }

        void HandlerCollision(model::GameSession& session);

    private:
        void AddOfficesInItems(const std::vector<model::Office>& offices);
        void AddObjectsInItems(const std::unordered_map<size_t, model::LostObject>& lost_object);
        void AddGatherer(const std::unordered_map<uint64_t, std::shared_ptr<model::Dog>>& gatherer);
        void ResetProvider() noexcept;
        void RequestEvent(model::GameSession& session, std::vector<collision_detector::GatheringEvent>& events);

    private:
        model::Game& game_;
        GameManager& game_manager_;
        collision_detector::ItemGathererProviderImpl provider_;
    };

} //app