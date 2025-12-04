cmake_minimum_required(VERSION 3.21)
project(ion VERSION 1.0 DESCRIPTION "ion")
project(ion-base VERSION 1.0 DESCRIPTION "ion base systems")
project(ion-game VERSION 1.0 DESCRIPTION "ion game components")
project(ion-development VERSION 1.0 DESCRIPTION "ion dev tools")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
find_package(box2d CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ImGui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(Python3 COMPONENTS Development REQUIRED)

add_library(ion-development STATIC
  src/development/gui.cc
 "include/game/game.h")
target_compile_features(ion-development PRIVATE cxx_std_20)
#target_compile_definitions(ion-development PRIVATE ION_EXPORTS)
set_target_properties(ion-development PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion-development PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-development PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-development PUBLIC
    glfw
    imgui::imgui
    OpenGL::GL
)

add_library(ion-base STATIC
  src/assets.cc
  src/context.cc
  src/shader.cc
  src/script.cc
  src/render.cc
  src/texture.cc
  src/physics.cc
  src/world.cc
  src/world_inspector.cc
)
target_compile_features(ion-base PRIVATE cxx_std_20)
#target_compile_definitions(ion-base PRIVATE ION_EXPORTS)
set_target_properties(ion-base PROPERTIES PRIVATE ${PROJECT_VERSION})
set_target_properties(ion-base PROPERTIES PRIVATE ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-base PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-base PUBLIC
    box2d::box2d
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    OpenGL::GL
    tinyfiledialogs::tinyfiledialogs
    Python3::Python
    ion-development
)

add_library(ion-game STATIC
  src/game/game.cc
  src/game/player.cc)
target_compile_features(ion-game PRIVATE cxx_std_20)
#target_compile_definitions(ion-game PRIVATE ION_EXPORTS)
set_target_properties(ion-game PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion-game PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-game PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-game PUBLIC
    ion-base
)

add_executable(ion)
target_compile_features(ion PRIVATE cxx_std_20)
#target_compile_definitions(ion PRIVATE ION_IMPORTS)
target_sources(ion PRIVATE
  src/main.cc
 "include/game/game.h")
set_target_properties(ion PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ion PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(ion PRIVATE
    ion-base
    ion-game
    ion-development
)