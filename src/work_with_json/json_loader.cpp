#include "json_loader.h"
#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <exception>


#include "../models/map.h"
#include "../logging/logger.h"

namespace json_loader {

    namespace json = boost::json;
    using namespace std::literals;

    bool ValidateKeys(const std::vector<std::string_view> name_coordinates, const json::object& obj){
        for(const auto& name_point : name_coordinates ){
            if(!obj.contains(name_point)){
                return false;
            }
        }
        return true;
    }

    std::vector<int> GetCoordinates(const std::vector<std::string_view> name_coordinates, const json::object& obj
                                    , std::string_view name_obj){
        if(!ValidateKeys(name_coordinates,obj)){
                std::string message{"Missing required coordinates "};
                bool is_first = true;

                for(auto name : name_coordinates) {
                    if(!is_first){
                        message += " or ";
                    }

                    is_first = false;
                    message += std::string(name);
                }
                message += " for " + std::string(name_obj) + "definition\n";

                throw std::logic_error(message);
        }

        std::vector<int> result;
        result.reserve(name_coordinates.size());

        for(const auto& name_point : name_coordinates ){
            auto value = obj.at(name_point);

            if (!value.is_int64()) {
                throw std::logic_error("Invalid type for coordinate " + std::string(name_point));
            }

            int temp_point = static_cast<int>(value.as_int64());

            result.emplace_back(temp_point);
        }

        return result;
    }

    bool IsValidColorFormat(const std::string& color) {
        // Проверяем базовую структуру
        if (color.empty() || color.size() != 7 || color[0] != '#') {
            return false;
        }
        
        // Проверяем, что все символы после '#' являются шестнадцатеричными цифрами
        for (size_t i = 1; i < 7; ++i) {
            char c = color[i];
            if (!std::isxdigit(c)) {
                return false;
            }
        }
        
        return true;
    }

    model::LootType GetLootType(const json::object& obj){
        model::LootType result;

        // Заполняем все поля
        for(const auto& [key,value] : obj) { 
            if( key == "name"sv) {
                result.name = value.as_string();
            } else if (key == "file"sv) {
                result.file = value.as_string();
            } else if (key == "type"sv) {
                result.type = value.as_string();
            } else if (key == "rotation"sv) {
                result.rotation = static_cast<int>(value.as_int64());
            } else if (key == "color"sv) {
                result.color = value.as_string();
            } else if (key == "scale"sv) {
                result.scale = value.as_double();
            } else if(key == "value"sv) { 
                result.value = static_cast<int>(value.as_int64());
            } else {

                #ifndef DEBAGER
                std::string message = "Unknown key in loot type:"s + std::string{key};
                #else
                std::string message = "Error in function " + __FUNCTION__ + 
                                                    " at file " + __FILE__ + 
                                                    " (line " + std::to_string(__LINE__) + "): " +
                                                    "Unknown key in loot type: " + key;
                #endif

                // Логируем неизвестный ключ как ошибку
                logger::LogEntryToConsole(json::object{}, message, boost::log::trivial::error);
            }
        }

        // Проверяем валидность масштаба
        if (result.scale <= 0.0) {
            throw std::logic_error("Scale must be positive"s);
        }

        return result;
    }

