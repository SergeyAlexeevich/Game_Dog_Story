#pragma once

#include <filesystem>
#include <string>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include "../models/game.h"

namespace json_loader {

namespace net = boost::asio;

std::string ReadFile(const std::filesystem::path& path, bool binary_mode = false);

model::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
