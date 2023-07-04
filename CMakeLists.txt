cmake_minimum_required(VERSION 3.23)

project(somiks_image_processor CXX)

# suppress no debug files linker warning
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
endif()


# external packages
## Boost
find_package(Boost CONFIG REQUIRED)
set(LIBS ${LIBS} Boost::headers Boost::filesystem Boost::json Boost::program_options)

## Jpeg
find_package(JPEG REQUIRED)
set(LIBS ${LIBS} libjpeg-turbo::libjpeg-turbo)


# sources
set(APP_NONMAIN_SOURCES
    "src/application/managers/extension_manager.cpp"
    "src/application/managers/config_manager.cpp"
    "src/application/managers/options_manager.cpp"
    "src/application/managers/format_manager.cpp"
    "src/application/managers/algorithm_manager.cpp"
    "src/application/api.cpp")

# formats
file(GLOB FORMAT_SOURCES ${CMAKE_SOURCE_DIR}/src/formats/*.cpp)
foreach(FILE ${FORMAT_SOURCES})
  message("Found format source: ${FILE}")
endforeach()
# algorithms
file(GLOB ALGO_SOURCES ${CMAKE_SOURCE_DIR}/src/algorithms/*.cpp)
foreach(FILE ${ALGO_SOURCES})
  message("Found algorithm source: ${FILE}")
endforeach()

add_library(nonmain_files OBJECT ${APP_NONMAIN_SOURCES} ${FORMAT_SOURCES} ${ALGO_SOURCES})
target_link_libraries(nonmain_files ${LIBS})

# configuration files
file(GLOB APP_FORMAT_CONFIGS ${CMAKE_SOURCE_DIR}/src/formats/configs/*.json)
file(GLOB APP_ALGO_CONFIGS ${CMAKE_SOURCE_DIR}/src/algorithms/configs/*.json)
foreach(FILE ${APP_FORMAT_CONFIGS})
  message("Found format config: ${FILE}")
endforeach()
foreach(FILE ${APP_ALGO_CONFIGS})
  message("Found algorithm config: ${FILE}")
endforeach()

# add config to target
function(target_add_config target)
  get_target_property(CFG_OUTPUT ${target} RUNTIME_OUTPUT_DIRECTORY)
  if(${CFG_OUTPUT} STREQUAL "CFG_OUTPUT-NOTFOUND")
    set(CFG_OUTPUT ${CMAKE_BINARY_DIR})
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    _copy_config_files(${CFG_OUTPUT}/Debug)
    _copy_config_files(${CFG_OUTPUT}/Release)
  else()
    _copy_config_files(${CFG_OUTPUT})
  endif()

endfunction()

# copy config files to directory
function(_copy_config_files dest)
  # format configuration files
  foreach(FILE ${APP_FORMAT_CONFIGS})
    get_filename_component(NAME "${FILE}" NAME)
    configure_file("${FILE}" "${dest}/formats/${NAME}")
  endforeach()

  # algorithms configuration files
  foreach(FILE ${APP_ALGO_CONFIGS})
    get_filename_component(NAME "${FILE}" NAME)
    configure_file("${FILE}" "${dest}/algorithms/${NAME}")
  endforeach()
endfunction()

# executables
add_executable(app "src/application/main.cpp")
target_link_libraries(app nonmain_files)
target_add_config(app)

add_executable(ssimp "src/application/cli.cpp")
target_link_libraries(ssimp nonmain_files)
target_add_config(ssimp)

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
  target_add_config(${NAME})

  add_test(NAME ${NAME} COMMAND ${NAME})
endforeach()
