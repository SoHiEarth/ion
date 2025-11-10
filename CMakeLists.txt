cmake_minimum_required(VERSION 3.21)
project(ion VERSION 1.0 DESCRIPTION "ion")
find_package(box2d CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ImGui CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
add_executable(ion)
target_compile_features(ion PRIVATE cxx_std_20)
target_sources(ion PRIVATE
  src/main.cc
  src/shader.cc
  src/transform.cc
)
set_target_properties(ion PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ion PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(ion PRIVATE
    box2d::box2d
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    OpenAL::OpenAL
    OpenGL::GL
    tinyfiledialogs::tinyfiledialogs
)
