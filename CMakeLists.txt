cmake_minimum_required(VERSION 2.8.11)

project(bard)
set(PROJECT_VERSION 2.0.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unknown-pragmas -Wextra -pedantic -pedantic-errors -std=gnu99")

include_directories(${PROJECT_SOURCE_DIR}/inc)


# Test Dependencies

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Determine if we should link with librt for targets that use "clock_gettime"
  include(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
  if(NOT HAVE_CLOCK_GETTIME)
    find_library(LIBRT NAMES rt)
  endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(HBS heartbeats-simple)
  pkg_check_modules(ENERGYMON energymon-default)
endif()


# Library

# OVERFLOW flag, causing POET fixed point to check for overflows print error statements
if(${OVERFLOW})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPOET_MATH_OVERFLOW -DOVERFLOW_WARNING -DUNDERFLOW_WARNING")
endif()

# FIXED_POINT flag, for compiling the fixed point version of POET
if(${FIXED_POINT})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFIXED_POINT")
endif()

add_library(bard src/poet.c src/poet_config_linux.c)


# Binaries

add_executable(bard_idle src/bard_idle.c)


# Tests

add_executable(math_ut test/math_ut.c)

add_executable(poet_config_test test/poet_config_test.c)
target_link_libraries(poet_config_test bard pthread)

if (HBS_FOUND AND ENERGYMON_FOUND)
  include_directories(${HBS_INCLUDE_DIRS} ${ENERGYMON_INCLUDE_DIRS})

  add_executable(double_loop_test test/double_loop_test.c)
  target_link_libraries(double_loop_test bard -L${HBS_LIBDIR} ${HBS_LIBRARIES} -L${ENERGYMON_LIBDIR} ${ENERGYMON_STATIC_LIBRARIES} ${LIBRT})

  add_executable(processor_speed_test test/processor_speed_test.c)
  target_link_libraries(processor_speed_test bard -L${HBS_LIBDIR} ${HBS_LIBRARIES} -L${ENERGYMON_LIBDIR} ${ENERGYMON_STATIC_LIBRARIES} ${LIBRT})
endif()


# pkg-config

set(PKG_CONFIG_EXEC_PREFIX "\${prefix}")
set(PKG_CONFIG_LIBDIR "\${prefix}/lib")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/include/${PROJECT_NAME}")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

set(PKG_CONFIG_NAME "${PROJECT_NAME}")
set(PKG_CONFIG_DESCRIPTION "Performance with Optimal Energy Toolkit")
set(PKG_CONFIG_LIBS "-L\${libdir} -lbard")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig.in
  ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/bard.pc
)


# Install

install(TARGETS bard DESTINATION lib)
install(TARGETS bard_idle DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/poet.h ${CMAKE_CURRENT_SOURCE_DIR}/inc/poet_config.h ${CMAKE_CURRENT_SOURCE_DIR}/inc/poet_math.h DESTINATION include/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_BINARY_DIR}/pkgconfig/ DESTINATION lib/pkgconfig)


# Uninstall

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY
)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
