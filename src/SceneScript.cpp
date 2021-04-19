#include <SceneScript.hpp>
#include <characters/Pig.hpp>

#include <iostream>

AbstractSceneHandler::AbstractSceneHandler()
    : finished(false)
{}

bool AbstractSceneHandler::is_finished() {
    return this->finished;
}

WalkTo::WalkTo(IGameCharacter* c, int desired_position_x)
    : AbstractSceneHandler()
    , character(c)
    , desired_position_x(desired_position_x)
{}

void WalkTo::run(SceneScript* script, int elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(this->character);

    auto const& pos_x = this->character->get_position().x;
    if (fabs(pos_x - desired_position_x) < 1) {
        pig->stop();
        this->finished = true;
    } else if (pos_x < desired_position_x) {
        pig->run_right();
    } else if (pos_x > desired_position_x) {
        pig->run_left();
    }
}

FaceTo::FaceTo(IGameCharacter* c, int desired_face)
    : AbstractSceneHandler()
    , character(c)
    , desired_face(desired_face)
{}

void FaceTo::run(SceneScript* script, int elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(this->character);
    pig->turn_to(this->desired_face);
    this->finished = true;
}

Talk::Talk(IGameCharacter* c, std::string const& message)
    : AbstractSceneHandler()
    , character(c)
    , message(message)
    , state(TalkState::NotStarted)
{}

void Talk::run(SceneScript* script, int elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(this->character);
    if (this->state == TalkState::NotStarted) {
        pig->talk(this->message);
        this->state = TalkState::Talking;
    } else if (this->state == TalkState::Talking) {
        auto keystates = SDL_GetKeyboardState(NULL);
        if (game_controller.get_state(ControllerAction::ActionKey) == ControllerState::JustPressed) {
            this->state = TalkState::Finished;
            pig->is_talking = false;
        }
    } else if (this->state == TalkState::Finished) {
        this->finished = true;
    }
}

WaitScriptEvent::WaitScriptEvent(IGameCharacter* c, int line_number)
    : AbstractSceneHandler()
    , character(c)
    , line_number(line_number)
{}

void WaitScriptEvent::run(SceneScript* script, int elapsed_time)
{
    auto* other_pig = dynamic_cast<Pig*>(this->character);
    if (this->line_number < other_pig->get_dynamic_property(SceneScriptLinePropertyId)) {
        this->finished = true;
    }
}

SceneScript::SceneScript(std::vector<ScriptLine> const& script)
    : full_script(std::move(script))
    , active_script_line(0)
{
}

void SceneScript::run(int elapsed_time)
{
    if (this->active_script_line >= this->full_script.size()) {
        return;
    }

    auto& [i, action] = this->full_script[active_script_line];
    action->run(this, elapsed_time);
    if (action->is_finished()) {
        this->active_script_line += 1;
    }
}

int SceneScript::get_active_script_line() const
{
    return this->active_script_line;
}

