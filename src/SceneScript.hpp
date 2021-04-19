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

    virtual void run(SceneScript* script, int elapsed_time) = 0;
    bool is_finished();

protected:
    bool finished;
};

class WalkTo : public AbstractSceneHandler
{
public:
    WalkTo(IGameCharacter* c, int desired_position_x);
    void run(SceneScript* script, int elapsed_time);

private:
    IGameCharacter* character;
    int desired_position_x;
};

class FaceTo : public AbstractSceneHandler
{
public:
    FaceTo(IGameCharacter* c, int desired_face);
    void run(SceneScript* script, int elapsed_time);

private:
    IGameCharacter* character;
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

    Talk(IGameCharacter* c, std::string const& message);
    void run(SceneScript* script, int elapsed_time);

private:
    IGameCharacter* character;
    std::string message;
    TalkState state;
};

class WaitScriptEvent : public AbstractSceneHandler
{
public:
    WaitScriptEvent(IGameCharacter* c, int line_number);
    void run(SceneScript* script, int elapsed_time);

private:
    IGameCharacter* character;
    int line_number;
};

class SceneScript {
public:
    using ScriptLine = std::tuple<int, std::shared_ptr<AbstractSceneHandler>>;

    SceneScript(std::vector<ScriptLine> const& script);
    void run(int elapsed_time);
    int get_active_script_line() const;

private:
    std::vector<ScriptLine> full_script;
    int active_script_line;
};

class SceneScript;
#endif
