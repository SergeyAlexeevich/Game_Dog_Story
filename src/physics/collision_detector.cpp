#include "collision_detector.h"

#include <cassert>

namespace collision_detector {

    CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
        // Проверим, что перемещение ненулевое.
        // Тут приходится использовать строгое равенство, а не приближённое,
        // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
        // расстояние.
        const double u_x = c.x - a.x;
        const double u_y = c.y - a.y;
        const double v_x = b.x - a.x;
        const double v_y = b.y - a.y;
        const double u_dot_v = u_x * v_x + u_y * v_y;
        const double u_len2 = u_x * u_x + u_y * u_y;
        const double v_len2 = v_x * v_x + v_y * v_y;
        const double proj_ratio = u_dot_v / v_len2;
        const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

        return CollectionResult(sq_distance, proj_ratio);
    }

    // Ищем события (доставка игрок пришел на базу или игрок собрал потерянный предмет)
    std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
        std::vector<GatheringEvent> detected_events;

        static auto PointsEqual = [](geom::Point2D p1, geom::Point2D p2) {
            return p1.x == p2.x && p1.y == p2.y;
        };

        for(size_t g = 0; g < provider.GatherersCount(); ++g){
            Gatherer gatherer = provider.GetGatherer(g);

            if(PointsEqual(gatherer.start_pos, gatherer.end_pos)){
                continue;
            }

            for (size_t i = 0; i < provider.ItemsCount(); ++i) {
                Item item = provider.GetItem(i);
                auto collect_result
                    = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);

                if (collect_result.IsCollected(gatherer.width + item.width)) {
                    GatheringEvent evt{.item_id = provider.GetItemId(i),
                                    .gatherer_id = provider.GetGathererId(g),
                                    .sq_distance = collect_result.sq_distance,
                                    .time = collect_result.proj_ratio,
                                    .actor = item.type == LOST_OBJ ? MOVE_BAG : MOVE_BASE};
                    detected_events.emplace_back(evt);
                }
            }
            
        }

        std::sort(detected_events.begin(), detected_events.end()
                    , [](const GatheringEvent& e_l, const GatheringEvent& e_r){
                        return e_l.time < e_r.time;

        });

        return detected_events;
    }

    // Возвращаем количество предметов
    size_t ItemGathererProviderImpl::ItemsCount() const {
        return items_.size();
    }

    // Возвращаем предмет по индексу
    Item ItemGathererProviderImpl::GetItem(size_t idx) const {
        ItemId id = items_id_.at(idx);
        return items_.at(id);
    }

    // Возвращаем id предмета по индексу
    ItemGathererProviderImpl::ItemId ItemGathererProviderImpl::GetItemId(size_t idx) const {
        return items_id_.at(idx);
    }

    // Добавляем предмет в индекс
    void ItemGathererProviderImpl::AddItem(ItemId item_id, Item item)  {
        items_id_.emplace_back(item_id);
        items_.emplace(item_id,item);
    }

    // Возвращаем количество собирателей
    size_t ItemGathererProviderImpl::GatherersCount() const{
        return gatherers_.size();
    }

    // Возвращаем соискателя по индексу
    Gatherer ItemGathererProviderImpl::GetGatherer(size_t idx) const{
        GathererId id = gatherers_id_.at(idx);

        return gatherers_.at(id);
    }

    // Возвращаем id соискателя по индексу
    ItemGathererProviderImpl::GathererId ItemGathererProviderImpl::GetGathererId(size_t idx) const {
        return gatherers_id_.at(idx);
    }

    // Добавляем соискателя в индекс
    void ItemGathererProviderImpl::AddGatherer(GathererId gatherer_id,  Gatherer gatherer)  {
        gatherers_id_.emplace_back(gatherer_id);
        gatherers_.emplace(gatherer_id, std::move(gatherer));
    }
   
    // Очищаем индекс соискателей
    void ItemGathererProviderImpl::ResetItemList() noexcept {
        items_id_.clear();
        items_.clear();
    }
   
    // Очищаем индекс предметов
    void ItemGathererProviderImpl::ResetGathererList() noexcept {
        gatherers_id_.clear();
        gatherers_.clear();
    }

    // Очищаем все индексы
    void ItemGathererProviderImpl::FullResetLists() noexcept {
        ResetGathererList();
        ResetItemList();
    };

} // namespace collision_detector