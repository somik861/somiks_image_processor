cmake_minimum_required(VERSION 3.23)

project(somiks_image_processor CXX)

find_package(Boost CONFIG REQUIRED)
set(LIBS ${LIBS} Boost::headers Boost::filesystem Boost::json)

set(APP_NONMAIN_SOURCES src/application/extension_matcher.cpp
                        src/application/config_manager.cpp)

add_library(nonmain_files OBJECT ${APP_NONMAIN_SOURCES})
target_link_libraries(nonmain_files ${LIBS})

add_executable(app src/application/main.cpp)
target_link_libraries(app nonmain_files)

# tests
enable_testing()

find_package(Catch2 CONFIG REQUIRED)

file(GLOB_RECURSE TEST_SRCS ${CMAKE_SOURCE_DIR}/tests/*.cpp)

foreach(TEST_SRC ${TEST_SRCS})
  message("Found test: ${TEST_SRC}")
  get_filename_component(NAME ${TEST_SRC} NAME_WLE)

  add_executable(${NAME} ${TEST_SRC})
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           ${CMAKE_BINARY_DIR}/tests/)
  target_link_libraries(${NAME} nonmain_files Catch2::Catch2WithMain)

  add_test(NAME ${NAME} COMMAND ${NAME})
endforeach()
