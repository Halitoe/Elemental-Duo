#include "Game.hpp"

#include <cstdlib>
#include <iostream>

using namespace elemental;

void require(bool condition, const char* message, const Game& game) {
    if (condition) return;
    std::cerr << message << " | room " << game.roomIndex()
              << " x=" << game.player.rect.x << " y=" << game.player.rect.y
              << " vx=" << game.player.vx << " vy=" << game.player.vy << '\n';
    std::exit(1);
}

template<class Stop>
void play(Game& game, int ticks, int direction, Stop stop) {
    for (int i=0; i<ticks && !stop(); ++i) game.update({direction});
}

int main() {
    Game game;
    game.start();
    auto untilRoom = [&](std::size_t next, int direction = 1) {
        play(game,1200,direction,[&] { return game.roomIndex() == next; });
        require(game.roomIndex()==next,"Expected next room",game);
    };
    auto rightUntil = [&](float x) {
        play(game,550,1,[&] { return game.player.rect.x >= x; });
        require(game.player.rect.x>=x,"Expected position",game);
    };
    auto wait = [&](int ticks) { play(game,ticks,0,[] { return false; }); };
    auto fly = [&](float x, float y) {
        rightUntil(x);
        play(game,210,0,[&] { return game.player.rect.y < y; });
        require(game.player.rect.y<y,"Expected lift",game);
    };

    untilRoom(1);
    rightUntil(115);
    game.update({0,false,true});
    wait(200);
    require(game.gateOpen,"Float reached sensor",game);
    untilRoom(2);

    rightUntil(499);
    require(game.player.icy && game.player.vx>300,"Ice acceleration",game);
    game.update({1,true});
    untilRoom(3);
    rightUntil(133);
    game.update({0,false,true});
    rightUntil(521);
    require(game.player.icy,"Frozen platform collision",game);
    game.update({1,true});
    untilRoom(4);

    fly(264,171);
    untilRoom(5);
    rightUntil(138);
    game.update({0,false,true});
    fly(255,136);
    require(game.object && game.object->rect.y<325,"Wind lifts light object",game);
    untilRoom(6);

    rightUntil(170);
    game.update({0,false,true});
    wait(250);
    require(game.contactClosed && game.gateOpen,"Electromagnet closes gate circuit",game);
    rightUntil(590);
    game.update({1,true});
    untilRoom(7);
    require(game.hazardActive,"Hazard starts live",game);
    rightUntil(164);
    game.update({0,false,true});
    wait(190);
    require(game.contactClosed && !game.hazardActive,"Conductor disables live floor",game);
    rightUntil(443);
    game.update({1,true});
    play(game,800,1,[&] { return game.screen==Screen::finished; });
    require(game.screen==Screen::finished,"Ending reached",game);

    game.start(1);
    game.player.rect.y = height+45;
    require(game.update({})==Cue::reset && game.player.rect.x==53,"Falling resets room",game);
    game.start(3);
    game.restartLevel();
    require(game.roomIndex()==6 && !game.switched,"Restart clears level state",game);
    game.menu();
    require(game.screen==Screen::menu,"Return to menu",game);
    std::cout << "All eight puzzles passed\n";
}
