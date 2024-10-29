cmake_minimum_required(VERSION 3.30)

project(waver LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COLOR_DIAGNOSTICS ON)


#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(boost 1.85.0 CONFIG REQUIRED COMPONENTS
        contracts
)

# find_package(GTest CONFIG REQUIRED)
add_executable(waver
        main.cpp
        include/net/ancillarycat/waver/waver.hpp
)
add_executable(test_cpp
        test.cpp
)

#include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(test_cpp PRIVATE
        nlohmann_json::nlohmann_json
        absl::base
        # fmt::fmt
)

target_link_libraries(waver PRIVATE
        nlohmann_json::nlohmann_json
        absl::base
        # fmt::fmt
        # Boost::boost
        # GTest::gtest
        # GTest::gtest_main
)

#include(CTest)
#enable_testing()

#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)
