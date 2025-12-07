cmake_minimum_required(VERSION 3.21)
project(ion VERSION 1.0 DESCRIPTION "ion")
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
find_package(pugixml CONFIG REQUIRED)

add_library(ion-base SHARED)
target_compile_features(ion-base PRIVATE cxx_std_20)
target_compile_definitions(ion-base PRIVATE ION_EXPORTS)
target_compile_definitions(ion-base PRIVATE ION_BUILD_VERSION="${PROJECT_VERSION}")
target_sources(ion-base PRIVATE
  src/base/assets.cc
  src/base/base_pipeline.cc
  src/base/defaults.cc
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
    pugixml::pugixml
)

add_library(ion-game SHARED)
target_compile_features(ion-game PRIVATE cxx_std_20)
target_compile_definitions(ion-game PRIVATE ION_EXPORTS)
target_compile_definitions(ion-game PRIVATE ION_BUILD_VERSION="${PROJECT_VERSION}")
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
target_sources(ion-development PRIVATE
  src/development/gui.cc
  src/development/package.cc
  src/development/inspector.cc
  src/development/main.cc
  src/development/startwindow.cc
)
target_compile_definitions(ion-development PRIVATE
  ION_BUILD_VERSION="${PROJECT_VERSION}"
  ION_DEVELOPMENT
  ION_IMPORTS
)
set_target_properties(ion-development PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(ion-development PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-development PUBLIC
    imgui::imgui
    ion-base
    ion-game
)

add_executable(ion-run)
target_compile_features(ion-run PRIVATE cxx_std_20)
target_sources(ion-run PRIVATE
  src/run/runner.cc
)
target_compile_definitions(ion-run PRIVATE ION_BUILD_VERSION="${PROJECT_VERSION}")
set_target_properties(ion-run PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(ion-run PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ion-run PRIVATE
    ion-base
    ion-game
)
set_target_properties(ion-run PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/assets/tools/run
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/assets/tools/run
)