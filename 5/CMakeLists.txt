cmake_minimum_required(VERSION 3.0.0)
project(Hello_World VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
find_package(Boost 1.84.0 REQUIRED)

find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

set(logger
        main.cpp
        Logger.h
)

add_executable(logger ${logger})
target_link_libraries(logger PRIVATE ${Boost_LIBRARIES} SQLite::SQLite3)
