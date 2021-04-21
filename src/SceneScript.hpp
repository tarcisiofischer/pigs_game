#ifndef __SCENE_SCRIPT
#define __SCENE_SCRIPT

#include <characters/IGameCharacter.hpp>
#include <GameController.hpp>
#include <vector>
#include <tuple>
#include <memory>

auto constexpr SceneScriptLinePropertyId = 1;

class SceneScript;
class IGameCharacter;

class AbstractSceneHandler
{
public:
    AbstractSceneHandler();

    virtual void run(IGameCharacter* c, SceneScript* script, double elapsed_time) = 0;
    bool is_finished();

protected:
    bool finished;
};

class WaitTime : public AbstractSceneHandler
{
public:
    WaitTime(double desired_time);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    double desired_time;
    double current_time;
};


class WalkTo : public AbstractSceneHandler
{
public:
    WalkTo(int desired_position_x);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    int desired_position_x;
};

class SetAngry : public AbstractSceneHandler
{
public:
    SetAngry(bool is_angry);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    bool is_angry;
};

class SetFear : public AbstractSceneHandler
{
public:
    SetFear(bool fear);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    bool is_fear;
};

class FaceTo : public AbstractSceneHandler
{
public:
    FaceTo(int desired_face);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    int desired_face;
};

class Talk : public AbstractSceneHandler
{
public:
    enum class TalkState {
        NotStarted = 0,
        Talking = 1,
        Finished = 2
    };

    Talk(std::string const& message, RGBColor const& talk_color);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    std::string message;
    RGBColor talk_color;
    TalkState state;
};

class WaitScriptEvent : public AbstractSceneHandler
{
public:
    WaitScriptEvent(IGameCharacter* other_character, int line_number);
    void run(IGameCharacter* c, SceneScript* script, double elapsed_time);

private:
    IGameCharacter* other_character;
    int line_number;
};

class SceneScript {
public:
    using ScriptLine = std::tuple<int, std::shared_ptr<AbstractSceneHandler>>;

    SceneScript(std::vector<ScriptLine> const& script);
    void run(IGameCharacter* c, double elapsed_time);
    int get_active_script_line() const;

private:
    std::vector<ScriptLine> full_script;
    int active_script_line;
};

class SceneScript;
#endif
