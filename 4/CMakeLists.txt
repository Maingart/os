cmake_minimum_required(VERSION 3.0.0)
project(Hello_World VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
find_package(Boost 1.84.0 REQUIRED)

set(logger
    main.cpp
)

set(emu
    emu.cpp
)

add_executable(emu ${emu})
add_executable(logger ${logger})

if (WIN32)
    target_link_libraries(emu PRIVATE ws2_32)
endif()
                                                        
target_include_directories(emu PRIVATE ${Boost_INCLUDE_DIRS})

                                                        