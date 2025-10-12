cmake_minimum_required(VERSION 3.15)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# Подключение Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

#_________________________________________________________библиотека обработки столкновений персанажей с предметами
# Создаем библиотеку для исключения дублирования кода и упращения тестирования
add_library(CollisionDetectionLib STATIC
	src/physics/collision_detector.h
	src/physics/collision_detector.cpp
	src/physics/geom.h
)

# Добавляем пути включения для библиотеки
target_include_directories(CollisionDetectionLib PUBLIC src/physics src)

# Добавляем внешние зависимости для библиотеки
target_link_libraries(CollisionDetectionLib PUBLIC CONAN_PKG::boost Threads::Threads)

#_________________________________________________________библиотека с игровыми моделями
# Модели
set(MODELS_SOURCES
    src/models/game.cpp
    src/models/map.cpp
    src/models/game_session.cpp
    src/models/dog.cpp
    src/models/map_roads.cpp
    src/models/loot_generator.cpp
    src/models/lost_object.cpp
    src/models/bag.cpp
    src/database/use_cases_impl.cpp
    src/database/postgres.cpp
)

# Создаем библиотеку для исключения дублирования кода и упращения тестирования
add_library(GameModelsLib STATIC ${MODELS_SOURCES} )

# Добавляем пути включения для библиотеки
target_include_directories(GameModelsLib PUBLIC src/models src/database src)

# Добавляем внешние зависимости для библиотеки
target_link_libraries(GameModelsLib PUBLIC CONAN_PKG::boost CollisionDetectionLib CONAN_PKG::libpqxx)

#_________________________________________________________библиотека сериализации/десериализации
# Сериализация
set(SERIALIZATION
    src/serialization_game/dog_serialization.cpp
    src/serialization_game/bag_serialization.cpp
    src/serialization_game/lost_object_serialization.cpp
    src/serialization_game/player_serialization.cpp
    src/serialization_game/game_session_serialization.cpp
    src/serialization_game/game_manager_serialization.cpp
)

# Создаем библиотеку для исключения дублирования кода и упращения тестирования
add_library(SerializationLib STATIC ${SERIALIZATION})

# Добавляем пути включения для библиотеки
target_include_directories(SerializationLib PUBLIC src/serialization_game src)

# Добавляем внешние зависимости для библиотеки
target_link_libraries(SerializationLib PUBLIC CONAN_PKG::boost Threads::Threads GameModelsLib)


# Настройка поддержки потоков
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Корневые исходники (только основные файлы)
set(ROOT_SOURCES
    src/main.cpp
    src/boost_json.cpp
)

# Обработчики запросов (только основные файлы)
set(REQUEST_SOURCES
    src/request/api_handler_game.cpp
    src/request/api_handler_maps_info.cpp
    src/request/static_file_handler.cpp
)

# Приложение
set(APPLICATION_SOURCES
    src/application/game_manager.cpp
    src/application/application.cpp
    src/application/collision_manager.cpp
)

# Доменные сущности
set(DOMEN_SOURCES
    src/domain_models/player.cpp
)

# Контроль доступа
set(ACCESS_SOURCES 
    src/access/access_control.cpp
)

# Логирование
set(LOGGING_SOURCES
    src/logging/logger.cpp
)

# Обработка ответов
set(RESPONSE_SOURCES
    src/response/response.cpp
)

# server
set(SERVER_SOURCES
    src/server/http_server.cpp
)

# Работа с JSON
set(JSON_SOURCES
    src/work_with_json/json_loader.cpp
    src/work_with_json/json_convert.cpp
)

# Работа с командной строкой
set(SOFT_OPT_SOURCES
    src/software_options/parser.cpp
)

# Работа с времем игры
set(TIME_MENAG_SOURCES
    src/time_management/ticker.cpp
)

# Сохранение и восстановление данных игры
set(GAME_DATA_PERSISTENCE_SOURSE
    src/game_data_persistence/backup_restore_manager.cpp
)

