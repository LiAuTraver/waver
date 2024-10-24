cmake_minimum_required(VERSION 3.30)

project(waver LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# "-DCMAKE_TOOLCHAIN_FILE=M:/vcpkg/scripts/buildsystems/vcpkg.cmake"
set(CMAKE_TOOLCHAIN_FILE "M:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# add fmt, nlohmann/json, and gtest
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(waver
	main.cpp
)

target_link_libraries(waver
	PRIVATE
	fmt::fmt
	nlohmann_json::nlohmann_json

	# GTest::gtest
	# GTest::gtest_main
)

# include(CTest)
# enable_testing()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
