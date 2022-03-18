#include <SceneScript.hpp>
#include <characters/Pig.hpp>

AbstractSceneHandler::AbstractSceneHandler()
    : finished(false)
{
}

bool AbstractSceneHandler::is_finished()
{
    return this->finished;
}

WaitTime::WaitTime(double desired_time)
    : AbstractSceneHandler()
    , desired_time(desired_time)
    , current_time(0.0)
{
}

void WaitTime::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    this->current_time += elapsed_time;
    if (this->current_time >= this->desired_time) {
        this->finished = true;
    }
}

WalkTo::WalkTo(int desired_position_x)
    : AbstractSceneHandler()
    , desired_position_x(desired_position_x)
{
}

void WalkTo::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(c);

    auto const& pos_x = c->get_position().x;
    if (fabs(pos_x - desired_position_x) < 1) {
        pig->stop();
        this->finished = true;
    } else if (pos_x < desired_position_x) {
        pig->run_right();
    } else if (pos_x > desired_position_x) {
        pig->run_left();
    }
}

FaceTo::FaceTo(int desired_face)
    : AbstractSceneHandler()
    , desired_face(desired_face)
{
}

void FaceTo::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(c);
    pig->turn_to(this->desired_face);
    this->finished = true;
}

Talk::Talk(std::string const& message, RGBColor const& talk_color)
    : AbstractSceneHandler()
    , message(message)
    , talk_color(talk_color)
    , state(TalkState::NotStarted)
{
}

void Talk::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(c);
    if (this->state == TalkState::NotStarted) {
        pig->talk(this->message, this->talk_color);
        this->state = TalkState::Talking;
    } else if (this->state == TalkState::Talking) {
        auto keystates = SDL_GetKeyboardState(NULL);
        if (game_controller.get_state(ControllerAction::AttackKey) == ControllerState::JustPressed) {
            this->state = TalkState::Finished;
            pig->is_talking = false;
        }
    } else if (this->state == TalkState::Finished) {
        this->finished = true;
    }
}

WaitScriptEvent::WaitScriptEvent(IGameCharacter* c, int line_number)
    : AbstractSceneHandler()
    , other_character(c)
    , line_number(line_number)
{
}

void WaitScriptEvent::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* other_pig = dynamic_cast<Pig*>(this->other_character);
    if (this->line_number < other_pig->get_dynamic_property(SceneScriptLinePropertyId)) {
        this->finished = true;
    }
}

SetAngry::SetAngry(bool is_angry)
    : AbstractSceneHandler()
    , is_angry(is_angry)
{
}

void SetAngry::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(c);
    pig->set_angry(this->is_angry);
    this->finished = true;
}

SetFear::SetFear(bool is_fear)
    : AbstractSceneHandler()
    , is_fear(is_fear)
{
}

void SetFear::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    auto* pig = dynamic_cast<Pig*>(c);
    pig->set_fear(this->is_fear);
    this->finished = true;
}

RunLambdaEvent::RunLambdaEvent(std::function<void()> const& lambda_f)
    : lambda_f(lambda_f)
{
}

void RunLambdaEvent::run(IGameCharacter* c, SceneScript* script, double elapsed_time)
{
    this->lambda_f();
    this->finished = true;
}

SceneScript::SceneScript(std::vector<ScriptLine> const& script)
    : full_script(std::move(script))
    , active_script_line(0)
{
}

void SceneScript::run(IGameCharacter* c, double elapsed_time)
{
    if (this->active_script_line >= this->full_script.size()) {
        return;
    }

    auto& [i, action] = this->full_script.at(active_script_line);
    action->run(c, this, elapsed_time);
    if (action->is_finished()) {
        this->active_script_line += 1;
    }
}

int SceneScript::get_active_script_line() const
{
    try {
        auto const& [i, _] = this->full_script.at(this->active_script_line);
        return i;
    } catch (std::out_of_range const&) {
        auto const& [i, _] = this->full_script.at(this->active_script_line - 1);
        return i + 1;
    }
}
