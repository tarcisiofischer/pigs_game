find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    main.cc

    bitmap_font.hpp
    constants.hpp
    drawing.cpp
    drawing.hpp

    characters/IGameCharacter.hpp
    characters/Cannon.cpp
    characters/Cannon.hpp
    characters/CannonBall.hpp
    characters/King.cpp
    characters/King.hpp
    characters/Pig.cpp
    characters/Pig.hpp
    characters/PigWithMatches.cpp
    characters/PigWithMatches.hpp

    collision/aabb.hpp
    collision/character_collision.hpp
    collision/CollisionRegion.hpp
    collision/enums.hpp
    collision/tilemap_collision.hpp
    collision/tilemap_collision.cpp

    Animation.cpp
    Animation.hpp
    bitmap_font.hpp
    constants.hpp
    drawing.cpp
    drawing.hpp
    GameMap.cpp
    GameMap.hpp
    io.cpp
    io.hpp
    random.hpp
    sdl_wrappers.cpp
    sdl_wrappers.hpp
    StateTimeout.cpp
    StateTimeout.hpp
    TransitionAnimation.cpp
    TransitionAnimation.hpp
    Vector2D.hpp
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

add_executable(
    MapEditor

    map_editor.cpp
    constants.hpp
    drawing.cpp
    drawing.hpp
    io.cpp
    io.hpp
    sdl_wrappers.cpp
    sdl_wrappers.hpp
    GameMap.cpp
    GameMap.hpp

    collision/aabb.hpp
)
target_compile_definitions(MapEditor PRIVATE FORCE_SCALE_SIZE=1)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
target_link_libraries(PigsGame ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})
target_link_libraries(MapEditor ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})
