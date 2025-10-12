#pragma once

#include <string>

#include <boost/beast/http.hpp>
#include <boost/json.hpp>

namespace http_response {

    enum class UriRequestValidationResult {
        VALIDMAPSLIST               // Корректный запрос списка карт api
        , VALIDMAPBYID              // Корректный запрос конкретной карты api
        , INVALIDPATH               // Неверный путь запроса
        , METHODNOTALLOWED          // Неподдерживаемый HTTP-метод
        , ROOTDIRECTORY             // валидный url путь к директории
        , FILENOTFOUND              // не существующий файл
        , OUTSIDEROOTDIRECTORY      // путь к файлу оказался вне корневого каталога
        , VALIDPATHTOFILE           // путь к файлу валиден
        , UNAUTHORIZED              // пользователь с таким токен отсутствует
    };

    using UriRequestValidationResult::VALIDMAPSLIST;
    using UriRequestValidationResult::VALIDMAPBYID;
    using UriRequestValidationResult::INVALIDPATH;
    using UriRequestValidationResult::METHODNOTALLOWED;
    using UriRequestValidationResult::ROOTDIRECTORY;
    using UriRequestValidationResult::FILENOTFOUND;
    using UriRequestValidationResult::OUTSIDEROOTDIRECTORY;
    using UriRequestValidationResult::VALIDPATHTOFILE;
    using UriRequestValidationResult::UNAUTHORIZED;

    enum class ActionByGame{
        JOINGAME,                   // Вход в игру
        PLAYERLIST,                 // Получение списка игроков
        UNSUPPORTEDMEDIATYPE        // некорректный заголовок Content-Type
    };

    using ActionByGame::JOINGAME;
    using ActionByGame::PLAYERLIST;
    using ActionByGame::UNSUPPORTEDMEDIATYPE;

    using namespace std::literals;
    namespace beast = boost::beast;
    namespace http = beast::http;
    using StringResponse = http::response<http::string_body>;

} // http_response