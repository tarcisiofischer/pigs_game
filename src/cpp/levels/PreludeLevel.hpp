#ifndef __LVL_PRELUDE_HPP
#define __LVL_PRELUDE_HPP

#include <SDL.h>

#include <GameMap.hpp>
#include <levels/IGameLevel.hpp>
#include <map>
#include <string>
#include <vector>

class IGameCharacter;
class TransitionAnimation;
class GameHandler;

static std::map<std::string, std::string> tr = {
    { "Hah! It worked.", "Ha! Deu certo." },
    { "Boss'll like to know we managed to stole Otto's treasure",
        "O chefe vai gostar de saber que conseguimos roubar o cofre do Otto!" },
    { "Y-yeah, but it'll t-t-take some t-time to take all this g-gold f-f-from "
      "here...",
        "M-mas vai levar um t-tempo pra gente tirar t-todo esse ouro "
        "d-da-daqui..." },
    { "Well... If you talk less and start working, it'll be faster",
        "Bom, se voce falar menos e comecar a trabalhar, vai ser mais rapido" },
    { "What if Otto wake up before we finish, boss?", "Mas e se o Otto acordar antes de a gente terminar, chefe?" },
    { "Yesterday was his birthday, man.", "Ontem foi o aniversario do cara." },
    { "He must be sleepin' after the party.", "Ele deve estar acabado na cama..." },
    { "We'll have plenty of time to steal everything.", "Vamos ter tempo de sobra pra carregar tudo." },
    { "Y-yeah, b-but my back hu-hurts, you know?", "Hmm... M-mas minhas costas estao m-me-meio doloridas, s-sabe?" },
    { "Now that you mentioned... My feet hurts a bit", "Po, eu to com um calo nas patas" },
    { "Did y-you see I've g-g-got this weird stain in my nose?",
        "T-tu v-viu que eu p-p-peguei uma p-pereba aqui no meu f-fu-fucinho?" },
    { "Wow! creepy, man. You should see a doctor.", "Po, que tenso, cara." },
    { "My mother-in-law had a similar thing on she's nose and...",
        "Minha sogra pegou uma pereba assim esses dias, melhor vc ir no medico" },
    { "Can you two stop the smalltalk...", "Voces dois podem parar de papinho..." },
    { "AND START WORKING??", "E COMECAR A TRABALHAR??" }
};

void prepare_script(std::vector<std::unique_ptr<IGameCharacter>>& game_characters, TransitionAnimation& transition_animation);

class PreludeLevel : public IGameLevel {
public:
    explicit PreludeLevel(GameHandler& game_handler);

    GameMap& get_map() override;
    std::vector<std::unique_ptr<IGameCharacter>>& get_characters() override;
    std::function<void()> get_collision_callback(int callback_collision_id, IGameCharacter* character) override;

private:
    GameMap map;
    std::vector<std::unique_ptr<IGameCharacter>> characters;
};

#endif
