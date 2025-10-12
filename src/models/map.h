#pragma once

#include "geometry_primitives.h"
#include "road.h"
#include "building.h"
#include "office.h"
#include "loot_types.h"



namespace model {

    inline double DEFAULT_SPEED_ON_MAP = 1.0;

    class Map {
    public:
        using Id = util::Tagged<std::string, Map>;
        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;
        using LootTypes = std::vector<LootType>;

        Map(Id id, std::string name) noexcept
            : id_(std::move(id))
            , name_(std::move(name)) {
        }

        const Id& GetId() const noexcept;
        const std::string& GetName() const noexcept;
        const Buildings& GetBuildings() const noexcept;
        const Roads& GetRoads() const noexcept;
        const Offices& GetOffices() const noexcept;
        const LootTypes& GetLootTypes() const noexcept;
        size_t GetBagCapacity() const noexcept;
        double GetSpeed_characters() const noexcept;

        void SetSpeedCharacters(double speed);
        void SetBagCapacity(size_t bag_capacity);

        void AddRoad(const Road& road);
        void AddBuilding(const Building& building);
        void AddOffice(Office office);
        void AddLootType(LootType loot_types);

    private:
        using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

        Id id_;
        std::string name_;
        Roads roads_;
        Buildings buildings_;
        OfficeIdToIndex warehouse_id_to_index_;
        Offices offices_;
        LootTypes loot_types_;
        double speed_characters_ = DEFAULT_SPEED_ON_MAP;
        size_t bag_capacity_ = 0;
        size_t road_id_ = 0;
    };
    
} // model