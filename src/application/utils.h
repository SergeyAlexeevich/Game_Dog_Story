#pragma once

#include <string>

#include "../models/map.h"

namespace app {

    // конвертация строки в число как есть например MAP == 836586
    inline size_t Stoi(const std::string& number) {
        size_t digit = 0;

        for(auto ch : number){
            digit += static_cast<size_t>(ch);
        }

        return digit;
    };

    enum class SessionStatus{
        FREE,
        FULL
    };

    using SessionStatus::FREE;
    using SessionStatus::FULL;

    struct GameSessionsHasher{ // по статусу и id карты
        size_t operator()(const std::pair<SessionStatus, model::Map::Id>& value)const{
            // Получаем числовое значение статуса сессии
            const size_t status = static_cast<size_t>(value.first);
                
            // Получаем идентификатор карты
            const model::Map::Id& mapId = value.second;
            size_t map_id_number = 0;

            for(auto ch : *mapId){
                map_id_number += static_cast<size_t>(ch);
            }
                
            // Комбинируем хеши с использованием XOR и сдвигов
            return (map_id_number * 17) ^ (status * 31) ^ (map_id_number << 5);
        }
    };

} // app