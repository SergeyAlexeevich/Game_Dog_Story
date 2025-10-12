#include "sdk.h"

#include <functional>
#include <filesystem>
#include <iostream>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "application/application.h"
#include "software_options/parser.h"
#include "logging/logger.h"
#include "request/request_handler.h"
#include "server/http_server.h"
#include "work_with_json/json_loader.h"
#include "game_data_persistence/backup_restore_manager.h"
#include "database/database_connection_settings.h"
#include "database/database_invariants.h"
#include "database/database_exceptions.h"

#define FOR_LOCAL

namespace {

namespace sys = boost::system;
using namespace std::literals;
namespace net = boost::asio;
namespace http = boost::beast::http;
namespace fs = std::filesystem;
namespace keywords = boost::log::keywords;

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    prog_opt::Args args = prog_opt::ParseCommandLine(argc, argv);
    std::shared_ptr<data_persistence::BackupRestoreManager> backup_restore_manager;
    auto save_game = [&backup_restore_manager](const app::GameManager& manager) {
                                                backup_restore_manager->SaveGame(manager);
                                    };
    bool needed_save = false;

    try {
        // Создание LoggingRequestHandler через псевдоним типа
        using RequestHandlerType = http_handler::RequestHandler;
        using LoggingHandlerType = logger::LogRequestHandler<RequestHandlerType>;

        // 0. Устанавливаем логирование в консоль
        boost::log::add_common_attributes();

        boost::log::add_console_log(
            std::clog,
            keywords::format = &logger::JsonFormatter,
            boost::log::keywords::auto_flush = true
        );

        // 1. Прочитать из переменной среды url базы данных
        const unsigned num_threads = std::thread::hardware_concurrency();
        const char* db_url = std::getenv(db_invariants::DB_URL.c_str());
        if (!db_url) {
            throw db_ex::EmptyDatabaseUrl();
        }

        #ifndef FOR_LOCAL
            const unsigned num_connections = 10u;
            db_conn_settings::DbConnectrioSettings db_settings{num_connections, std::move(db_url)};
        #else
            const unsigned num_connections = 5u;
            db_conn_settings::DbConnectrioSettings db_settings{num_connections, std::move(db_url)};
        #endif

        // 2. Устанавливаем путь до статического контента
        fs::path root_path = args.www_root;
        // 3. Инициализируем io_context
        net::io_context ioc(num_threads);
        // 4. Устанавливаем флаг начальной позиции персонажей
        model::RANDOMIZE_SPAWN_POINTS = args.randomize_spawn_points;
        // 5. Загружаем карту и строим строим модель игры
        model::Game game = json_loader::LoadGame(args.config_file);
        // 6. Создаем экземпляр приложения и передаем игровую модель
            // 6.1 Создаем параметры экземпляра приложения
            std::optional<double> tick_period;
            std::optional<double> save_interval;
            fs::path root_save_path = args.state_file;
            bool auto_save_needed = false;

            // 6.2 Проверяем, был ли задан параметр с временным периодом(небходим для автотаймера) в командной строке
            if (args.tick_period != 0) {
                tick_period = static_cast<double>(args.tick_period);
            }

            // 6.3 Проверяем, был ли задан параметр с путем сохранения игрового состояния в командной строке
            if(!root_save_path.empty()){
                // 5.3.1 Устанавливаем флаги необходимости автосохранения и восстановления
                auto_save_needed = true;
                needed_save = true;
                // 6.3.3 Проверяем, был ли задан параметр с временным переодом(необходим для автосохранения 
                // в течении игрового процесса) в командной строке
                if (args.save_state_period != 0) {
                    save_interval = static_cast<double>(args.save_state_period);
                }
            } else {
                logger::LogEntryToConsole(
                    boost::json::object{
                        {"error", "Missing required configuration parameter"},
                        {"parameter", "state_file"},
                        {"status", "error"},
                        {"usage", "--state-file <path_to_save_file>"}
                    },
                    "State file path is required for saving game data. "
                    "Please provide the path using the --state-file command line option"s,
                    boost::log::trivial::error
                );
            }
        auto strand = boost::asio::make_strand(ioc);
            // 6.4 Создаем экземпляр приложения
            app::Application application(ioc, strand, std::move(game), tick_period, save_interval, db_settings);
            // 6.5 Устанавливаем параметры сохранения экземпляру приложения
            application.SetSaveNeeded(auto_save_needed);

        // 7. Создаем экземпляр backup_restore_manager
        if(needed_save) {
            backup_restore_manager = std::make_shared<data_persistence::BackupRestoreManager>(root_save_path, save_interval, auto_save_needed);
            // 7.1 Связываем сигналы в Application с слотами BackupRestoreManager
            backup_restore_manager->ConnectionToSignals(application.GetSerializeSignal(), application.GetRestoreSignal());
            // 7.2 Задаем восстановление
            if(auto_save_needed) {
                backup_restore_manager->Restore(application.GetManager(), application.GetGame());
                application.SetSavedGame(save_game);
            }
        }

        // 8. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, &application](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                std::string message("Signal "s + std::to_string(signal_number) + " signal_number"s);
                logger::LogEntryToConsole(boost::json::object{}, message);
                ioc.stop();
            }   
        });

        // 9. Создаём обработчик HTTP-запросов и связываем его с моделью игры, задаем путь до статического контента
        auto handler = std::make_shared<RequestHandlerType>(application, root_path, ioc, strand);
        auto logging_handler = std::make_shared<LoggingHandlerType>(*handler);
        // 10. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port}, [logging_handler](auto&& endp, auto&& req, auto&& send) {
            logging_handler->operator()(std::forward<decltype(endp)>(endp)
                            , std::forward<decltype(req)>(req)
                            , std::forward<decltype(send)>(send)
                        );
        });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        logger::LogEntryToConsole(
            boost::json::object{
                {"port"s,8080},
                {"address"s,"0.0.0.0"s},
            }
            , "Server has started"s);

        // 11. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            //ioc.run();
            try {
                // Ваш код с Boost.Asio
                ioc.run();
            } catch (const std::exception& e) {
                std::cerr << "Исключение в io_context: " << e.what() << std::endl;
               std::throw_with_nested(std::runtime_error("Ошибка в io_context"));
            } 
        });

        // 12 Сохранения состояния сервера
        if(!root_save_path.empty()) {
            backup_restore_manager->SetAutoSave(false);
            application.EmitSerializeSignal();            
        }

    } catch (const std::exception& ex) {    
        logger::LogEntryToConsole(boost::json::object{  {"exeption"s, ex.what()},
                                                        {"exit_code", EXIT_FAILURE}
                                                     },
                                  "Unrecoverable error: application will terminate"s,
                                  boost::log::trivial::fatal);
        net::io_context ioc;
        ioc.stop();

        if(needed_save) {
            backup_restore_manager->RenameBackupFile();
        }

        return EXIT_FAILURE;
    }

    // успешное завершение работы сервера
    logger::LogEntryToConsole(boost::json::object{ {"exit_code", EXIT_SUCCESS} },
                                                   "Server gracefully terminated"s,
                                                   boost::log::trivial::info
    );

}
