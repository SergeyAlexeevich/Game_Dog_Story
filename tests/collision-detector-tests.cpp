#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../physics/collision_detector.h"

#include <string>
#include <memory>

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

namespace collision_detector {

} // namespace collision_detector_tests

namespace catch_tests {

    using namespace std::literals;
    const std::string TAG = "[FindGatherEvents]"s;

    TEST_CASE("Gather collect one item moving on x-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item{{12.5, 0}, 0.6};
        collision_detector::Gatherer gatherer{{0, 0}, {22.5, 0}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 1);
        CHECK(events[0].item_id == 0);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item.position.x/gatherer.end_pos.x), 1e-9)); 
    }

    TEST_CASE("Gather collect one item moving on x-axis on edge"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item{{12.5, 0}, 0.6};
        collision_detector::Gatherer gatherer{{0, 0}, {12.5, 0}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 1);
        CHECK(events[0].item_id == 0);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item.position.x/gatherer.end_pos.x), 1e-9)); 
    }

    TEST_CASE("Gather collect one item moving on x-axis on side"s, TAG) {
        using Catch::Matchers::WithinRel;
        collision_detector::Item item{{12.5, 0.5}, 0.0};
        collision_detector::Gatherer gatherer{{0, 0.1}, {22.5, 0.1}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 1);
        CHECK(events[0].item_id == 0);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinRel(0.16, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item.position.x/gatherer.end_pos.x), 1e-9)); 
    }

    TEST_CASE("Gather collect one item moving on y-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item{{0, 12.5}, 0.6};
        collision_detector::Gatherer gatherer{{0, 0}, {0, 22.5}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 1);
        CHECK(events[0].item_id == 0);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item.position.y/gatherer.end_pos.y), 1e-9)); 
    }

    TEST_CASE("Gather collect two unordered items moving on x-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item1{{12.5, 0}, 0.6};
        collision_detector::Item item2{{6.5, 0}, 0.6};
        collision_detector::Gatherer gatherer{{0, 0}, {22.5, 0}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item1);
        provider.AddItem(1, item2);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 2);

        CHECK(events[0].item_id == 1);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item2.position.x/gatherer.end_pos.x), 1e-9)); 
        
        CHECK(events[1].item_id == 0);
        CHECK(events[1].gatherer_id == 0);
        CHECK_THAT(events[1].sq_distance, WithinRel(0.0, 1e-9));
        CHECK_THAT(events[1].time, WithinRel((item1.position.x/gatherer.end_pos.x), 1e-9)); 
    }

    TEST_CASE("Gather collect one of two items moving on x-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item1{{42.5, 0}, 0.6};
        collision_detector::Item item2{{6.5, 0}, 0.6};
        collision_detector::Gatherer gatherer{{0, 0}, {22.5, 0}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item1);
        provider.AddItem(1, item2);
        provider.AddGatherer(0, gatherer);
        auto events = collision_detector::FindGatherEvents(provider);

        CHECK(events.size() == 1);

        CHECK(events[0].item_id == 1);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item2.position.x/gatherer.end_pos.x), 1e-9)); 
    }

    TEST_CASE("Two gathers collect two separate items moving on x-axis and y-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item1{{0, 12.5}, 0.6};
        collision_detector::Item item2{{6.5, 0}, 0.6};
        collision_detector::Gatherer gatherer1{{0, 0}, {22.5, 0}, 0.6};
        collision_detector::Gatherer gatherer2{{0, 0}, {0, 22.5}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item1);
        provider.AddItem(1, item2);
        provider.AddGatherer(0, gatherer1);
        provider.AddGatherer(1, gatherer2);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 2);

        CHECK(events[0].item_id == 1);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item2.position.x/gatherer1.end_pos.x), 1e-9)); 
        
        CHECK(events[1].item_id == 0);
        CHECK(events[1].gatherer_id == 1);
        CHECK_THAT(events[1].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[1].time, WithinRel((item1.position.y/gatherer2.end_pos.y), 1e-9)); 
    }

    TEST_CASE("Two gathers collect three items moving on x-axis"s, TAG) {
        using Catch::Matchers::WithinRel;
        using Catch::Matchers::WithinAbs;
        collision_detector::Item item1{{12.5, 0}, 0.6};
        collision_detector::Item item2{{6.5, 0}, 0.6};
        collision_detector::Gatherer gatherer1{{0, 0}, {22.5, 0}, 0.6};
        collision_detector::Gatherer gatherer2{{0, 0}, {10, 0}, 0.6};
        collision_detector::ItemGathererProviderImpl provider;
        provider.AddItem(0, item1);
        provider.AddItem(1, item2);
        provider.AddGatherer(0, gatherer1);
        provider.AddGatherer(1, gatherer2);
        auto events = collision_detector::FindGatherEvents(provider);
        
        CHECK(events.size() == 3);

        CHECK(events[0].item_id == 1);
        CHECK(events[0].gatherer_id == 0);
        CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[0].time, WithinRel((item2.position.x/gatherer1.end_pos.x), 1e-9)); 

        CHECK(events[1].item_id == 0);
        CHECK(events[1].gatherer_id == 0);
        CHECK_THAT(events[1].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[1].time, WithinRel((item1.position.x/gatherer1.end_pos.x), 1e-9)); 

        CHECK(events[2].item_id == 1);
        CHECK(events[2].gatherer_id == 1);
        CHECK_THAT(events[2].sq_distance, WithinAbs(0.0, 1e-9));
        CHECK_THAT(events[2].time, WithinRel((item2.position.x/gatherer2.end_pos.x), 1e-9)); 


    }
 } // namespace catch_tests