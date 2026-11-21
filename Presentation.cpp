#include "Presentation.hpp"

#include <cmath>
#include <initializer_list>

namespace elemental {
namespace {
using Color = sf::Color;
constexpr Color paper{234,245,255};
constexpr Color mutedText{167,189,209};
constexpr Color nav{11,18,31};
constexpr std::array<Color,4> accents = {{{69,229,241},{158,213,255},{255,185,98},{255,111,199}}};
constexpr std::array<Color,4> skies = {{{16,70,102},{45,89,132},{99,53,79},{71,33,82}}};

Color alpha(Color color, std::uint8_t opacity) {
    color.a = opacity;
    return color;
}

void fill(sf::RenderTarget& target, Rect r, Color color) {
    sf::RectangleShape shape({r.w,r.h});
    shape.setPosition({r.x,r.y});
    shape.setFillColor(color);
    target.draw(shape);
}

void gradient(sf::RenderTarget& target, Rect r, Color top, Color bottom) {
    const sf::Vertex vertices[] = {
        {{r.x,r.y},top}, {{r.right(),r.y},top},
        {{r.x,r.bottom()},bottom}, {{r.right(),r.bottom()},bottom}
    };
    target.draw(vertices,4,sf::PrimitiveType::TriangleStrip);
}

void circle(sf::RenderTarget& target, float x, float y, float radius, Color color) {
    sf::CircleShape shape(radius,32);
    shape.setPosition({x-radius,y-radius});
    shape.setFillColor(color);
    target.draw(shape);
}

void polygon(sf::RenderTarget& target, std::initializer_list<sf::Vector2f> points, Color color) {
    sf::ConvexShape poly(points.size());
    std::size_t i = 0;
    for (const auto point : points) poly.setPoint(i++,point);
    poly.setFillColor(color);
    target.draw(poly);
}

void line(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, float thick, Color color) {
    const auto delta = to-from;
    const float length = std::hypot(delta.x,delta.y);
    if (length < .01f) return;
    const sf::Vector2f normal{-delta.y*thick/(2*length),delta.x*thick/(2*length)};
    polygon(target,{from+normal,to+normal,to-normal,from-normal},color);
}

void caption(sf::RenderTarget& target, const sf::Font& font, const wchar_t* content,
             float x, float y, unsigned int size, Color color, bool bold = false) {
    sf::Text text(font,content,size);
    if (bold) text.setStyle(sf::Text::Bold);
    text.setFillColor(color);
    text.setPosition({x,y});
    target.draw(text);
}

Rect scene(Rect r) { r.y += sceneTop; return r; }

void scenery(sf::RenderTarget& window, std::size_t environment, float t) {
    const Color accent = accents[environment];
    gradient(window,{0,sceneTop,width,height},skies[environment],{11,18,35});
    const float top = sceneTop;
    if (environment == 0) {
        for (int i=0; i<10; ++i) {
            const float x = 50+i*112+std::sin(t*.35f+i)*22;
            polygon(window,{{x,top},{x+42,top},{x+169,top+height},{x+104,top+height}},alpha(accent,13));
        }
        polygon(window,{{0,top+210},{109,top+171},{192,top+236},{310,top+188},{463,top+272},
                        {617,top+229},{771,top+274},{960,top+184},{960,top+height},{0,top+height}},
                {11,43,61});
        for (int i=0; i<21; ++i) {
            const float x = 10+i*48;
            const float y = top+321+i%4*13;
            polygon(window,{{x-7,top+height},{x+5,y},{x+18,y+44},{x+26,y-19},{x+37,top+height}},{20,73,83});
        }
    } else if (environment == 1) {
        for (int i=0; i<7; ++i) {
            const float x=i*174-145, y=top+53+i*47%123;
            polygon(window,{{x,top+390},{x+169,y},{x+333,top+390}}, {40,76,113});
            polygon(window,{{x+169,y},{x+129,y+102},{x+166,y+73},{x+212,y+92}}, {167,210,230});
            polygon(window,{{x+169,y},{x+333,top+390},{x+198,top+390}}, {35,58,92});
        }
        for (int i=0; i<50; ++i) {
            const float x = std::fmod(i*179+t*14,width);
            const float y = top+std::fmod(i*61+t*(13+i%8),height);
            circle(window,x,y,1.f+i%3*.45f,alpha(paper,100+i%4*23));
        }
    } else if (environment == 2) {
        for (int i=8; i>=1; --i) circle(window,727,top+104,55+i*8,alpha(accent,6));
        circle(window,727,top+104,54,{255,194,126});
        polygon(window,{{0,top+255},{124,top+215},{270,top+261},{401,top+195},{595,top+262},
                        {760,top+270},{960,top+211},{960,top+height},{0,top+height}},{60,44,70});
        for (int i=0; i<7; ++i) {
            const float x=38+i*149, y=top+158+i*47%95;
            gradient(window,{x,y,42,top+height-y},{122,86,111},{33,33,54});
            fill(window,{x-9,y-10,60,10},{145,102,122});
            fill(window,{x+8,y+8,3,top+height-y-8},{146,101,118});
        }
        for (int i=0; i<14; ++i) {
            const float x=std::fmod(i*147+t*32,1110.f)-100;
            fill(window,{x,top+50+i*47%242,65.f+i%4*17,2},alpha(accent,34));
        }
    } else {
        for (int x=0; x<960; x+=30) fill(window,{float(x),top,1,height},alpha(accent,22));
        for (int y=0; y<440; y+=30) fill(window,{0,top+y,width,1},alpha(accent,22));
        for (int i=0; i<6; ++i) {
            const float x=29+i*167, y=top+97+i%2*42;
            fill(window,{x,y,105,164},{43,31,61});
            fill(window,{x+8,y+10,89,52},{23,24,42});
            for (int j=0; j<7; ++j) {
                const float bar=7+j*4+std::sin(t*2+i+j)*4;
                fill(window,{x+14+j*11,y+50-bar,6,bar},alpha(accent,102));
            }
            fill(window,{x+11,y+127,83,4},alpha(accent,50));
        }
    }
}

void terrain(sf::RenderTarget& window, Rect r, bool ice, Color accent) {
    r = scene(r);
    gradient(window,r,ice ? Color{57,116,150} : Color{41,58,78},{13,23,38});
    fill(window,{r.x,r.y,r.w,3},ice ? paper : accent);
    if (r.h < 16) return;
    for (float x=r.x+12; x+9<r.right(); x+=31) {
        fill(window,{x,r.y+9,7,2},alpha(accent,73));
        if (ice && r.h > 30) line(window,{x,r.y+12},{x+7,r.y+26},1,alpha(paper,71));
    }
}

void waterArea(sf::RenderTarget& window, Rect area, float flow, Color accent, float t) {
    const Rect w = scene(area);
    gradient(window,w,alpha(accent,64),{18,76,120,152});
    for (float x=w.x; x+8<=w.right(); x+=8) {
        const float y = w.y+std::sin(x*.079f+t*4)*2;
        const float next = w.y+std::sin((x+8)*.079f+t*4)*2;
        line(window,{x,y},{x+8,next},2,alpha(accent,205));
    }
    for (int i=0; i<24; ++i) {
        const float drift = std::fmod(i*123+std::abs(flow)*t,w.w-16);
        const float x = flow>0 ? w.x+8+drift : w.right()-8-drift;
        const float y = w.bottom()-5-std::fmod(i*41+t*(16+i%7),w.h-10);
        circle(window,x,y,1.5f+i%4*.5f,alpha(accent,60+i%4*38));
    }
}

void switchBox(sf::RenderTarget& window, const sf::Font& font, Rect world, bool on, Color accent) {
    const Rect r = scene(world);
    gradient(window,r,{58,75,95},{20,30,49});
    fill(window,{r.x+3,r.y+4,r.w-6,13},{15,25,40});
    circle(window,r.center(),r.y+10,4,on ? accent : Color{229,136,82});
    caption(window,font,L"E",r.center()-6,r.y-23,14,paper,true);
}

void circuitLines(sf::RenderTarget& window, const Circuit& circuit, bool live, bool closed, Color accent) {
    const auto source = scene(circuit.source), lever = scene(circuit.lever);
    const auto magnet = scene(circuit.magnet), contact = scene(circuit.contact);
    const float rail = source.bottom()+16;
    const Color liveColor = live ? accent : alpha(accent,63);
    for (const auto r : {source,lever,magnet}) line(window,{r.center(),r.bottom()},{r.center(),rail},2,liveColor);
    line(window,{source.center(),rail},{magnet.center(),rail},2,liveColor);
    line(window,{contact.center(),contact.bottom()},{contact.center(),rail+8},2,closed ? paper : alpha(accent,63));
    gradient(window,source,{71,56,92},{30,27,51});
    fill(window,{source.x+6,source.y+7,source.w-12,4},accent);
    fill(window,{source.x+6,source.y+16,source.w-12,3},alpha(accent,126));
    gradient(window,magnet,liveColor,{35,28,57});
    for (int i=0; i<3; ++i) fill(window,{magnet.x+4,magnet.y+7+i*9,magnet.w-8,2},{52,28,70});
    fill(window,contact,closed ? paper : alpha(accent,104));
}

void portal(sf::RenderTarget& window, Rect e, bool open, Color color, float t) {
    e=scene(e);
    if (!open) color={109,130,151};
    for (int i=4; i>=1; --i) fill(window,{e.x-i*3,e.y-i*3,e.w+i*6,e.h+i*3},alpha(color,13));
    gradient(window,e,color,{32,47,70});
    gradient(window,{e.x+4,e.y+5,e.w-8,e.h-5},{17,22,49},alpha(color,150));
    for (int i=0; i<7; ++i) {
        const float y=e.y+8+std::fmod(t*27+i*13,e.h-10);
        fill(window,{e.x+8+i*7%22,y,5.f+i%3,2},alpha(color,150));
    }
}

void playerSprite(sf::RenderTarget& window, const Body& player, Color accent, float t) {
    const auto p = scene(player.rect);
    const float stride = player.grounded && std::abs(player.vx)>20 ? std::sin(t*28)*3 : 0;
    fill(window,{p.x+3,p.y+28,7,10+stride},{134,156,192});
    fill(window,{p.x+15,p.y+28,7,10-stride},{134,156,192});
    gradient(window,{p.x+1,p.y+12,p.w-2,19},paper,{164,183,208});
    circle(window,p.center(),p.y+11,12,paper);
    gradient(window,{p.x+3,p.y+6,p.w-6,12},{28,65,97},{14,24,44});
    fill(window,{p.x+(player.vx<-4 ? 6.f : 15.f),p.y+9,4,5},accent);
    fill(window,{p.x+4,p.y+23,17,3},accent);
}
}

bool Presentation::loadFont(const std::filesystem::path& path) { return font.openFromFile(path); }

Rect Presentation::menuTile(std::size_t index) { return {58.f+214.f*index,372,202,74}; }

void Presentation::draw(sf::RenderWindow& window, const Game& game, float t,
                        std::size_t selected, bool muted) const {
    const std::size_t environment = game.screen==Screen::menu ? selected : game.roomIndex()/2;
    const Color accent = accents[environment];
    scenery(window,environment,t);
    if (game.screen==Screen::playing) {
        const auto& room = game.room();
        if (room.water) {
            auto w = room.water->area;
            if (room.floating && game.switched) { w.h += w.y-254; w.y=254; }
            waterArea(window,w,room.water->current,accent,t);
        }
        if (room.wind) {
            const Rect w = scene(room.wind->area);
            const bool upward = !room.lever || game.switched;
            gradient(window,w,alpha(accent,15),alpha(accent,63));
            for (int i=0; i<26; ++i) {
                const float x=w.x+12+i*73%int(w.w-24);
                const float offset=std::fmod(i*43+t*(62+i%7*14),w.h-28);
                const float y=upward ? w.bottom()-17-offset : w.y+14+offset;
                const float sign = upward ? -1.f : 1.f;
                line(window,{x,y-sign*9},{x,y+sign*8},1.5f,alpha(accent,146));
                line(window,{x-4,y+sign*3},{x,y+sign*8},1.5f,alpha(accent,146));
                line(window,{x+4,y+sign*3},{x,y+sign*8},1.5f,alpha(accent,146));
            }
        }
        for (const auto& solid : room.solids) terrain(window,solid.bounds,solid.ice,accent);
        if (room.ramp) {
            const auto r=*room.ramp;
            polygon(window,{{r.left,sceneTop+r.top},{r.right,sceneTop+r.bottom},
                            {r.right,sceneTop+height},{r.left,sceneTop+height}},{40,73,104});
            line(window,{r.left,sceneTop+r.top},{r.right,sceneTop+r.bottom},3,paper);
        }
        if (room.phase) {
            if (game.switched) terrain(window,*room.phase,true,accent);
            else {
                const auto channel = Rect{room.phase->x,room.phase->y,
                                          room.hazard->w,height-room.phase->y};
                waterArea(window,channel,19,accent,t);
            }
            if (game.switched) {
                const auto open = Rect{room.phase->right(),room.phase->y,
                                       room.hazard->right()-room.phase->right(),height-room.phase->y};
                waterArea(window,open,19,accent,t);
            }
        }
        if (room.circuit) {
            circuitLines(window,*room.circuit,game.circuitPowered,game.contactClosed,accent);
            switchBox(window,font,room.circuit->lever,game.switched,accent);
        }
        if (room.lever) switchBox(window,font,*room.lever,game.switched,accent);
        if (room.floatSensor) {
            const auto sensor = scene(*room.floatSensor);
            fill(window,{sensor.x,sensor.y,sensor.w,3},alpha(accent,175));
            circle(window,sensor.center(),sensor.y,5,game.gateOpen ? paper : accent);
        }
        if (game.object) {
            const auto o=scene(game.object->rect);
            const Color material = room.windObject || room.floating ? Color{255,200,117} : Color{198,207,224};
            gradient(window,o,material,{51,57,77});
            fill(window,{o.x+5,o.y+5,o.w-10,3},alpha(paper,130));
            fill(window,{o.x+5,o.bottom()-8,o.w-10,3},room.floating ? accent : Color{96,115,146});
        }
        if (room.hazard && room.circuit) {
            const Rect r = scene(*room.hazard);
            fill(window,{r.x,r.bottom()-4,r.w,4}, game.hazardActive ? accent : Color{73,143,125});
            if (game.hazardActive) {
                gradient(window,r,alpha(accent,9),alpha(accent,102));
                for (float x=r.x+8; x<r.right()-7; x+=17) {
                    const float y=r.bottom()-5-std::abs(std::sin(t*11+x))*r.h*.7f;
                    line(window,{x-6,r.bottom()-4},{x,y},1.5f,accent);
                    line(window,{x,y},{x+5,y+4},1.5f,accent);
                }
            }
        }
        if (room.gate && !game.gateOpen) {
            const auto g=scene(*room.gate);
            gradient(window,g,alpha(accent,204),{37,38,63});
            for (float y=g.y+6; y<g.bottom()-4; y+=11) fill(window,{g.x+3,y,g.w-6,2},alpha(paper,125));
        }
        portal(window,room.exit,!room.gate || game.gateOpen,accent,t);
        playerSprite(window,game.player,accent,t);

        fill(window,{0,0,width,80},nav);
        fill(window,{0,78,width,2},alpha(accent,124));
        caption(window,font,L"ELEMENTAL  /  DUO",28,10,15,accent,true);
        caption(window,font,room.title,28,33,28,paper,true);
        caption(window,font,levelNames[environment],667,17,18,paper,true);
        for (int i=0; i<8; ++i) {
            const auto roomIndex=static_cast<std::size_t>(i);
            const float x=667+i*30.f+i/2*9.f;
            const Color progress=roomIndex<game.roomIndex() ? mutedText : roomIndex==game.roomIndex() ? accent : Color{55,69,91};
            fill(window,{x,53,22,5},progress);
        }
        fill(window,{0,520,width,74},nav);
        caption(window,font,room.hint,26,527,16,paper);
        caption(window,font,L"A / D lub ← / →  ruch     SPACJA  skok     E  interakcja     R  restart     ESC  menu",
                26,556,13,mutedText);
        caption(window,font,muted ? L"M  dźwięk wył." : L"M  dźwięk wł.",818,556,13,mutedText);
        return;
    }

    gradient(window,{0,0,width,viewHeight},{9,15,28,165},{8,15,27,249});
    if (game.screen==Screen::finished) {
        caption(window,font,L"ELEMENTAL  /  DUO",58,90,21,accent,true);
        caption(window,font,L"Wyprawa",56,150,62,paper,true);
        caption(window,font,L"ukończona.",56,223,62,accent,true);
        caption(window,font,L"Portal w laboratorium zamknął wyprawę.",59,335,23,mutedText);
        caption(window,font,L"ENTER  zagraj od nowa     ESC  menu",59,430,18,paper,true);
        return;
    }
    caption(window,font,L"Elemental",54,76,86,paper,true);
    caption(window,font,L"Duo",57,163,86,accent,true);
    caption(window,font,L"Gra logiczno-platformowa",62,279,22,mutedText);
    for (std::size_t i=0;i<levelNames.size();++i) {
        const Rect r=menuTile(i);
        fill(window,r,i==selected ? Color{38,57,76} : Color{24,36,55});
        fill(window,{r.x,r.y,r.w,3},i==selected ? accents[i] : alpha(accents[i],95));
        const wchar_t number[]={wchar_t(L'1'+i),0};
        caption(window,font,number,r.x+12,r.y+11,13,accents[i],true);
        caption(window,font,levelNames[i],r.x+12,r.y+34,15,paper,true);
    }
    caption(window,font,L"ENTER  rozpocznij     ← / →  wybierz poziom",59,476,17,paper,true);
    caption(window,font,L"Przy pierwszej rozgrywce wybierz Podwodną przeprawę.",59,526,15,mutedText);
}

}
