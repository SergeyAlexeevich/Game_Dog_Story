#pragma once

#include "geom.h"

#include <algorithm>
#include <vector>
#include <unordered_map>

namespace collision_detector {

    enum class TypeObject{
        LOST_OBJ,
        BASE
    };

    enum class Actor{
        MOVE_BAG,
        MOVE_BASE
    };

    using TypeObject::LOST_OBJ;
    using TypeObject::BASE;
    using Actor::MOVE_BAG;
    using Actor::MOVE_BASE;

    struct CollectionResult {
        bool IsCollected(double collect_radius) const {
            return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
        }

        // квадрат расстояния до точки
        double sq_distance;
        // доля пройденного отрезка
        double proj_ratio;
    };

    // Движемся из точки a в точку b и пытаемся подобрать точку c.
    // Эта функция реализована в уроке.
    CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

    struct Item {
        geom::Point2D position;
        double width;
        TypeObject type = LOST_OBJ;
    };

    struct Gatherer {
        geom::Point2D start_pos;
        geom::Point2D end_pos;
        double width;
    };

    class ItemGathererProvider {
    protected:
        ~ItemGathererProvider() = default;

    public:
        virtual size_t ItemsCount() const = 0;
        virtual Item GetItem(size_t idx) const = 0;
        virtual size_t GatherersCount() const = 0;
        virtual Gatherer GetGatherer(size_t idx) const = 0;
        virtual size_t GetItemId(size_t idx) const = 0;
        virtual size_t GetGathererId(size_t idx) const = 0;
    };

    struct GatheringEvent {
        size_t item_id;
        size_t gatherer_id;
        double sq_distance;
        double time;
        // Выполненное действие
        Actor actor = MOVE_BAG;
    };

    class ItemGathererProviderImpl: public ItemGathererProvider {
    public:
        using ItemId = size_t;
        using GathererId = size_t;

        virtual ~ItemGathererProviderImpl() = default;
        
        size_t ItemsCount() const;

        Item GetItem(size_t idx) const override;
        ItemId GetItemId(size_t idx) const override;
        size_t GatherersCount() const override;
        Gatherer GetGatherer(size_t idx) const override;
        GathererId GetGathererId(size_t idx) const override;

        void AddItem(ItemId item_id, Item item);
        void AddGatherer(GathererId gatherer_id, Gatherer gatherer);

        void ResetItemList() noexcept;
        void ResetGathererList() noexcept;

        void FullResetLists() noexcept;
        
    private:
        std::vector<ItemId> items_id_;
        std::unordered_map<ItemId, Item> items_;
        std::vector<GathererId> gatherers_id_;
        std::unordered_map<ItemId, Gatherer> gatherers_;
    };

    // Эту функцию вам нужно будет реализовать в соответствующем задании.
    // При проверке ваших тестов она не нужна - функция будет линковаться снаружи.
    std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

}  // namespace collision_detector