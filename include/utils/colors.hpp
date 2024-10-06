
#pragma once

#ifndef COLORS_HPP
#define COLORS_HPP

#include "utils/string.hpp"
#include <cstdint> // std::uint8_t

namespace utils {

    struct Color {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
    };
    
    // Allows for accessing colors from the list below by name
    struct ColorComparator {
        bool operator()(std::string_view first, std::string_view second) const;
    };
    
    extern std::unordered_map<std::string_view, Color, std::hash<std::string_view>, ColorComparator> colors;
    
    Color ansi_to_rgb(std::uint8_t code);
    
    // Ref: https://www.uxgem.com/docs/css-color-keywords
    Color black = Color(0, 0, 0);
    Color aliceblue = Color(240, 248, 255);
    Color antiquewhite = Color(250, 235, 215);
    Color aqua = Color(0, 255, 255);
    Color aquamarine = Color(127, 255, 212);
    Color azure = Color(240, 255, 255);
    Color beige = Color(245, 245, 220);
    Color bisque = Color(255, 228, 196);
    Color blanchedalmond = Color(255, 235, 205);
    Color blue = Color(0, 0, 255);
    Color blueviolet = Color(138, 43, 226);
    Color brown = Color(165, 42, 42);
    Color burlywood = Color(222, 184, 135);
    Color cadetblue = Color(95, 158, 160);
    Color chartreuse = Color(127, 255, 0);
    Color chocolate = Color(210, 105, 30);
    Color coral = Color(255, 127, 80);
    Color cornflowerblue = Color(100, 149, 237);
    Color cornsilk = Color(255, 248, 220);
    Color crimson = Color(220, 20, 60);
    Color cyan = Color(0, 255, 255);
    Color darkblue = Color(0, 0, 139);
    Color darkcyan = Color(0, 139, 139);
    Color darkgoldenrod = Color(184, 134, 11);
    Color darkgray = Color(169, 169, 169);
    Color darkgreen = Color(0, 100, 0);
    Color darkkhaki = Color(189, 183, 107);
    Color darkmagenta = Color(139, 0, 139);
    Color darkolivegreen = Color(85, 107, 47);
    Color darkorange = Color(255, 140, 0);
    Color darkorchid = Color(153, 50, 204);
    Color darkred = Color(139, 0, 0);
    Color darksalmon = Color(233, 150, 122);
    Color darkseagreen = Color(143, 188, 143);
    Color darkslateblue = Color(72, 61, 139);
    Color darkslategray = Color(47, 79, 79);
    Color darkturquoise = Color(0, 206, 209);
    Color darkviolet = Color(148, 0, 211);
    Color deeppink = Color(255, 20, 147);
    Color deepskyblue = Color(0, 191, 255);
    Color dimgray = Color(105, 105, 105);
    Color dodgerblue = Color(30, 144, 255);
    Color firebrick = Color(178, 34, 34);
    Color floralwhite = Color(255, 250, 240);
    Color forestgreen = Color(34, 139, 34);
    Color fuchsia = Color(255, 0, 255);
    Color gainsboro = Color(220, 220, 220);
    Color ghostwhite = Color(248, 248, 255);
    Color gold = Color(255, 215, 0);
    Color goldenrod = Color(218, 165, 32);
    Color gray = Color(128, 128, 128);
    Color green = Color(0, 128, 0);
    Color greenyellow = Color(173, 255, 47);
    Color honeydew = Color(240, 255, 240);
    Color hotpink = Color(255, 105, 180);
    Color indianred = Color(205, 92, 92);
    Color indigo = Color(75, 0, 130);
    Color ivory = Color(255, 255, 240);
    Color khaki = Color(240, 230, 140);
    Color lavender = Color(230, 230, 250);
    Color lavenderblush = Color(255, 240, 245);
    Color lawngreen = Color(124, 252, 0);
    Color lemonchiffon = Color(255, 250, 205);
    Color lightblue = Color(173, 216, 230);
    Color lightcoral = Color(240, 128, 128);
    Color lightcyan = Color(224, 255, 255);
    Color lightgoldenrodyellow = Color(250, 250, 210);
    Color lightgray = Color(211, 211, 211);
    Color lightgreen = Color(144, 238, 144);
    Color lightpink = Color(255, 182, 193);
    Color lightsalmon = Color(255, 160, 122);
    Color lightseagreen = Color(32, 178, 170);
    Color lightskyblue = Color(135, 206, 250);
    Color lightslategray = Color(119, 136, 153);
    Color lightsteelblue = Color(176, 196, 222);
    Color lightyellow = Color(255, 255, 224);
    Color lime = Color(0, 255, 0);
    Color limegreen = Color(50, 205, 50);
    Color linen = Color(250, 240, 230);
    Color magenta = Color(255, 0, 255);
    Color maroon = Color(128, 0, 0);
    Color mediumaquamarine = Color(102, 205, 170);
    Color mediumblue = Color(0, 0, 205);
    Color mediumorchid = Color(186, 85, 211);
    Color mediumpurple = Color(147, 112, 219);
    Color mediumseagreen = Color(60, 179, 113);
    Color mediumslateblue = Color(123, 104, 238);
    Color mediumspringgreen = Color(0, 250, 154);
    Color mediumturquoise = Color(72, 209, 204);
    Color mediumvioletred = Color(199, 21, 133);
    Color midnightblue = Color(25, 25, 112);
    Color mintcream = Color(245, 255, 250);
    Color mistyrose = Color(255, 228, 225);
    Color moccasin = Color(255, 228, 181);
    Color navajowhite = Color(255, 222, 173);
    Color navy = Color(0, 0, 128);
    Color oldlace = Color(253, 245, 230);
    Color olive = Color(128, 128, 0);
    Color olivedrab = Color(107, 142, 35);
    Color orange = Color(255, 165, 0);
    Color orangered = Color(255, 69, 0);
    Color orchid = Color(218, 112, 214);
    Color palegoldenrod = Color(238, 232, 170);
    Color palegreen = Color(152, 251, 152);
    Color paleturquoise = Color(175, 238, 238);
    Color palevioletred = Color(219, 112, 147);
    Color papayawhip = Color(255, 239, 213);
    Color peachpuff = Color(255, 218, 185);
    Color peru = Color(205, 133, 63);
    Color pink = Color(255, 192, 203);
    Color plum = Color(221, 160, 221);
    Color powderblue = Color(176, 224, 230);
    Color purple = Color(128, 0, 128);
    Color rebeccapurple = Color(102, 51, 153);
    Color red = Color(255, 0, 0);
    Color rosybrown = Color(188, 143, 143);
    Color royalblue = Color(65, 105, 225);
    Color saddlebrown = Color(139, 69, 19);
    Color salmon = Color(250, 128, 114);
    Color sandybrown = Color(244, 164, 96);
    Color seagreen = Color(46, 139, 87);
    Color seashell = Color(255, 245, 238);
    Color sienna = Color(160, 82, 45);
    Color silver = Color(192, 192, 192);
    Color skyblue = Color(135, 206, 235);
    Color slateblue = Color(106, 90, 205);
    Color slategray = Color(112, 128, 144);
    Color snow = Color(255, 250, 250);
    Color springgreen = Color(0, 255, 127);
    Color steelblue = Color(70, 130, 180);
    Color tan = Color(210, 180, 140);
    Color teal = Color(0, 128, 128);
    Color thistle = Color(216, 191, 216);
    Color tomato = Color(255, 99, 71);
    Color turquoise = Color(64, 224, 208);
    Color violet = Color(238, 130, 238);
    Color wheat = Color(245, 222, 179);
    Color white = Color(255, 255, 255);
    Color whitesmoke = Color(245, 245, 245);
    Color yellow = Color(255, 255, 0);
    Color yellowgreen = Color(154, 205, 50);
    
}

#endif // COLORS_HPP
