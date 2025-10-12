#pragma once

#include <functional>
#include <string>
#include <mutex>

#include <boost/beast/http.hpp>

#include "../application/application.h"
#include "../models/game.h"
#include "../work_with_json/json_convert.h"
#include "utils.h"

namespace http_response {

    using namespace std::literals;

    using std::string;
    using StringResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;

    namespace beast = boost::beast;
    namespace http = beast::http;

    class Response {
    public:
    using SendFunction = std::function<void(StringResponse)>;

    explicit Response(app::Application& application) : application_(application) {
    }
    ~Response() = default;

    // Создание ответа
    static StringResponse CreateStringResponse(http::status status
                                        , unsigned http_version
                                        , std::string_view body
                                        , bool keep_alive
                                        , std::string_view content_type
                                        , std::string_view cache_control
                                        , std::string_view allow);

    static FileResponse CreateFileResponse(http::status status
                                            , unsigned http_version
                                            , http::file_body::value_type& file
                                            , bool keep_alive
                                            , std::string_view content_type
                                            , std::string_view cache_control) ;

    private:    
    static string GetBody(const string& body, const model::Game& game);

    private:
    app::Application& application_;
    SendFunction send_;
    static std::mutex mtx_for_file_;
    static std::mutex mtx_for_string_;
    };
    
} // http_response