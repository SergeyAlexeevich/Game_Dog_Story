#pragma once

#include <boost/serialization/serialization.hpp>

#include "../models/geometry_primitives.h"

namespace model {

    template <typename Archive>
    void serialize(Archive& ar, model::Point& point, [[maybe_unused]] const unsigned version) {
        ar& point.x;
        ar& point.y;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Size& size, [[maybe_unused]] const unsigned version) {
        ar& size.height;
        ar& size.width;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Rectangle& rectangle, [[maybe_unused]] const unsigned version) {
        ar& rectangle.position;
        ar& rectangle.size;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Offset& offset, [[maybe_unused]] const unsigned version) {
        ar& offset.dx;
        ar& offset.dy;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Direction& direction, [[maybe_unused]] const unsigned version) {
        int temp;
        ar & temp;
        direction = static_cast<model::Direction>(temp);
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Position& position, [[maybe_unused]] const unsigned version) {
        ar& position.x;
        ar& position.y;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::Velocity& velocity, [[maybe_unused]] const unsigned version) {
        ar& velocity.vx;
        ar& velocity.vy;
    }

    template <typename Archive>
    void serialize(Archive& ar, model::TypeRoad& type_road, [[maybe_unused]] const unsigned version) {
        int temp;
        ar & temp;
        type_road = static_cast<model::TypeRoad>(temp);
    }    

} // namespace model