# Добавляем заголовочные файлы
target_include_directories(GameModelsLib PUBLIC
    src/
    src/application/
    src/access/
    src/domain_models/
    src/models
    src/logging/
    src/response/
    src/request/
    src/server/
    src/software_options/
    src/work_with_json/
    src/game_data_persistence/
)

# Создание исполняемого файла игры
add_executable(game_server
    ${SOFT_OPT_SOURCES}
    ${TIME_MENAG_SOURCES}
    ${ROOT_SOURCES}
    ${ACCESS_SOURCES}
    ${REQUEST_SOURCES}
    ${SERVER_SOURCES}
    ${APPLICATION_SOURCES}
    ${LOGGING_SOURCES}
    ${RESPONSE_SOURCES}
    ${JSON_SOURCES} 
    ${DOMEN_SOURCES}  
    ${GAME_DATA_PERSISTENCE_SOURSE}
)

# Добавляем зависимость целей от статической библиотеки.
target_link_libraries(game_server PRIVATE Threads::Threads SerializationLib )

include(CTest)
enable_testing()
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)

#________________________________________________________________________________тесты для генератора лутов
# Создание исполняемого файла тестов генератора лутов
add_executable(catch_tests_loot_gen
	tests/loot_generator_tests.cpp
)

# Добавляем внешние зависимости для тестов
target_link_libraries(catch_tests_loot_gen PRIVATE CONAN_PKG::catch2 GameModelsLib)

# Настройка обнаружения тестов
catch_discover_tests(catch_tests_loot_gen)

#________________________________________________________________________________тесты для "определения столкновения с предметом"
# Создание исполняемого файла тестов
add_executable(collision_detection_tests
	tests/collision-detector-tests.cpp
)

# Добавляем внешние зависимости для тестов
target_link_libraries(collision_detection_tests CONAN_PKG::catch2 CollisionDetectionLib)

# Настройка обнаружения тестов
catch_discover_tests(collision_detection_tests)
#________________________________________________________________________________тесты для "сериализации состояния игры"
# Создание исполняемого файла тестов
add_executable(serialization_tests
	tests/state-serialization-tests.cpp
    ${JSON_SOURCES} 
    ${LOGGING_SOURCES}
    ${DOMEN_SOURCES}
    ${APPLICATION_SOURCES}
)

# Добавляем внешние зависимости для тестов
target_link_libraries(serialization_tests CONAN_PKG::catch2 SerializationLib)

# Настройка обнаружения тестов
catch_discover_tests(serialization_tests)

#-------------------------------------------------------------------------------------------------------
# Boost.Beast будет использовать std::string_view вместо boost::string_view
add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

#-------------------------------------------------------------------------------------------------------
# Определяем тип сборки и сразу применяем соответствующие флаги
if(NOT CMAKE_BUILD_TYPE)
    # Если тип сборки не задан, устанавливаем Release по умолчанию
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# Общие флаги компиляции
target_compile_options(game_server PRIVATE
    -Wall               # все предупреждения
    -Wextra             # дополнительные предупреждения
)

# Настройки компиляции в зависимости от типа сборки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(game_server PRIVATE
        -g3                 # (максимальная отладочная информация)
        -O0                 # (без оптимизации)
   )
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # Настройки для Release с отладочной информацией
    target_compile_options(game_server PRIVATE
        -g                  # (базовая отладочная информация)
        -O2                 # (оптимизация производительности)
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(game_server PRIVATE
        -Os                  # оптимизация размера кода и данных (размер важнее скорости)
        -flto                # оптимизация времени компоновки (Link Time Optimization)
        -ffunction-sections  # размещение каждой функции в отдельном разделе ELF
        -fdata-sections      # размещение каждой переменной в отдельном разделе ELF
        -Wl,--gc-sections    # сборка мусора для секций (удаляет неиспользуемые секции)
    )
else()
    target_compile_options(game_server PRIVATE
        -O2                 # (оптимизация производительности)
    )
endif()