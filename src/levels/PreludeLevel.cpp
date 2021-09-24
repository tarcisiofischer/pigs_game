#include <levels/PreludeLevel.hpp>

#include <characters/builder.hpp>
#include <characters/IGameCharacter.hpp>
#include <characters/Pig.hpp>

#include <TransitionAnimation.hpp>
#include <io.hpp>
#include <GameHandler.hpp>

PreludeLevel::PreludeLevel(GameHandler& game_handler)
    : map(load_map("maps/intro.map"))
    , characters(build_game_characters(game_handler.get_renderer(), map))
{
    prepare_script(this->characters, game_handler.get_transition_animation());
}

GameMap& PreludeLevel::get_map()
{
    return this->map;
}

std::vector<IGameCharacter*>& PreludeLevel::get_characters()
{
    return this->characters;
}

std::function<void()> PreludeLevel::get_collision_callback(int callback_collision_id, IGameCharacter* character)
{
    return nullptr;
}

void prepare_script(std::vector<IGameCharacter*> game_characters, TransitionAnimation& transition_animation)
{
    auto pig1 = dynamic_cast<Pig*>(game_characters[0]);
    auto pig2 = dynamic_cast<Pig*>(game_characters[1]);
    auto pig3 = dynamic_cast<Pig*>(game_characters[2]);

    auto pig1_color = RGBColor{0, 100, 0};
    auto pig2_color = RGBColor{250, 50, 50};
    auto pig3_color = RGBColor{20, 30, 250};

    transition_animation.register_transition_callback([&](){
        // transition_animation.register_transition_callback(nullptr);
    });
    auto on_finish_animation = [&](){
        transition_animation.reset();
    };

    #define _(T) std::make_unique<T>
    pig1->set_script({{
        {1, _(WalkTo)(130)},
        {5, _(FaceTo)(-1)},
        {10, _(Talk)(tr["Hah! It worked."], pig1_color)},
        {15, _(Talk)(tr["Boss'll like to know we managed to stole Otto's treasure"], pig1_color)},
        // {20, _(Talk)("heheheh.")},
        {25, _(WaitScriptEvent)(pig2, 15)},
        {30, _(Talk)(tr["Well... If you talk less and start working, it'll be faster"], pig1_color)},
        {35, _(WaitScriptEvent)(pig3, 15)},
        {40, _(Talk)(tr["Yesterday was his birthday, man."], pig1_color)},
        {45, _(Talk)(tr["He must be sleepin' after the party."], pig1_color)},
        {50, _(Talk)(tr["We'll have plenty of time to steal everything."], pig1_color)},
        {51, _(WaitScriptEvent)(pig2, 45)},
        {52, _(SetAngry)(true)},
        {55, _(WaitScriptEvent)(pig3, 40)},
        {60, _(Talk)(tr["Can you two stop the smalltalk..."], pig1_color)},
        {65, _(Talk)(tr["AND START WORKING??"], pig1_color)},
        {70, _(RunLambdaEvent)(on_finish_animation)},
    }});
    pig2->set_script({{
        {0, _(WaitTime)(1000.0)},
        {0, _(WalkTo)(100)},
        {5, _(FaceTo)(+1)},
        {10, _(WaitScriptEvent)(pig1, 15)},
        {15, _(Talk)(tr["Y-yeah, but it'll t-t-take some t-time to take all this g-gold f-f-from here..."], pig2_color)},
        {20, _(WaitScriptEvent)(pig3, 10)},
        {25, _(FaceTo)(-1)},
        {26, _(WaitScriptEvent)(pig1, 35)},
        {27, _(FaceTo)(+1)},
        {30, _(WaitScriptEvent)(pig1, 50)},
        {35, _(Talk)(tr["Y-yeah, b-but my back hu-hurts, you know?"], pig2_color)},
        {25, _(FaceTo)(-1)},
        {40, _(WaitScriptEvent)(pig3, 25)},
        {45, _(Talk)(tr["Did y-you see I've g-g-got this weird stain in my nose?"], pig2_color)},
        {50, _(WaitScriptEvent)(pig1, 55)},
        {55, _(FaceTo)(+1)},
        {60, _(WaitScriptEvent)(pig1, 60)},
        {65, _(SetFear)(true)},
        {70, _(WaitScriptEvent)(pig1, 65)},
    }});
    pig3->set_script({{
        {0, _(WaitTime)(500.0)},
        {1, _(WalkTo)(80)},
        {5, _(FaceTo)(+1)},
        {10, _(WaitScriptEvent)(pig1, 30)},
        {15, _(Talk)(tr["What if Otto wake up before we finish, boss?"], pig3_color)},
        {20, _(WaitScriptEvent)(pig2, 35)},
        {25, _(Talk)(tr["Now that you mentioned... My feet hurts a bit"], pig3_color)},
        {30, _(WaitScriptEvent)(pig2, 45)},
        {35, _(Talk)(tr["Wow! creepy, man. You should see a doctor."], pig3_color)},
        {40, _(Talk)(tr["My mother-in-law had a similar thing on she's nose and..."], pig3_color)},
        {45, _(WaitScriptEvent)(pig1, 60)},
        {50, _(SetFear)(true)},
        {55, _(WaitScriptEvent)(pig1, 65)},
    }});
    #undef _
}
