#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace elemental {

constexpr float width = 960.f;
constexpr float height = 440.f;
constexpr float timeStep = 1.f / 120.f;

struct Rect {
    float x, y, w, h;
    float right() const { return x+w; }
    float bottom() const { return y+h; }
    float center() const { return x+w/2; }
};

bool intersects(Rect a, Rect b);

struct Solid { Rect bounds; bool ice = false; };
struct Ramp { float left, right, top, bottom; };
struct Water { Rect area; float current; };
struct Wind { Rect area; float pressureBelow, pressureAbove, crosswind; };
struct Circuit { Rect source, lever, magnet, contact; bool powersHazard; };
struct Room {
    const wchar_t* title;
    const wchar_t* hint;
    float spawnX, spawnY;
    Rect exit;
    std::vector<Solid> solids;
    std::optional<Ramp> ramp;
    std::optional<Water> water;
    std::optional<Wind> wind;
    std::optional<Rect> lever, phase, gate, hazard, floating, floatSensor, conductive, windObject;
    std::optional<Circuit> circuit;
};

const std::array<Room,8>& rooms();

struct Input { int direction = 0; bool jump = false, interact = false; };
struct Body { Rect rect; float vx = 0, vy = 0; bool grounded = false, icy = false; };
enum class Screen { menu, playing, finished };
enum class Cue { none, jump, switchOn, clear, reset };

class Game {
public:
    void start(std::size_t level = 0);
    void restartLevel();
    void menu();
    Cue update(Input input, float dt = timeStep);
    const Room& room() const { return rooms()[index]; }
    std::size_t roomIndex() const { return index; }
    Screen screen = Screen::menu;
    Body player{{55,310,25,38}};
    std::optional<Body> object;
    bool switched = false, gateOpen = false, hazardActive = false;
    bool circuitPowered = false, contactClosed = false;
private:
    void load(std::size_t next);
    Cue advance();
    std::size_t index = 0;
};

}
