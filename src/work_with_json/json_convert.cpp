#include "json_convert.h"

#include <sstream>

#include <boost/json.hpp>

namespace json_converter{
    
    namespace json = boost::json;

    using namespace std::literals;

    std::string GetMapListToJson(const model::Game& game) {
        auto maps_game = game.GetMaps();

        if (maps_game.empty()) {
            throw std::logic_error("No maps available in the game");
        }

        json::array array_obj;

        for(const auto& map : maps_game){
            array_obj.emplace_back( 
                json::object { {"id"sv, *map.GetId() }, { "name"sv, map.GetName() } }
            );
        }

        return json::serialize(array_obj);
    }

    json::array GetRoadsToJson(const model::Map& map){
        auto roads = map.GetRoads();

        if (roads.empty()) {
            throw std::logic_error("No roads available in the map");
        }

        json::array array_roads;

        for(model::Road road : roads){
            json::object obj{ {"x0"sv, road.GetStart().x }, {"y0"sv, road.GetStart().y } };

            if(road.IsHorizontal()){
                obj["x1"sv] = road.GetEnd().x;
            } else {
                obj["y1"sv] = road.GetEnd().y;
            }

            array_roads.emplace_back(std::move(obj));
        }

        return array_roads;
    }

    json::array GetBuildingsToJson(const model::Map& map) {
        auto buildings = map.GetBuildings();

        if (buildings.empty()) {
            return json::array{};
        }

        json::array array_building;

        for(model::Building building : buildings){
            array_building.emplace_back(
                json::object{ 
                    {"x"sv, building.GetBounds().position.x}
                    , {"y"sv, building.GetBounds().position.y}
                    , {"w"sv, building.GetBounds().size.width}
                    , {"h"sv, building.GetBounds().size.height}
                }
            );
        }

        return array_building;
    }

    json::array GetOfficesToJson(const model::Map& map) {
        auto offices = map.GetOffices();

        if (offices.empty()) {
            return json::array{};
        }

        json::array array_offices;
        for(model::Office office : offices){
            array_offices.emplace_back(
                json::object{
                    {"id"sv, *office.GetId()}
                    , {"x"sv, office.GetPosition().x}
                    , {"y"sv, office.GetPosition().y}
                    , {"offsetX"sv, office.GetOffset().dx}
                    , {"offsetY"sv, office.GetOffset().dy}
                }
            );
        }

        return array_offices;
    }

    json::array GetLootTypes(const model::Map& map) {
        auto loot_types = map.GetLootTypes();
        json::array arr_loot_types;

        for(const auto& loot_type : loot_types){
            json::object obj;

            if (loot_type.name) {
                obj.emplace("name"s, loot_type.name.value());
            } 
            
            if (loot_type.file) {
                obj.emplace("file"s, loot_type.file.value());
            }

            if (loot_type.type) {
                obj.emplace("type"s, loot_type.type.value());
            }

            if (loot_type.rotation) {
                obj.emplace("rotation"s, loot_type.rotation.value());
            }

            if (loot_type.color) {
                obj.emplace("color"s, loot_type.color.value());
            }

            if (loot_type.scale) {
                obj.emplace("scale"s, loot_type.scale.value());
            }

            if (loot_type.value) {
                obj.emplace("value"s, loot_type.value.value());
            }

            arr_loot_types.emplace_back(obj);
        }

        return arr_loot_types;
    }

    std::string GetNotFoundMapToJson() noexcept {
        json::object obj{
            {"code"sv, "mapNotFound"sv}
            , {"message"sv, "Map not found"sv}
        };

        return json::serialize(obj);
    }

    std::string GetFoundMapToJson(const model::Game& game, std::string_view id) {
        const model::Map* founded_map = game.FindMap(model::Map::Id{std::string(id)});

        if(!founded_map){
            return GetNotFoundMapToJson();
        }

        json::object map{
            {"id"sv, *founded_map->GetId()}
            , {"name"sv, founded_map->GetName()}
            , {"roads"sv, GetRoadsToJson(*founded_map) }
        };

        if(json::array building = GetBuildingsToJson(*founded_map); !building.empty()){
            map.emplace("buildings"sv, building);
        }

        if(json::array offices = GetOfficesToJson(*founded_map); !offices.empty() ){
            map.emplace("offices"sv, offices);
        }

        map.emplace("lootTypes"sv, GetLootTypes(*founded_map));

        return json::serialize(map);
    }

    std::string GetBadRequestToJson() noexcept {
        json::object obj{
            {"code"sv, "badRequest"sv},
            {"message"sv, "Bad request"sv}
        };

        return json::serialize(obj);
    }

    std::string GetMethodNotAllowedJson() noexcept {
        json::object obj{
            {"code"sv, "invalidMethod"sv},
            {"message"sv, "Method not allowed"sv}
        };

        return json::serialize(obj);
    }

} //json_converter