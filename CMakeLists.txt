cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "svbb")
set(CURRENT_VERSION 0.1)
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION} LANGUAGES CXX)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(IS_TOPLEVEL_PROJECT TRUE)
else()
  set(IS_TOPLEVEL_PROJECT FALSE)
endif()

option(SVBB_BUILD_TESTING "Build and run SVBB tests " ${IS_TOPLEVEL_PROJECT})
option(SVBB_BUILD_EXAMPLES "Build and run SVBB Example executables " ${IS_TOPLEVEL_PROJECT})

include(GNUInstallDirs)


set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")
set(EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

set(INCLUDE_FILES 
    "${INCLUDE_DIR}/svbb.hpp"
    "${INCLUDE_DIR}/tokenize.hpp"
    "${INCLUDE_DIR}/config.hpp"
    "${INCLUDE_DIR}/trim.hpp"
    "${INCLUDE_DIR}/split.hpp"
    "${INCLUDE_DIR}/util.hpp"
    "${INCLUDE_DIR}/literals.hpp"
    "${INCLUDE_DIR}/char_traits.hpp"
	"${INCLUDE_DIR}/token_iterator.hpp"
)
set(TEST_FILES 
    "${TEST_DIR}/svbb.t.cpp"
    "${TEST_DIR}/tokenize.t.cpp"
    "${TEST_DIR}/trim.t.cpp"
    "${TEST_DIR}/split.t.cpp"
    "${TEST_DIR}/util.t.cpp"
	"${TEST_DIR}/token_iterator.t.cpp"
)

set(EXAMPLES
    "tokenize_for_loop"
    "tokenize_std_count"
)

set(EXAMPLES_CONSTEXPR
	"tokenize_constexpr"
)


add_library(${PROJECT_NAME} INTERFACE)
add_library(svbb::svbb ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_library(${PROJECT_NAME}_config INTERFACE)
add_library(svbb::config ALIAS ${PROJECT_NAME}_config)

if(MSVC)
	target_compile_options("${PROJECT_NAME}_config" INTERFACE "/W4" "/WX")

	if(MSVC_VERSION GREATER_EQUAL 1910)
		#target_compile_features("${PROJECT_NAME}_config" INTERFACE cxx_std_17)
		target_compile_options("${PROJECT_NAME}_config" INTERFACE "/permissive-" "/std:c++latest")
		target_compile_definitions("${PROJECT_NAME}_config" INTERFACE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	elseif(MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_NO_CXX14_CONSTEXPR)
		#target_compile_features("${PROJECT_NAME}_config" INTERFACE cxx_std_11)
		set(SVBB_USE_STX_STRING_VIEW ON)
	else()
		message(WARNING "Unsupported MSCV version.")
        target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_NO_CONSTEXPR SVBB_NO_NOEXCEPT)
		set(SVBB_USE_BOOST_STRING_VIEW ON)
	endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options("${PROJECT_NAME}_config" INTERFACE "-Wall" "-Werror" "-pedantic")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
		target_compile_features("${PROJECT_NAME}_config" INTERFACE cxx_std_17)
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
		target_compile_features("${PROJECT_NAME}_config" INTERFACE cxx_std_11)
        target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_NO_CXX14_CONSTEXPR)
		set(SVBB_USE_STX_STRING_VIEW ON)
	else()
		message(ERROR "Unsupported GCC version.")
	endif()
else()
	message(ERROR "Unsupported compiler.")
endif()

if(SVBB_CONSTEXPR_ALL_THE_THINGS)
    target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_CONSTEXPR_ALL_THE_THINGS)
endif()

if(SVBB_USE_BOOST_STRING_VIEW)
	find_package(Boost 1.61 REQUIRED QUIET)
	target_link_libraries("${PROJECT_NAME}_config" INTERFACE Boost::boost)
	target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_USE_BOOST_STRING_VIEW BOOST_ALL_NO_LIB)
elseif(SVBB_USE_STX_STRING_VIEW)
	target_compile_definitions("${PROJECT_NAME}_config" INTERFACE SVBB_USE_STX_STRING_VIEW)
	target_include_directories("${PROJECT_NAME}_config" INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extern)
endif()

if(SVBB_BUILD_TESTING)
	enable_testing()
    add_executable("${PROJECT_NAME}_test" ${INCLUDE_FILES} ${TEST_FILES} "${TEST_DIR}/test_config.hpp")
	target_link_libraries("${PROJECT_NAME}_test" PRIVATE svbb::svbb svbb::config)
	add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

if(SVBB_BUILD_EXAMPLES)
	foreach(_example ${EXAMPLES})
        add_executable("${_example}" "${EXAMPLE_DIR}/${_example}.cpp")
        target_link_libraries("${_example}" PRIVATE svbb::svbb svbb::config)
    endforeach()

    if(SVBB_CONSTEXPR_ALL_THE_THINGS)
        foreach(_example ${EXAMPLES_CONSTEXPR})
            add_executable("${_example}" "${EXAMPLE_DIR}/${_example}.cpp")
            target_link_libraries("${_example}" PRIVATE svbb::svbb svbb::config)
        endforeach()
    endif()
endif()


# locations are provided by GNUInstallDirs
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}_Targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(
    EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/svbb
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
