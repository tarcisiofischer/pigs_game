find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(
   PigsGame
   main.cc

   bitmap_font.hpp
   constants.hpp
   drawing.hpp

   Animation.hpp
   CollisionRegion.hpp
   IGameCharacter.hpp
   Vector2D.hpp
)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
target_link_libraries(PigsGame ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})
