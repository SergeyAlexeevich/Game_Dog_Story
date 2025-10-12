#include "response.h"


namespace http_response {
    std::mutex http_response::Response::mtx_for_file_;
    std::mutex http_response::Response::mtx_for_string_;

    StringResponse Response::CreateStringResponse(http::status status
                                            , unsigned http_version
                                            , std::string_view body
                                            , bool keep_alive
                                            , std::string_view content_type
                                            , std::string_view cache_control
                                            , std::string_view allow) {

        std::lock_guard lgd(mtx_for_string_);
        // Формирую ответ со статусом и версией равной версии запроса
        StringResponse response(status, http_version);
        // Добавляю заголовок Content-Type: application/json
        response.set(http::field::content_type, content_type);
        
        if(!cache_control.empty()){
            response.set(http::field::cache_control, cache_control);
        }

        if(!allow.empty()){
            response.set(http::field::allow, allow);
        }

        if(!body.empty()){
            response.body() = std::move(body);
        }

        // Формирую заголовок Content-Length, сообщающий длину тела ответа
        response.content_length(body.size());
        // Формирую заголовок поддержания соединения (Connection) в зависимости от значения заголовка в запросе
        response.keep_alive(keep_alive);

        // Возвращаю сформированный ответ
        return response;
    }

    FileResponse Response::CreateFileResponse(http::status status, unsigned http_version
                                                    , http::file_body::value_type &file
                                                    , bool keep_alive
                                                    , std::string_view content_type
                                                    , std::string_view cache_control)  {

        std::lock_guard lgd(mtx_for_file_);
        FileResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = std::move(file);
        response.prepare_payload();
        response.keep_alive(keep_alive);

        if (!cache_control.empty()) {
            response.set(http::field::cache_control, cache_control);
        }

        return response;
    }

    std::string Response::GetBody(const std::string &body, const model::Game& game) {
        if(body.empty()){
            return json_converter::GetMapListToJson(game);
        }

        return body;
    }

} // http_response