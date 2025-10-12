#include "map.h"

#include <stdexcept>

#include "application/game_manager.h"

namespace model {
    
    using namespace std::literals;

    const Map::Id& Map::GetId() const noexcept {
        return id_;
    }

    const std::string &Map::GetName() const noexcept {
        return name_;
    }

    const Map::Buildings &Map::GetBuildings() const noexcept {
        return buildings_;
    }

    const Map::Roads &Map::GetRoads() const noexcept {
        return roads_;
    }

    const Map::Offices &Map::GetOffices() const noexcept {
        return offices_;
    }

    const Map::LootTypes &Map::GetLootTypes() const noexcept {
        return loot_types_;
    }

    size_t Map::GetBagCapacity() const noexcept {
        return bag_capacity_;
    }

    void Map::AddRoad(const Road &road) {
        roads_.emplace_back(road);
        roads_.back().SetId(++road_id_);
    }

    void Map::AddBuilding(const Building &building) {
        buildings_.emplace_back(building);
    }

    void Map::SetSpeedCharacters(double speed){
        speed_characters_ = speed;
    }

    double Map::GetSpeed_characters() const noexcept {
        return speed_characters_;
    }

    void Map::AddOffice(Office office) {
        if (warehouse_id_to_index_.contains(office.GetId())) {
            throw std::invalid_argument("Duplicate warehouse");
        }

        const size_t index = offices_.size();
        Office& o = offices_.emplace_back(std::move(office));

        try {
            warehouse_id_to_index_.emplace(o.GetId(), index);
        } catch (...) {
            // Удаляем офис из вектора, если не удалось вставить в unordered_map
            offices_.pop_back();
            throw;
        }
    }

    void Map::AddLootType(LootType loot_types) {
        loot_types_.emplace_back(loot_types);
    }

    void Map::SetBagCapacity(size_t bag_capacity) {
        bag_capacity_ = bag_capacity;
    }

}  // namespace model