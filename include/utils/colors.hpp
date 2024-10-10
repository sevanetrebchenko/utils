
#pragma once

#ifndef COLORS_HPP
#define COLORS_HPP

#include <string_view> // std::string_view
#include <unordered_map> // std::unordered_map
#include <cstdint> // std::uint8_t

namespace utils {

    struct Color {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        
        std::uint8_t operator[](std::size_t index) const;
    };
    
    struct ColorComparator {
        bool operator()(std::string_view first, std::string_view second) const;
    };
    
    // Allows for accessing colors from the list below by name
    extern std::unordered_map<std::string_view, Color, std::hash<std::string_view>, ColorComparator> colors;
    
    Color ansi_to_rgb(std::uint8_t code);
    
    // Ref: https://www.uxgem.com/docs/css-color-keywords
    static Color black = Color(0, 0, 0);
    static Color aliceblue = Color(240, 248, 255);
    static Color antiquewhite = Color(250, 235, 215);
    static Color aqua = Color(0, 255, 255);
    static Color aquamarine = Color(127, 255, 212);
    static Color azure = Color(240, 255, 255);
    static Color beige = Color(245, 245, 220);
    static Color bisque = Color(255, 228, 196);
    static Color blanchedalmond = Color(255, 235, 205);
    static Color blue = Color(0, 0, 255);
    static Color blueviolet = Color(138, 43, 226);
    static Color brown = Color(165, 42, 42);
    static Color burlywood = Color(222, 184, 135);
    static Color cadetblue = Color(95, 158, 160);
    static Color chartreuse = Color(127, 255, 0);
    static Color chocolate = Color(210, 105, 30);
    static Color coral = Color(255, 127, 80);
    static Color cornflowerblue = Color(100, 149, 237);
    static Color cornsilk = Color(255, 248, 220);
    static Color crimson = Color(220, 20, 60);
    static Color cyan = Color(0, 255, 255);
    static Color darkblue = Color(0, 0, 139);
    static Color darkcyan = Color(0, 139, 139);
    static Color darkgoldenrod = Color(184, 134, 11);
    static Color darkgray = Color(169, 169, 169);
    static Color darkgreen = Color(0, 100, 0);
    static Color darkkhaki = Color(189, 183, 107);
    static Color darkmagenta = Color(139, 0, 139);
    static Color darkolivegreen = Color(85, 107, 47);
    static Color darkorange = Color(255, 140, 0);
    static Color darkorchid = Color(153, 50, 204);
    static Color darkred = Color(139, 0, 0);
    static Color darksalmon = Color(233, 150, 122);
    static Color darkseagreen = Color(143, 188, 143);
    static Color darkslateblue = Color(72, 61, 139);
    static Color darkslategray = Color(47, 79, 79);
    static Color darkturquoise = Color(0, 206, 209);
    static Color darkviolet = Color(148, 0, 211);
    static Color deeppink = Color(255, 20, 147);
    static Color deepskyblue = Color(0, 191, 255);
    static Color dimgray = Color(105, 105, 105);
    static Color dodgerblue = Color(30, 144, 255);
    static Color firebrick = Color(178, 34, 34);
    static Color floralwhite = Color(255, 250, 240);
    static Color forestgreen = Color(34, 139, 34);
    static Color fuchsia = Color(255, 0, 255);
    static Color gainsboro = Color(220, 220, 220);
    static Color ghostwhite = Color(248, 248, 255);
    static Color gold = Color(255, 215, 0);
    static Color goldenrod = Color(218, 165, 32);
    static Color gray = Color(128, 128, 128);
    static Color green = Color(0, 128, 0);
    static Color greenyellow = Color(173, 255, 47);
    static Color honeydew = Color(240, 255, 240);
    static Color hotpink = Color(255, 105, 180);
    static Color indianred = Color(205, 92, 92);
    static Color indigo = Color(75, 0, 130);
    static Color ivory = Color(255, 255, 240);
    static Color khaki = Color(240, 230, 140);
    static Color lavender = Color(230, 230, 250);
    static Color lavenderblush = Color(255, 240, 245);
    static Color lawngreen = Color(124, 252, 0);
    static Color lemonchiffon = Color(255, 250, 205);
    static Color lightblue = Color(173, 216, 230);
    static Color lightcoral = Color(240, 128, 128);
    static Color lightcyan = Color(224, 255, 255);
    static Color lightgoldenrodyellow = Color(250, 250, 210);
    static Color lightgray = Color(211, 211, 211);
    static Color lightgreen = Color(144, 238, 144);
    static Color lightpink = Color(255, 182, 193);
    static Color lightsalmon = Color(255, 160, 122);
    static Color lightseagreen = Color(32, 178, 170);
    static Color lightskyblue = Color(135, 206, 250);
    static Color lightslategray = Color(119, 136, 153);
    static Color lightsteelblue = Color(176, 196, 222);
    static Color lightyellow = Color(255, 255, 224);
    static Color lime = Color(0, 255, 0);
    static Color limegreen = Color(50, 205, 50);
    static Color linen = Color(250, 240, 230);
    static Color magenta = Color(255, 0, 255);
    static Color maroon = Color(128, 0, 0);
    static Color mediumaquamarine = Color(102, 205, 170);
    static Color mediumblue = Color(0, 0, 205);
    static Color mediumorchid = Color(186, 85, 211);
    static Color mediumpurple = Color(147, 112, 219);
    static Color mediumseagreen = Color(60, 179, 113);
    static Color mediumslateblue = Color(123, 104, 238);
    static Color mediumspringgreen = Color(0, 250, 154);
    static Color mediumturquoise = Color(72, 209, 204);
    static Color mediumvioletred = Color(199, 21, 133);
    static Color midnightblue = Color(25, 25, 112);
    static Color mintcream = Color(245, 255, 250);
    static Color mistyrose = Color(255, 228, 225);
    static Color moccasin = Color(255, 228, 181);
    static Color navajowhite = Color(255, 222, 173);
    static Color navy = Color(0, 0, 128);
    static Color oldlace = Color(253, 245, 230);
    static Color olive = Color(128, 128, 0);
    static Color olivedrab = Color(107, 142, 35);
    static Color orange = Color(255, 165, 0);
    static Color orangered = Color(255, 69, 0);
    static Color orchid = Color(218, 112, 214);
    static Color palegoldenrod = Color(238, 232, 170);
    static Color palegreen = Color(152, 251, 152);
    static Color paleturquoise = Color(175, 238, 238);
    static Color palevioletred = Color(219, 112, 147);
    static Color papayawhip = Color(255, 239, 213);
    static Color peachpuff = Color(255, 218, 185);
    static Color peru = Color(205, 133, 63);
    static Color pink = Color(255, 192, 203);
    static Color plum = Color(221, 160, 221);
    static Color powderblue = Color(176, 224, 230);
    static Color purple = Color(128, 0, 128);
    static Color rebeccapurple = Color(102, 51, 153);
    static Color red = Color(255, 0, 0);
    static Color rosybrown = Color(188, 143, 143);
    static Color royalblue = Color(65, 105, 225);
    static Color saddlebrown = Color(139, 69, 19);
    static Color salmon = Color(250, 128, 114);
    static Color sandybrown = Color(244, 164, 96);
    static Color seagreen = Color(46, 139, 87);
    static Color seashell = Color(255, 245, 238);
    static Color sienna = Color(160, 82, 45);
    static Color silver = Color(192, 192, 192);
    static Color skyblue = Color(135, 206, 235);
    static Color slateblue = Color(106, 90, 205);
    static Color slategray = Color(112, 128, 144);
    static Color snow = Color(255, 250, 250);
    static Color springgreen = Color(0, 255, 127);
    static Color steelblue = Color(70, 130, 180);
    static Color tan = Color(210, 180, 140);
    static Color teal = Color(0, 128, 128);
    static Color thistle = Color(216, 191, 216);
    static Color tomato = Color(255, 99, 71);
    static Color turquoise = Color(64, 224, 208);
    static Color violet = Color(238, 130, 238);
    static Color wheat = Color(245, 222, 179);
    static Color white = Color(255, 255, 255);
    static Color whitesmoke = Color(245, 245, 245);
    static Color yellow = Color(255, 255, 0);
    static Color yellowgreen = Color(154, 205, 50);
    
}

#endif // COLORS_HPP
