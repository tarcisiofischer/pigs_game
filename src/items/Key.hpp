#ifndef PIGSGAME_KEY_H
#define PIGSGAME_KEY_H

#include <characters/IGameCharacter.hpp>
#include <Animation.hpp>
#include <SceneScript.hpp>
#include <Vector2D.hpp>
#include <random.hpp>
#include <sdl_wrappers.hpp>

class Key : public IGameCharacter {
public:
    static auto constexpr collision_size = Vector2D<int> { 18, 18 };
    static auto constexpr IDLE_ANIMATION = 0;

    Key(SDL_Renderer* renderer, double pos_x, double pos_y);

    void update(double elapsedTime) override;
    void run_animation(double elapsedTime) override;
    void set_position(double x, double y) override;
    Vector2D<double> get_position() const override;
    Vector2D<double> get_velocity() const override;
    void set_velocity(double x, double y) override;
    void handle_collision(CollisionType const& type, CollisionSide const& side) override;
    CollisionRegionInformation get_collision_region_information() const override;
    void on_after_collision() override;

    void collect();

public:
    Vector2D<double> position;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    bool is_collected;
    std::map<int, Animation> animations;
};

#endif //PIGSGAME_KEY_H
