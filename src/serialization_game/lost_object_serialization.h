#pragma once

#include <string>

#include <boost/serialization/serialization.hpp>

#include "../models/lost_object.h"
#include "geometry_primitives_serialization.h"

namespace serialization {

    using RoadId = size_t;

    // LostObjectRepr (LostObjectRepresentation) - сериализованное представление класса LostObject
    class LostObjectRepr{
    public:
        LostObjectRepr() = default;
        explicit LostObjectRepr(const model::LostObject& lost_object)
        : id_{lost_object.GetId()}
        , pos_{lost_object.GetPosition()}
        , type_{lost_object.GetType()}
        , value_{lost_object.GetScore()}
        , road_id_{lost_object.GetRoad()} {
        }

        // Восстанавливаем данные потерянного объекта
        [[nodiscard]] model::LostObject Restore() const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

    private:
        size_t id_;
        model::Position pos_;
        size_t type_;
        size_t value_ = 0;
        RoadId road_id_;
        bool is_restore_ = false;
    };

    template <typename Archive>
    inline void LostObjectRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& pos_;
        ar& type_;
        ar& value_;
        ar& road_id_;
        ar& is_restore_;
    }

} // namespace serialization