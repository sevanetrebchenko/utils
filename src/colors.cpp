
#include "utils/colors.hpp"

namespace utils {
    
    std::unordered_map<std::string_view, Color, std::hash<std::string_view>, ColorComparator> colors = {
        { "aliceblue", aliceblue },
        { "antiquewhite", antiquewhite },
        { "aqua", aqua },
        { "aquamarine", aquamarine },
        { "azure", azure },
        { "beige", beige },
        { "bisque", bisque },
        { "black", black },
        { "blanchedalmond", blanchedalmond },
        { "blue", blue },
        { "blueviolet", blueviolet },
        { "brown", brown },
        { "burlywood", burlywood },
        { "cadetblue", cadetblue },
        { "chartreuse", chartreuse },
        { "chocolate", chocolate },
        { "coral", coral },
        { "cornflowerblue", cornflowerblue },
        { "cornsilk", cornsilk },
        { "crimson", crimson },
        { "cyan", cyan },
        { "darkblue", darkblue },
        { "darkcyan", darkcyan },
        { "darkgoldenrod", darkgoldenrod },
        { "darkgray", darkgray },
        { "darkgreen", darkgreen },
        { "darkkhaki", darkkhaki },
        { "darkmagenta", darkmagenta },
        { "darkolivegreen", darkolivegreen },
        { "darkorange", darkorange },
        { "darkorchid", darkorchid },
        { "darkred", darkred },
        { "darksalmon", darksalmon },
        { "darkseagreen", darkseagreen },
        { "darkslateblue", darkslateblue },
        { "darkslategray", darkslategray },
        { "darkturquoise", darkturquoise },
        { "darkviolet", darkviolet },
        { "deeppink", deeppink },
        { "deepskyblue", deepskyblue },
        { "dimgray", dimgray },
        { "dodgerblue", dodgerblue },
        { "firebrick", firebrick },
        { "floralwhite", floralwhite },
        { "forestgreen", forestgreen },
        { "fuchsia", fuchsia },
        { "gainsboro", gainsboro },
        { "ghostwhite", ghostwhite },
        { "gold", gold },
        { "goldenrod", goldenrod },
        { "gray", gray },
        { "green", green },
        { "greenyellow", greenyellow },
        { "honeydew", honeydew },
        { "hotpink", hotpink },
        { "indianred", indianred },
        { "indigo", indigo },
        { "ivory", ivory },
        { "khaki", khaki },
        { "lavender", lavender },
        { "lavenderblush", lavenderblush },
        { "lawngreen", lawngreen },
        { "lemonchiffon", lemonchiffon },
        { "lightblue", lightblue },
        { "lightcoral", lightcoral },
        { "lightcyan", lightcyan },
        { "lightgoldenrodyellow", lightgoldenrodyellow },
        { "lightgray", lightgray },
        { "lightgreen", lightgreen },
        { "lightpink", lightpink },
        { "lightsalmon", lightsalmon },
        { "lightseagreen", lightseagreen },
        { "lightskyblue", lightskyblue },
        { "lightslategray", lightslategray },
        { "lightsteelblue", lightsteelblue },
        { "lightyellow", lightyellow },
        { "lime", lime },
        { "limegreen", limegreen },
        { "linen", linen },
        { "magenta", magenta },
        { "maroon", maroon },
        { "mediumaquamarine", mediumaquamarine },
        { "mediumblue", mediumblue },
        { "mediumorchid", mediumorchid },
        { "mediumpurple", mediumpurple },
        { "mediumseagreen", mediumseagreen },
        { "mediumslateblue", mediumslateblue },
        { "mediumspringgreen", mediumspringgreen },
        { "mediumturquoise", mediumturquoise },
        { "mediumvioletred", mediumvioletred },
        { "midnightblue", midnightblue },
        { "mintcream", mintcream },
        { "mistyrose", mistyrose },
        { "moccasin", moccasin },
        { "navajowhite", navajowhite },
        { "navy", navy },
        { "oldlace", oldlace },
        { "olive", olive },
        { "olivedrab", olivedrab },
        { "orange", orange },
        { "orangered", orangered },
        { "orchid", orchid },
        { "palegoldenrod", palegoldenrod },
        { "palegreen", palegreen },
        { "paleturquoise", paleturquoise },
        { "palevioletred", palevioletred },
        { "papayawhip", papayawhip },
        { "peachpuff", peachpuff },
        { "peru", peru },
        { "pink", pink },
        { "plum", plum },
        { "powderblue", powderblue },
        { "purple", purple },
        { "rebeccapurple", rebeccapurple },
        { "red", red },
        { "rosybrown", rosybrown },
        { "royalblue", royalblue },
        { "saddlebrown", saddlebrown },
        { "salmon", salmon },
        { "sandybrown", sandybrown },
        { "seagreen", seagreen },
        { "seashell", seashell },
        { "sienna", sienna },
        { "silver", silver },
        { "skyblue", skyblue },
        { "slateblue", slateblue },
        { "slategray", slategray },
        { "snow", snow },
        { "springgreen", springgreen },
        { "steelblue", steelblue },
        { "tan", tan },
        { "teal", teal },
        { "thistle", thistle },
        { "tomato", tomato },
        { "turquoise", turquoise },
        { "violet", violet },
        { "wheat", wheat },
        { "white", white },
        { "whitesmoke", whitesmoke },
        { "yellow", yellow },
        { "yellowgreen", yellowgreen }
    };

    bool ColorComparator::operator()(std::string_view first, std::string_view second) const {
        return icasecmp(first, second);
    }
    
    Color ansi_to_rgb(std::uint8_t code) {
        if (code < 16) {
            // Standard colors (0-15)
            static const Color standard_colors[] = {
                { 0, 0, 0 },         // Black
                { 128, 0, 0 },       // Red
                { 0, 128, 0 },       // Green
                { 128, 128, 0 },     // Yellow
                { 0, 0, 128 },       // Blue
                { 128, 0, 128 },     // Magenta
                { 0, 128, 128 },     // Cyan
                { 192, 192, 192 },   // White (Light Gray)
                { 128, 128, 128 },   // Dark Gray
                { 255, 0, 0 },       // Bright Red
                { 0, 255, 0 },       // Bright Green
                { 255, 255, 0 },     // Bright Yellow
                { 0, 0, 255 },       // Bright Blue
                { 255, 0, 255 },     // Bright Magenta
                { 0, 255, 255 },     // Bright Cyan
                { 255, 255, 255 }    // Bright White
            };
            return standard_colors[code];
        }
        else if (code >= 16 && code <= 231) {
            // 6x6x6 color cube (16-231)
            code -= 16;
            std::uint8_t r = (code / 36) % 6 * 51;
            std::uint8_t g = (code / 6) % 6 * 51;
            std::uint8_t b = code % 6 * 51;
            return {r, g, b};
        }
        else if (code >= 232 && code <= 255) {
            // Grayscale (232-255)
            std::uint8_t g = 8 + (code - 232) * 10;
            return { g, g, g };
        }
        else {
            // Invalid code, return black
            return { 0, 0, 0 };
        }
    }
    
}