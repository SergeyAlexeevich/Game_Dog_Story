#include "bag_serialization.h"

namespace serialization {

    [[nodiscard]] model::Bag BagRepr::Restore() const {
            // Создаем временный объект рюкзака
            model::Bag bag{max_capacity_};

            // Восстанавливаем каждый объект через LostObjectRepr
            for (const auto& obj_repr : objects_) {
                bag.AddObject(obj_repr.Restore());
            }

            return bag;
        }

} // namespace serialization