    std::string ReadFile(const std::filesystem::path& path, bool binary_mode) {  
        if (!std::filesystem::exists(path)) {
            throw std::filesystem::filesystem_error(
                "File does not exist", 
                path, 
                std::make_error_code(std::errc::no_such_file_or_directory)
            );
        }

        std::ifstream file;

        if(binary_mode){
            file.open(path, std::ios::binary);
        } else{
            file.open(path);
        }

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + std::string(path));
        }

        if(binary_mode){
            file.seekg(0, std::ios::end);
            std::streamoff file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(file_size);

            if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
                throw std::runtime_error("Error reading binary file: " + std::string(path));
            }
            
            return std::string(buffer.begin(),buffer.end());
        } else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string text_config = buffer.str();

            if (text_config.empty()) {
                throw std::runtime_error("File is empty: " + std::string(path));
            }

            return text_config;
        }
    }

    void AddRoadsToMap(model::Map& map, const json::value& config_map_json){  
        // Проверяю наличие ключа
        if (!config_map_json.as_object().contains("roads"sv)) {
            throw std::logic_error("Error: the map configuration is missing the required 'roads' parameter\n"s);
        }

        const auto& roads_json = config_map_json.as_object().at("roads"sv);

        // Проверяю, что массив roads не пустой
        if (roads_json.as_array().empty()) {
            throw std::logic_error("Error: the 'roads' array must contain at least one element\n"s);
        }

        for(auto coordinates_road : roads_json.as_array()){
            // Проверяю, что элемент является объектом
            if (!coordinates_road.is_object()) {
                logger::LogEntryToConsole(json::object{}, "Error: each road must be an object"s, boost::log::trivial::error);
                continue;
            }

            auto road_obj = coordinates_road.as_object();
            std::string_view variable_coordinate;
            bool is_horizontal = true;

            if(road_obj.contains("x1")){
                variable_coordinate = "x1";
            } else{
                variable_coordinate = "y1";
                is_horizontal = false;
            }

            const std::vector<int> coordinates = GetCoordinates(
                                std::vector{"x0"sv,"y0"sv, variable_coordinate},road_obj,"roads"sv
                            );

            model::Coord x{coordinates[0]};
            model::Coord y{coordinates[1]};
            model::Coord end{coordinates[2]};
            model::Point point_start{x,y};

            if(is_horizontal){
                map.AddRoad(model::Road{model::Road::HORIZONTAL, point_start,end});
            } else {
                map.AddRoad(model::Road{model::Road::VERTICAL, point_start,end});
            }
        }
    }

    void AddBuildingsToMap(model::Map& map, const json::value& config_map_json){ 
        // Проверяю наличие ключа
        if (!config_map_json.as_object().contains("buildings"sv)) {
            std::string message = "Error: the map configuration is missing the required 'buildings' parameter"s;
            logger::LogEntryToConsole(json::object{}, message, boost::log::trivial::error);
            
            return;
        }

        const auto& buildings_json = config_map_json.as_object().at("buildings"sv);

        // Проверяю, что массив roads не пустой, если пустой то не стойт выполнять дальнейшие действия
        if (buildings_json.as_array().empty()) {
            return;
        }

        for(auto coordinates_buildings : buildings_json.as_array()){
            // Проверяю, что элемент является объектом
            if (!coordinates_buildings.is_object()) {
                std::string message = "Error: each building must be an object"s;
                logger::LogEntryToConsole(json::object{}, message , boost::log::trivial::error);
                continue;
            }

            json::object buildings_obj = coordinates_buildings.as_object();

            if(!ValidateKeys (std::vector{"x"sv,"y"sv, "w"sv, "h"sv}, buildings_obj)){
                throw std::logic_error ("Missing required coordinates \"x\" or \"y\" or \"w\" or \"h\" for buildin definition"s);
            }

            const std::vector<int> coordinates = GetCoordinates(
                                std::vector{"x"sv,"y"sv,"w"sv,"h"sv}, buildings_obj, "buildings"sv
                            );

            model::Coord x{coordinates[0]};
            model::Coord y{coordinates[1]};
            model::Coord w{coordinates[2]};
            model::Coord h{coordinates[3]};

            map.AddBuilding(model::Building{model::Rectangle{model::Point{x,y}, model::Size{w,h} } });
        }
    }

    void AddOfficesToMap(model::Map& map, const json::value& config_map_json){  
        // Проверяю наличие ключа
        if (!config_map_json.as_object().contains("offices"sv)) {
            std::string message = "Error: the map configuration is missing the required 'offices' parameter"s;
            logger::LogEntryToConsole(json::object{}, message, boost::log::trivial::error);
            return;
        }

        const auto& offices_json = config_map_json.as_object().at("offices"sv);

        // Проверяю, что массив roads не пустой, если пустой то не стойт выполнять дальнейшие действия
        if (offices_json.as_array().empty()) {
            return;
        }

        for(auto coordinates_offices : offices_json.as_array()){
            // Проверяю, что элемент является объектом
            if (!coordinates_offices.is_object()) {
                std::string message = "Error: each offices must be an object"s;
                logger::LogEntryToConsole(json::object{}, message, boost::log::trivial::error);
                continue;
            }

            auto offices_obj = coordinates_offices.as_object();
            std::string id;

            if(auto found_id = offices_obj.find("id"sv);found_id != offices_obj.end()){
                id = found_id->value().as_string();
            } else {
                throw std::logic_error("Missing required 'id' field for office definition"s);
            }

            const std::vector<int> coordinates = GetCoordinates(
                                std::vector{"x"sv,"y"sv, "offsetX"sv, "offsetY"sv},offices_obj,"offices"sv
                            );

            model::Coord x{coordinates[0]};
            model::Coord y{coordinates[1]};
            model::Coord dx{coordinates[2]};
            model::Coord dy{coordinates[3]};

            map.AddOffice(
                model::Office(
                    model::Office::Id{id}
                    , model::Point{x,y}
                    , model::Offset{dx,dy}
                )
            );
        }

    }

    void AddLootTypesToMap(model::Map& map, const json::value& config_map_json){
        // Проверяю наличие ключа lootTypes
        if (!config_map_json.as_object().contains("lootTypes"sv)) {
            throw std::logic_error("Missing 'lootTypes' parameter in map configuration"s);
        }

        const auto& loot_types_json = config_map_json.as_object().at("lootTypes"sv);

        // Проверяю, что массив loot_types не пустой, если пустой то не стойт выполнять дальнейшие действия
        if (loot_types_json.as_array().empty()) {
            return;
        }

        for(auto loot : loot_types_json.as_array()){
            // Проверяю, что элемент является объектом
            if (!loot.is_object()) {
                throw std::logic_error("Each loot type must be an object"s);
            }

            json::object loot_obj = loot.as_object();

            try {
                map.AddLootType(GetLootType(loot_obj));
            } catch (const std::exception& e) {
                std::string message =  "Error processing loot type: " + std::string(e.what());
                logger::LogEntryToConsole(json::object{}, message, boost::log::trivial::error);
            }
        }

    }

    model::Game::GameSetting GetGameSetting(const json::object& config_map_obj) {
        model::Game::GameSetting setting;

        for( const auto& [key,value] : config_map_obj){
            if(key == "period"sv){
                setting.default_period = value.as_double();
            } else if (key == "probability"sv){
                setting.default_probability = value.as_double();
            }
        }

        // Проверяем корректность значений
        if (setting.default_period <= 0.0) {
            throw std::logic_error("Period must be positive"s);
        }

        if (setting.default_probability < 0.0 || setting.default_probability > 1.0) {
            throw std::logic_error("Probability must be in range 0, 1"s);
        }

        return setting;
    }

    model::Game LoadGame(const std::filesystem::path& json_path) {
        //читаем содиржимое json 
        std::string text_config(ReadFile(json_path));
        // парсим json
        auto config_map_json = json::parse(text_config);

        // Проверяем наличие ключа "maps"
        if (!config_map_json.as_object().contains("maps"sv)) {
            throw std::logic_error("Missing 'maps' key in root JSON object"s);
        }

        model::Game game;
        // Получаем массив карт
        const auto& maps_array = config_map_json.as_object().at("maps").as_array();

        // Проверяем наличие ключа, для установки скорости персонажа по умолчанию
        if(auto def_speed = config_map_json.as_object().find("defaultDogSpeed"s)
                                                            ; def_speed != config_map_json.as_object().end()) {
            model::DEFAULT_SPEED_ON_MAP = def_speed->value().as_double();
        }

        // Создаем объект настроек для игры
        model::Game::GameSetting setting;

        // Проверяем наличие ключа 'lootGeneratorConfig', при отсутствии ключа файл конфагурации являетсыя не валидным
        auto loot_generator_config = config_map_json.as_object().find("lootGeneratorConfig"sv);

        // Если ключ отсутствует кидаем исключение и выходим, т.к. файл конфигурации невалиден
        if (loot_generator_config == config_map_json.as_object().end()) {
            throw std::logic_error("Invalid config.json! Missing 'lootGeneratorConfig' key in root JSON object."s);
        }

        // Получаем итератор для получаения максимальной вместимости рюкзака
        const auto default_bag_capacity = config_map_json.as_object().find("defaultBagCapacity"sv);
        
        setting = GetGameSetting(loot_generator_config->value().as_object());

        // Проверяем наличие ключа, для установки максимальной вместимости рюкзака персонажа
        if(default_bag_capacity != config_map_json.as_object().end()){
            setting.default_bag_capacity = static_cast<size_t>(default_bag_capacity->value().as_uint64());
        }

        // Проверяем наличие ключа, для установки максимального времени бездействия персонажа
        if(auto def_player_retirement_time = config_map_json.as_object().find("dogRetirementTime"s)
                                            ; def_player_retirement_time != config_map_json.as_object().end()) {
            const double SECONDS_IN_MILLISECONDS = 1000.0;
            auto obj_value = def_player_retirement_time->value();

            if(obj_value.is_int64()) {
            setting.dog_retirement_time = static_cast<double>(obj_value.as_int64()) 
                                                                                        * SECONDS_IN_MILLISECONDS;
            } else if(obj_value.is_double()) {
                setting.dog_retirement_time = static_cast<double>(obj_value.as_double()) 
                                                                                        * SECONDS_IN_MILLISECONDS;
            }
        }

        game.SetGameSetting(setting);

        for(const auto& map_config : maps_array){
            // Проверяем, что элемент является объектом
            if (!map_config.is_object()) {
                throw std::logic_error("Each map must be an object");
            }
            
            const auto& map_config_obj = map_config.as_object();
            
            // Извлекаем обязательные поля
            if (!map_config_obj.contains("id"sv) || !map_config_obj.contains("name"sv)) {
                throw std::logic_error("Missing required 'id' or 'name' field for map");
            }

            std::string id{map_config.as_object().at("id").as_string()};
            std::string name{map_config.as_object().at("name").as_string()};

            // Добавляем id и name для карты
            model::Map map{model::Map::Id{id},name};

            // Если скорость на карте отличается от скорости по умолчанию, то устанавливаем скорость
            if(auto speed = map_config_obj.find("dogSpeed"s); speed != map_config_obj.end()){
                map.SetSpeedCharacters(speed->value().as_double());
            }

            // Если в настройках карты есть ключ bagCapacity, то устанавливаем это значение, иначе значение по умолчанию (==3)
            auto bag_capasity = map_config_obj.find("bagCapacity"s);
            bag_capasity != map_config_obj.end() ? map.SetBagCapacity(static_cast<size_t>(bag_capasity->value().as_uint64())) 
                                                : map.SetBagCapacity(game.GetGameSetting().default_bag_capacity);

            // Добавляем объекты на карту
            AddRoadsToMap(map, map_config);
            AddBuildingsToMap(map, map_config);
            AddOfficesToMap(map, map_config);
            AddLootTypesToMap(map,map_config);
            game.AddMap(std::move(map));

        }
        
        return game;
    }

}  // namespace json_loader
