#pragma once

#include <vector>

#include <boost/serialization/vector.hpp>

#include "../models/bag.h"
#include "lost_object_serialization.h"

namespace serialization {

    using ConteinerByObj = std::vector<serialization::LostObjectRepr>;

    // BagRepr (BagRepresentation) - сериализованное представление класса Bag
    class BagRepr{
    public:
        BagRepr() = default;
        explicit BagRepr(const model::Bag& bag)
        : max_capacity_{bag.GetMaxCapacity()} {
            for (const auto& obj : bag.GetObjects()) {
                objects_.emplace_back(LostObjectRepr(obj));
            }
        }

        // Восстанавливаем данные рюкзака
        [[nodiscard]] model::Bag Restore() const;

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version);

    private:
        size_t max_capacity_ = 0;    
        ConteinerByObj objects_;
    };

    template <typename Archive>
    inline void BagRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& max_capacity_;
        ar& objects_;
    }

} // namespace serialization