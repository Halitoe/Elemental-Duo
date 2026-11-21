#include "Game.hpp"
#include <utility>

namespace elemental {

const std::array<Room,8>& rooms() {
    static const auto data = [] {
        std::array<Room,8> r{};
        auto base = [&](int i, const wchar_t* title, const wchar_t* hint, float startY, float exitY,
                        std::vector<Solid> solids) {
            r[i].title = title;
            r[i].hint = hint;
            r[i].spawnX = 53;
            r[i].spawnY = startY;
            r[i].exit = {883,exitY,44,65};
            r[i].solids = std::move(solids);
        };
        base(0,L"Prąd morski",L"Wejdź do wody i płyń w prawo. Spacja pozwala szybciej się wynurzyć.",
             310,305,{{{0,360,216,80}},{{650,370,310,70}},{{216,429,434,11}}});
        r[0].water = Water{{216,271,434,158},44};

        base(1,L"Unosząca się boja",L"Naciśnij E przy zaworze. Woda uniesie boję do czujnika i otworzy bramę.",
             312,303,{{{0,365,200,75}},{{680,368,280,72}},{{200,427,480,13}}});
        r[1].water = Water{{200,345,480,82},-30};
        r[1].lever = Rect{132,323,30,42};
        r[1].floating = Rect{522,388,45,34};
        r[1].floatSensor = Rect{513,255,64,34};
        r[1].gate = Rect{759,238,22,130};

        base(2,L"Rozbieg na lodzie",L"Rozpędź się na oblodzonej pochylni i skocz tuż przed końcem platformy.",
             290,303,{{{0,343,200,97}},{{350,368,180,72},true},{{728,368,232,72}}});
        r[2].ramp = Ramp{200,350,343,368};

        base(3,L"Zamarznięta przeprawa",L"Użyj E przy agregacie, by zamrozić kanał. Nabierz prędkości przed skokiem.",
             306,303,{{{0,368,228,72}},{{740,368,220,72}}});
        r[3].lever = Rect{149,326,29,42};
        r[3].phase = Rect{228,368,323,20};
        r[3].hazard = Rect{228,375,512,65};

        base(4,L"Komora wznoszenia",L"Wejdź do komina. Poczekaj, aż strumień uniesie Cię nad półkę, i rusz w prawo.",
             315,195,{{{0,375,342,65}},{{498,260,462,28}}});
        r[4].wind = Wind{{247,80,265,295},1880,150,10};

        base(5,L"Różnica ciśnień",L"Wciśnij E przy pulpicie, by odwrócić obieg. Po wzniesieniu rusz na górną półkę.",
             315,153,{{{0,375,316,65}},{{480,218,480,30}}});
        r[5].wind = Wind{{234,52,274,323},2020,120,32};
        r[5].windObject = Rect{292,351,24,24};
        r[5].lever = Rect{154,333,27,42};

        base(6,L"Zasilanie bramy",L"Włącz elektromagnes klawiszem E. Skrzynia domknie obwód otwierający bramę.",
             319,310,{{{0,375,960,65}}});
        r[6].conductive = Rect{343,337,38,38};
        r[6].circuit = Circuit{{102,338,32,37},{192,333,26,42},{661,331,31,44},{610,369,44,6},false};
        r[6].gate = Rect{746,242,22,133};

        base(7,L"Odcięcie napięcia",L"Włącz obwód i poczekaj, aż metal połączy styki. Przeskocz skrzynię i przejdź przez wyłączoną strefę.",
             319,310,{{{0,375,960,65}}});
        r[7].conductive = Rect{284,337,38,38};
        r[7].circuit = Circuit{{91,338,32,37},{180,333,26,42},{520,331,31,44},{472,369,42,6},true};
        r[7].hazard = Rect{583,342,211,33};
        return r;
    }();
    return data;
}

}
