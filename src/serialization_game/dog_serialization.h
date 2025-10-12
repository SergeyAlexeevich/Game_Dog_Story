#pragma once

#include <string>

#include <boost/serialization/serialization.hpp>

#include "../models/geometry_primitives.h"
#include "../models/dog.h"
#include "geometry_primitives_serialization.h"

namespace serialization {

    using namespace std::literals;

    // DogRepr (DogRepresentation) - сериализованное представление класса Dog
    class DogRepr{
    public:
        DogRepr() = default;
        explicit DogRepr(const model::Dog& dog)
        : name_{dog.GetName()}
        , id_{dog.GetId()}
        , direction_{dog.GetDirection()}
        , current_pos_{dog.GetCurrentPosition()}
        , old_pos_{dog.GetOldPosition()}
        , velocity_{dog.GetVelocity()}
        , curent_road_id_{dog.GetRoadId()} {
        }

        // Восстанавливаем данные пса
        [[nodiscard]] model::Dog Restore() const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

    private:
        std::string name_ = ""s;
        uint64_t id_ = 0;
        model::Direction direction_{model::Direction::NORTH};
        model::Position current_pos_{0.0, 0.0};
        model::Position old_pos_{0.0, 0.0};
        model::Velocity velocity_{0.0, 0.0};
        model::RoadId curent_road_id_ = 1;
    };

    template <typename Archive>
    inline void DogRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& name_;
        ar& direction_;
        ar& current_pos_;
        ar& old_pos_;
        ar& velocity_;
        ar& curent_road_id_;
    }

} // namespace serialization