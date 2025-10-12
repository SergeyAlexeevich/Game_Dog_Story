#include "http_server.h"

#include <iostream>

#include <boost/asio/dispatch.hpp>

#include "../request/request_handler.h"

namespace http_server {

using namespace std::literals;

void ReportError(beast::error_code ec, std::string_view what) {
    std::cerr << what << ": "sv << ec.message() << std::endl;
}

//---------------------------------------------------------------------------------------------------- методы SessionBase
    void SessionBase::Run() {
    // Вызываем метод Read, используя executor объекта stream_.
    // Таким образом вся работа со stream_ будет выполняться, используя его executor
    net::dispatch(stream_.get_executor(),
                  beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

tcp::endpoint SessionBase::GetEndpoint() const {
  return stream_.socket().remote_endpoint();
}

    void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
        if (ec) {
            return ReportError(ec, "write"sv);
        }

        if (close) {
            // Семантика ответа требует закрыть соединение
            return Close();
        }

        // Считываем следующий запрос
        Read();
    }

    void SessionBase::Read() {
        /* Асинхронное чтение запроса */
        // Очищаем запрос от прежнего значения (метод Read может быть вызван несколько раз)
        request_ = {};
        stream_.expires_after(30s);
        // Считываем request_ из stream_, используя buffer_ для хранения считанных данных
        http::async_read(stream_, buffer_, request_,
                         // По окончании операции будет вызван метод OnRead
                         beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
    }

    void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
        if (ec == http::error::end_of_stream) {
            // Нормальная ситуация - клиент закрыл соединение
            return Close();
        }

        if (ec) {
            // Произошла ошибка чтения, выводит в её в stdout с помощью ReportError.
            return ReportError(ec, "read"sv);
        }

        // Запрос прочитан без ошибок, делегируйте его обработку классу-наследнику
        HandleRequest(std::move(request_));
    }

    void SessionBase::Close() {
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        
        if (ec && ec != beast::errc::not_connected) {
            throw boost::system::system_error(ec, "Error when fully closing the socket"s);
        }
    }


}  // namespace http_server