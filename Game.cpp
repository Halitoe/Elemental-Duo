#include "Game.hpp"

#include <algorithm>
#include <cmath>

namespace elemental {
namespace {
constexpr float gravity = 1000.f;

void moveHorizontal(Body& p, float dt, const std::vector<Solid>& solids) {
    p.rect.x = std::clamp(p.rect.x+p.vx*dt,0.f,width-p.rect.w);
    for (const auto& solid : solids) {
        if (!intersects(p.rect,solid.bounds)) continue;
        if (p.vx > 0) p.rect.x = solid.bounds.x-p.rect.w;
        if (p.vx < 0) p.rect.x = solid.bounds.right();
        p.vx = 0;
    }
}

void moveVertical(Body& p, float dt, const std::vector<Solid>& solids, std::optional<Ramp> ramp) {
    const float oldFeet = p.rect.bottom();
    const bool wasGrounded = p.grounded;
    p.rect.y += p.vy*dt;
    p.grounded = p.icy = false;
    for (const auto& solid : solids) {
        const auto ground = solid.bounds;
        if (!intersects(p.rect,ground)) continue;
        if (p.vy >= 0 && oldFeet <= ground.y+1) {
            p.rect.y = ground.y-p.rect.h;
            p.grounded = true;
            p.icy = solid.ice;
            p.vy = 0;
        } else if (p.vy < 0 && oldFeet-p.rect.h >= ground.bottom()-1) {
            p.rect.y = ground.bottom();
            p.vy = 0;
        }
    }
    if (ramp && p.vy >= 0 && p.rect.x >= ramp->left && p.rect.x <= ramp->right) {
        const float t = (p.rect.x-ramp->left)/(ramp->right-ramp->left);
        const float surface = ramp->top+t*(ramp->bottom-ramp->top);
        if (p.rect.bottom() >= surface || (wasGrounded && surface-oldFeet < 5)) {
            p.rect.y = surface-p.rect.h;
            p.grounded = p.icy = true;
            p.vy = 0;
        }
    }
}

float submerged(Rect body, Rect water) {
    return std::clamp((body.bottom()-water.y)/body.h,0.f,1.f);
}

bool near(Rect a, Rect b) {
    return std::abs(a.center()-b.center()) < 61 &&
           std::abs(a.y+a.h/2-b.y-b.h/2) < 66;
}
}

bool intersects(Rect a, Rect b) {
    return a.x < b.right() && a.right() > b.x && a.y < b.bottom() && a.bottom() > b.y;
}

void Game::load(std::size_t next) {
    index = next;
    const auto& level = room();
    player = Body{{level.spawnX,level.spawnY,25,38}};
    object.reset();
    if (level.floating) object = Body{*level.floating};
    if (level.conductive) object = Body{*level.conductive};
    if (level.windObject) object = Body{*level.windObject};
    switched = gateOpen = circuitPowered = contactClosed = false;
    hazardActive = level.hazard.has_value();
}

void Game::start(std::size_t level) {
    screen = Screen::playing;
    load(std::min(level,std::size_t{3})*2);
}

void Game::restartLevel() { if (screen == Screen::playing) load(index-index%2); }

void Game::menu() {
    load(0);
    screen = Screen::menu;
}

Cue Game::advance() {
    if (index+1 == rooms().size()) screen = Screen::finished;
    else load(index+1);
    return Cue::clear;
}

Cue Game::update(Input input, float dt) {
    if (screen != Screen::playing) return Cue::none;
    const auto& level = room();
    Cue cue = Cue::none;
    const auto lever = level.circuit ? std::optional<Rect>{level.circuit->lever} : level.lever;
    if (lever && input.interact && near(player.rect,*lever)) {
        switched = !switched;
        cue = Cue::switchOn;
    }

    auto water = level.water;
    if (water && level.floating && switched) {
        water->area.h += water->area.y-254;
        water->area.y = 254;
    }
    const bool swimming = water && intersects(player.rect,water->area);
    const bool inWind = level.wind && intersects(player.rect,level.wind->area);
    auto& p = player;
    const float speed = swimming ? 185.f : p.icy ? 460.f : 245.f;
    const float acceleration = swimming ? 820.f : p.icy ? 700.f : p.grounded ? 2600.f : 1150.f;
    if (input.direction && (p.vx*input.direction < 0 || std::abs(p.vx) < speed)) {
        p.vx = std::clamp(p.vx+input.direction*acceleration*dt,-speed,speed);
    } else if (!input.direction) {
        const float drag = swimming ? 4.f : p.grounded ? (p.icy ? 0.42f : 15.f) : 0.6f;
        p.vx *= std::exp(-drag*dt);
    }
    if (p.icy && level.ramp && p.rect.x >= level.ramp->left && p.rect.x <= level.ramp->right) {
        const auto& ramp = *level.ramp;
        p.vx += gravity*(ramp.bottom-ramp.top)/(ramp.right-ramp.left)*dt;
    }
    if (swimming) {
        p.vx += (water->current-1.48f*p.vx)*dt;
        p.vy += (gravity-1730.f*submerged(p.rect,water->area)-5.f*p.vy)*dt;
        if (input.jump) { p.vy = -235; cue = Cue::jump; }
    } else {
        if (input.jump && p.grounded) { p.vy = -445; cue = Cue::jump; }
        float force = gravity;
        if (inWind) {
            const float direction = level.lever && !switched ? -1.f : 1.f;
            force -= direction*(level.wind->pressureBelow-level.wind->pressureAbove);
            p.vx += direction*level.wind->crosswind*dt;
        }
        p.vy += force*dt;
    }
    p.vy = std::clamp(p.vy,-465.f,650.f);

    auto solids = level.solids;
    if (level.phase && switched) solids.push_back({*level.phase,true});
    if (object && level.floating && water) {
        object->vy += (gravity-2050.f*submerged(object->rect,water->area)-5.f*object->vy)*dt;
        moveVertical(*object,dt,level.solids,std::nullopt);
        gateOpen = intersects(object->rect,*level.floatSensor) || intersects(p.rect,*level.gate);
    }
    if (object && level.windObject && level.wind) {
        const auto& wind = *level.wind;
        const float direction = switched ? 1.f : -1.f;
        const float lift = intersects(object->rect,wind.area) ?
            direction*(wind.pressureBelow-wind.pressureAbove)*1.23f : 0.f;
        object->vy += (gravity-lift-3.1f*object->vy)*dt;
        moveVertical(*object,dt,level.solids,std::nullopt);
        if (object->rect.y < 20) { object->rect.y = 20; object->vy = 0; }
    }
    if (object && level.circuit) {
        const auto& circuit = *level.circuit;
        circuitPowered = switched;
        const float target = circuit.magnet.x-object->rect.w-4;
        const float distance = target-object->rect.x;
        if (circuitPowered && distance > 0) {
            object->vx = std::min(230.f,object->vx+(350.f+22000.f/(45.f+distance))*dt);
            object->rect.x = std::min(target,object->rect.x+object->vx*dt);
        } else object->vx = 0;
        contactClosed = intersects(object->rect,circuit.contact);
        gateOpen = (circuitPowered && contactClosed) ||
                   (level.gate && intersects(p.rect,*level.gate));
        hazardActive = circuit.powersHazard && !(circuitPowered && contactClosed);
    }
    if (object && level.conductive) solids.push_back({object->rect});
    if (object && level.floating && p.rect.bottom() <= object->rect.y+1) solids.push_back({object->rect});
    if (level.gate && !gateOpen) solids.push_back({*level.gate});
    moveHorizontal(p,dt,solids);
    moveVertical(p,dt,solids,level.ramp);
    if (p.rect.y < 8) { p.rect.y = 8; p.vy = std::max(0.f,p.vy); }

    bool danger = hazardActive && level.hazard && intersects(p.rect,*level.hazard);
    if (danger && level.phase && switched && p.rect.bottom() <= level.phase->bottom() &&
        p.rect.x < level.phase->right()) danger = false;
    if (p.rect.y > height+35 || danger) { load(index); return Cue::reset; }
    if (intersects(p.rect,level.exit)) return advance();
    return cue;
}

}
