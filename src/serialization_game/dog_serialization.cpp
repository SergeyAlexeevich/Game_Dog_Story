#include "dog_serialization.h"

namespace serialization {

    [[nodiscard]] model::Dog DogRepr::Restore() const {
            model::Dog dog{name_, id_};
            dog.SetVelocity(velocity_);
            dog.SetDirection(direction_);
            dog.SetPosition(old_pos_);
            dog.SetPosition(current_pos_);
            dog.SetRoadId(curent_road_id_);
            
            return dog;
    }

} // namespace serialization