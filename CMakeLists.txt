cmake_minimum_required(VERSION 3.21)
project(ion-base VERSION 1.0 DESCRIPTION "ion base systems")
project(ion-game VERSION 1.0 DESCRIPTION "ion game components")
project(ion-development VERSION 1.0 DESCRIPTION "ion dev tools")
project(ion-run VERSION 1.0 DESCRIPTION "ion runner")
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

add_library(ion-base STATIC)
target_compile_features(ion-base PRIVATE cxx_std_20)
#target_compile_definitions(ion-base PRIVATE ION_EXPORTS)
target_sources(ion-base PRIVATE
  src/base/assets.cc
  src/base/context.cc
  src/base/shader.cc
  src/base/script.cc
  src/base/render.cc
  src/base/texture.cc
  src/base/physics.cc
  src/base/world.cc
)
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
)

add_library(ion-game STATIC)
target_compile_features(ion-game PRIVATE cxx_std_20)
#target_compile_definitions(ion-game PRIVATE ION_EXPORTS)
target_sources(ion-game PRIVATE
  src/game/game.cc
)
set_target_properties(ion-game PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion-game PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-game PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-game PUBLIC
    ion-base
)

add_executable(ion-development)
target_compile_features(ion-development PRIVATE cxx_std_20)
#target_compile_definitions(ion-development PRIVATE ION_EXPORTS)
target_sources(ion-development PRIVATE
  src/development/gui.cc
  src/development/write_to_disk.cc
  src/development/package.cc
  src/development/inspector.cc
  src/development/main.cc
)
set_target_properties(ion-development PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion-development PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-development PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-development PUBLIC
    imgui::imgui
    ion-base
    ion-game
)

add_executable(ion-run)
target_compile_features(ion-run PRIVATE cxx_std_20)
#target_compile_definitions(ion-run PRIVATE ION_IMPORTS)
target_sources(ion-run PRIVATE
  src/run/runner.cc
)
set_target_properties(ion-run PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(ion-run PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(ion-run PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-run PRIVATE
    ion-base
    ion-game
)
# ion-run should built in engine/tools/run
set_target_properties(ion-run PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/assets/tools/run)
set_target_properties(ion-run PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/assets/tools/run)