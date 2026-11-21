#include "Presentation.hpp"

#include <SFML/Audio.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace {
using namespace elemental;
using Key = sf::Keyboard::Key;

class Sounds {
public:
    Sounds() {
        const std::array<float,4> pitches = {440,575,735,190};
        for (std::size_t i=0; i<pitches.size(); ++i) {
            std::vector<std::int16_t> samples(4400);
            float phase=0;
            for (std::size_t j=0; j<samples.size(); ++j) {
                const float t=static_cast<float>(j)/static_cast<float>(samples.size());
                phase += 6.2831853f*pitches[i]*(1.f+t*(i==3 ? -.58f : .46f))/22000.f;
                const float envelope = std::min(j/135.f,1.f)*(1-t)*(1-t);
                samples[j]=static_cast<std::int16_t>(4100*envelope*(std::sin(phase)+.22f*std::sin(phase*2)));
            }
            if (buffers[i].loadFromSamples(samples.data(),samples.size(),1,22000,{sf::SoundChannel::Mono})) {
                sounds[i].emplace(buffers[i]);
            }
        }
    }
    void play(Cue cue) {
        if (muted || cue==Cue::none) return;
        const auto i=static_cast<std::size_t>(cue)-1;
        if (sounds[i]) sounds[i]->play();
    }
    void toggle() {
        muted = !muted;
        if (muted) for (auto& sound : sounds) if (sound) sound->stop();
    }
    bool muted = false;
private:
    std::array<sf::SoundBuffer,4> buffers;
    std::array<std::optional<sf::Sound>,4> sounds;
};

void fit(sf::RenderWindow& window, sf::Vector2u size) {
    if (size.x==0 || size.y==0) return;
    sf::View view(sf::FloatRect({0,0},{width,viewHeight}));
    const float canvasRatio=width/viewHeight;
    const float windowRatio=static_cast<float>(size.x)/static_cast<float>(size.y);
    if (windowRatio>canvasRatio) {
        const float scaled=canvasRatio/windowRatio;
        view.setViewport(sf::FloatRect({(1-scaled)*.5f,0},{scaled,1}));
    } else {
        const float scaled=windowRatio/canvasRatio;
        view.setViewport(sf::FloatRect({0,(1-scaled)*.5f},{1,scaled}));
    }
    window.setView(view);
}

int movement() {
    const bool right=sf::Keyboard::isKeyPressed(Key::Right) || sf::Keyboard::isKeyPressed(Key::D);
    const bool left=sf::Keyboard::isKeyPressed(Key::Left) || sf::Keyboard::isKeyPressed(Key::A);
    return static_cast<int>(right)-static_cast<int>(left);
}
}

int main(int argc, char* argv[]) {
    using namespace elemental;
    Presentation graphics;
    const auto asset = std::filesystem::path("assets")/"DejaVuSans.ttf";
    const auto executable = argc>0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
    if (!graphics.loadFont(executable/asset) &&
        !graphics.loadFont(std::filesystem::path(ELEMENTAL_ASSET_PATH)/asset.filename())) {
        std::cerr << "Could not open assets/DejaVuSans.ttf\n";
        return 1;
    }
    sf::RenderWindow window(sf::VideoMode({1120,693}),"Elemental Duo");
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    fit(window,window.getSize());

    Sounds sounds;
    Game game;
    game.menu();
    std::size_t selected=0;
    bool jump=false, interact=false;
    sf::Clock delta, elapsed;
    float accumulator=0;
    while (window.isOpen()) {
        while (const auto event=window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* resize=event->getIf<sf::Event::Resized>()) fit(window,resize->size);
            if (event->is<sf::Event::FocusLost>()) jump=interact=false;
            if (const auto* click=event->getIf<sf::Event::MouseButtonPressed>();
                click && click->button==sf::Mouse::Button::Left && game.screen==Screen::menu) {
                const auto pos=window.mapPixelToCoords(click->position);
                for (std::size_t i=0; i<4; ++i) {
                    if (!intersects({pos.x,pos.y,1,1},Presentation::menuTile(i))) continue;
                    selected=i;
                    game.start(selected);
                    sounds.play(Cue::switchOn);
                    break;
                }
            }
            const auto* key=event->getIf<sf::Event::KeyPressed>();
            if (!key) continue;
            if (key->code==Key::M) sounds.toggle();
            if (key->code==Key::Escape) {
                if (game.screen==Screen::menu) window.close();
                else { game.menu(); jump=interact=false; }
                continue;
            }
            if (game.screen==Screen::menu) {
                if (key->code==Key::Left || key->code==Key::A) selected=(selected+3)%4;
                if (key->code==Key::Right || key->code==Key::D) selected=(selected+1)%4;
                if (key->code==Key::Enter) { game.start(selected); sounds.play(Cue::switchOn); }
            } else if (game.screen==Screen::finished) {
                if (key->code==Key::Enter) { game.start(); sounds.play(Cue::switchOn); }
            } else {
                if (key->code==Key::R) { game.restartLevel(); jump=interact=false; sounds.play(Cue::reset); }
                if (key->code==Key::Space || key->code==Key::Up || key->code==Key::W) jump=true;
                if (key->code==Key::E) interact=true;
            }
        }
        if (!window.isOpen()) break;
        accumulator += std::min(delta.restart().asSeconds(),.05f);
        while (accumulator>=timeStep) {
            if (window.hasFocus()) sounds.play(game.update({movement(),jump,interact}));
            jump=interact=false;
            accumulator-=timeStep;
        }
        window.clear(sf::Color(6,12,21));
        graphics.draw(window,game,elapsed.getElapsedTime().asSeconds(),selected,sounds.muted);
        window.display();
    }
}
