#include "lost_object_serialization.h"

namespace serialization {

    [[nodiscard]] model::LostObject LostObjectRepr::Restore() const {
        model::LostObject lost_object{pos_, type_, value_, true};

        lost_object.SetRoadId(road_id_);
        lost_object.SetIdForRestore(model::LostObject::Id{id_});

        return lost_object;
    }

} // namespace serialization