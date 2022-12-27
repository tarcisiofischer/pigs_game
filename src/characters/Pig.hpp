#ifndef __PIG_HPP
#define __PIG_HPP

#include <Animation.hpp>
#include <SceneScript.hpp>
#include <Vector2D.hpp>
#include <characters/IGameCharacter.hpp>
#include <random.hpp>
#include <sdl_wrappers.hpp>

// TODO PIG-12: Remove this
extern Vector2D<int> camera_offset;

class Pig : public IGameCharacter {
public:
    static auto constexpr IDLE_ANIMATION = 0;
    static auto constexpr RUNNING_ANIMATION = 1;
    static auto constexpr TAKING_DAMAGE_ANIMATION = 2;
    static auto constexpr DYING_ANIMATION = 3;
    static auto constexpr TALKING_ANIMATION = 4;
    static auto constexpr ANGRY_ANIMATION = 5;
    static auto constexpr ANGRY_TALKING_ANIMATION = 6;
    static auto constexpr FEAR_ANIMATION = 7;

    static auto constexpr collision_size = Vector2D<int> { 18, 18 };
    static auto constexpr SPRITESHEET_OFFSET = Vector2D<int> { 31, 33 };

    Pig(SDL_Renderer* renderer, double pos_x, double pos_y);
    Pig(Pig const& other);

    Pig& operator=(Pig const& other);

    virtual ~Pig();

    void set_script(SceneScript&& s);
    void set_position(double x, double y) override;
    Vector2D<double> get_position() const override;
    Vector2D<double> get_velocity() const override;
    void set_velocity(double x, double y) override;
    CollisionRegionInformation get_collision_region_information() const override;
    void handle_collision(CollisionType const& type, CollisionSide const& side) override;
    void on_after_collision() override;
    void update(double elapsedTime) override;
    void start_taking_damage();
    void run_animation(double elapsedTime) override;
    void think(double elapsedTime);
    int get_dynamic_property(int property_id) const;
    void run_left();
    void run_right();
    void stop();
    void turn_to(int face);
    void talk(std::string const& message, RGBColor const& talk_color);
    void set_angry(bool angry);
    void set_fear(bool fear);

private:
    void connect_callbacks();

public:
    int running_side;
    std::map<int, Animation> animations;
    int face;
    Vector2D<double> position;
    Vector2D<double> old_position;
    Vector2D<double> velocity;
    SDL_Renderer* renderer;
    SDL_Texture* spritesheet;
    double think_timeout;
    bool is_taking_damage;
    int life;
    bool is_dying;
    bool is_dead;
    bool is_talking;
    bool is_angry;
    bool is_fear;
    std::string talking_message;
    RGBColor talk_color;

    std::optional<std::function<void()>> on_start_taking_damage;
    std::optional<SceneScript> script;
    std::vector<std::function<void(int)>> post_animations;
};

#endif
