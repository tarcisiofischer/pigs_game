find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(
    PigsGame
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
    characters/Pig.hpp
    characters/PigWithMatches.hpp

    collision/aabb.hpp
    collision/character_collision.hpp
    collision/CollisionRegion.hpp
    collision/common.hpp
    collision/enums.hpp
    collision/tilemap_collision.hpp

    Animation.hpp
    bitmap_font.hpp
    constants.hpp
    drawing.hpp
    random.hpp
    sdl_wrappers.cpp
    sdl_wrappers.hpp
    StateTimeout.hpp
    TransitionAnimation.hpp
    Vector2D.hpp
)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
target_link_libraries(PigsGame ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})
