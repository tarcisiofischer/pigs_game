find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(pybind11 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    main.cpp

    bitmap_font.hpp
    constants.hpp
    drawing.cpp
    drawing.hpp

    characters/builder.cpp
    characters/builder.hpp
    characters/IGameCharacter.hpp
    characters/Cannon.cpp
    characters/Cannon.hpp
    characters/CannonBall.hpp
    characters/Pig.cpp
    characters/Pig.hpp
    characters/PigWithMatches.cpp
    characters/PigWithMatches.hpp
    characters/Liv.cpp
    characters/Liv.hpp

    items/Key.cpp
    items/Key.hpp

    collision/aabb.hpp
    collision/character_collision.cpp
    collision/character_collision.hpp
    collision/CollisionRegion.hpp
    collision/enums.hpp
    collision/tilemap_collision.hpp
    collision/tilemap_collision.cpp

    levels/IGameLevel.hpp
    levels/EntryLevel.hpp
    levels/EntryLevel.cpp
    levels/Level2.hpp
    levels/Level2.cpp
    levels/PreludeLevel.hpp
    levels/PreludeLevel.cpp

    screens/IScreen.hpp
    screens/TitleScreen.hpp
    screens/TitleScreen.cpp
    screens/GameScreen.hpp
    screens/GameScreen.cpp

    Animation.cpp
    Animation.hpp
    AssetsRegistry.cpp
    AssetsRegistry.hpp
    bitmap_font.hpp
    constants.hpp
    drawing.cpp
    drawing.hpp
    GameController.cpp
    GameController.hpp
    GameHandler.cpp
    GameHandler.hpp
    GameTimeHandler.cpp
    GameTimeHandler.hpp
    GameMap.cpp
    GameMap.hpp
    io.cpp
    io.hpp
    logging.hpp
    random.hpp
    sdl_wrappers.cpp
    sdl_wrappers.hpp
    SoundHandler.cpp
    SoundHandler.hpp
    SceneScript.cpp
    SceneScript.hpp
    StateTimeout.cpp
    StateTimeout.hpp
    TransitionAnimation.cpp
    TransitionAnimation.hpp
    Vector2D.hpp
    WindowShaker.cpp
    WindowShaker.hpp
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES ${SOURCE_FILES}
)

add_executable(
    PigsGame
    ${SOURCE_FILES}
)

target_link_libraries(
PigsGame
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
)
set_target_properties(PigsGame
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeMono.ttf DESTINATION ${CMAKE_BINARY_DIR}/bin/FreeMono.ttf)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ DESTINATION ${CMAKE_BINARY_DIR}/bin/assets/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../maps/ DESTINATION ${CMAKE_BINARY_DIR}/bin/maps/)

add_library(
    pyPigs
        SHARED
        pyPigs.cpp
)

target_link_libraries(
    pyPigs
        PRIVATE
        pybind11::module
)

set_target_properties(
    pyPigs
        PROPERTIES
        OUTPUT_NAME pyPigs
        PREFIX ""
)
