cmake_minimum_required(VERSION 3.23)

project(somiks_image_processor CXX)

find_package(Boost CONFIG REQUIRED)
set(LIBS ${LIBS} Boost::headers)

add_executable(app src/application/main.cpp src/application/extension_matcher.cpp)
target_link_libraries(app ${LIBS})
