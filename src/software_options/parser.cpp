#include "parser.h"

#include <iostream>

#include <boost/program_options.hpp>

#include "../logging/logger.h"

namespace prog_opt {

    using namespace std::literals;

    [[nodiscard]] Args ParseCommandLine(int argc, const char* const argv[]) {
        namespace po = boost::program_options;

        po::options_description desc{"All options"s};
        Args args;

        desc.add_options()
            ("help,h", "produce help message")
            ("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s), "set tick period")
            ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
            ("www-root,w", po::value(&args.www_root)->value_name("dir"s), "set static files root")
            ("randomize-spawn-points", po::value(&args.randomize_spawn_points), "spawn dogs at random positions")
            ("state-file", po::value(&args.state_file)->value_name("file"s), "set file for save and restore game state")
            ("save-state-period", po::value(&args.save_state_period)->value_name("milliseconds"s), "set save game state period");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help"s)) {
            std::cout << desc;
            std::exit(0);
        }

        if (!vm.contains("config-file"s)) {
            std::string error_msg = "Config file have not been specified"s;
            std::string www_root;

            // Добавим доп проверку для более информативного сообщения
            if(!vm.contains("www-root"s)){
                www_root = " --www-root <path-to-static-files>"s;
            }

            // Используемый синтаксис
            std::string usage_syntax = "game_server "s + www_root;

            // Пример использования
            const std::string usage_example = "Example: game_server --config-file /data/config.json --www-root static"s;

            logger::LogEntryToConsole(boost::json::object{  {"Usage"s, usage_syntax},
                                                            {"Example"s, usage_example},
                                                            {"exit_code", EXIT_FAILURE}  
                }
                , error_msg
                , boost::log::trivial::error);

            throw ConfigFileNotSpecifiedException();
        }

        if (!vm.contains("www-root"s)) {
            std::string error_msg = "Static content path is not specified"s;
            // Используемый синтаксис
            std::string usage_syntax = "game_server --config-file /data/config.json"s;
            // Пример использования
            const std::string usage_example = "Example: game_server --config-file /data/config.json --www-root static"s;
            logger::LogEntryToConsole(boost::json::object{  {"Usage"s, usage_syntax},
                                                            {"Example"s, usage_example},
                                                            {"exit_code", EXIT_FAILURE}  
                }
                , error_msg
                , boost::log::trivial::error);

            throw StaticContentPathNotSpecifiedException();
        }

        return args;
    };

}