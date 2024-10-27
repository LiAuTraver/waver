cmake_minimum_required(VERSION 3.30)

project(waver LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# execute_process(
# 	COMMAND dev
# 	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# )

# "-DCMAKE_TOOLCHAIN_FILE=M:/vcpkg/scripts/buildsystems/vcpkg.cmake"
# set(CMAKE_TOOLCHAIN_FILE "M:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# add fmt, nlohmann/json, and gtest
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# find_package(GTest CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(boost COMPONENTS contracts CONFIG REQUIRED)
add_executable(waver
        main.cpp
        include/net/ancillarycat/waver/waver.hpp
        include/net/ancillarycat/waver/internal/config.hpp
        include/net/ancillarycat/waver/internal/vcd_fwd.hpp
        include/net/ancillarycat/waver/internal/lexer.hpp
        include/net/ancillarycat/waver/internal/vcd.hpp
        include/net/ancillarycat/waver/internal/meta_elements.hpp
)
add_executable(test test.cpp
        include/net/ancillarycat/waver/internal/meta_elements.hpp)

target_link_libraries(test
        PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        absl::base
)

target_link_libraries(waver
        PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        absl::base
        # GTest::gtest
        # GTest::gtest_main
)

# include(CTest)
# enable_testing()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
