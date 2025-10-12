#pragma once

#include "../models/game.h"
#include <string>

namespace json_converter{

    std::string GetMapListToJson(const model::Game& game);
    std::string GetFoundMapToJson(const model::Game& game, std::string_view id);
    std::string GetBadRequestToJson()noexcept;
    std::string GetNotFoundMapToJson() noexcept;
    std::string GetMethodNotAllowedJson() noexcept;

} //json_converter
