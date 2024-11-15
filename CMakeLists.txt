cmake_minimum_required(VERSION 3.30)

project(waver VERSION 1.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_TOOLCHAIN_FILE "M:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# include(cmake/cpm_fwd.cmake)
set(WAVER_USE_BOOST_CONTRACT ON)
set(WAVER_DEV_MODE ON)

if(DEFINED WAVER_USE_CPM)
	CPMAddPackage("gh:nlohmann/json@3.11.3")
	CPMAddPackage("gh:abseil/abseil-cpp@20240116.2")
endif()

if(WAVER_USE_BOOST_CONTRACT)
	# lower-case `boost` for header-only libraries
	find_package(boost CONFIG REQUIRED COMPONENTS
		contracts
	)
	set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME OFF)
	add_compile_options(-DWAVER_USE_BOOST_CONTRACT)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

include_directories(include)

add_executable(waver
	main.cpp
)
target_link_libraries(waver PRIVATE
	nlohmann_json::nlohmann_json
	absl::base
	fmt::fmt
)

if(DEFINED WAVER_DEV_MODE)
	# Upper-case `Boost` for non-header-only libraries(needs to link)
	find_package(Boost CONFIG REQUIRED COMPONENTS
		program_options
	)
	add_executable(waver_test
		test/test.cpp
	)

	target_link_libraries(waver_test PRIVATE
		GTest::gmock_main
		GTest::gtest_main
		nlohmann_json::nlohmann_json
		absl::base
		fmt::fmt
	)

	add_executable(mytest2
		mytest2.cpp
	)
	target_link_libraries(mytest2 PRIVATE
		Boost::program_options
	)
endif(DEFINED WAVER_DEV_MODE)

# include(CTest)
# enable_testing()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
