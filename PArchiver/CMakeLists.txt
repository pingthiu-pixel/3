cmake_minimum_required(VERSION 3.10)
project(PArchiver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск пакетов
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)
find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)

# Включаем директорию с заголовками
include_directories(${CMAKE_SOURCE_DIR}/include)

# Исходные файлы
set(SOURCES
    src/archiver_core.cpp
    src/ui_main_window.cpp
)

# Создаем исполняемый файл
add_executable(parchiver ${SOURCES})

# Подключаем библиотеки
target_link_libraries(parchiver 
    ${LIBARCHIVE_LIBRARIES}
    sfml-system
    sfml-window
    sfml-graphics
    sfml-network
)

target_include_directories(parchiver PRIVATE ${LIBARCHIVE_INCLUDE_DIRS})

# Установка флагов компиляции
target_compile_options(parchiver PRIVATE -Wall -Wextra -O2)

# Информация о проекте
set_target_properties(parchiver PROPERTIES
    OUTPUT_NAME "PArchiver"
    DESCRIPTION "Современный архиватор файлов с графическим интерфейсом"
)

# Инсталляция
install(TARGETS parchiver DESTINATION bin)
