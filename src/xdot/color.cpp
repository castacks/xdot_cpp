#include "xdot_cpp/xdot/color.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace xdot_cpp {
namespace xdot {

std::map<std::string, Color> ColorLookup::color_map_;

Color Color::from_hex(const std::string& hex) {
    std::string hex_str = hex;
    if (hex_str[0] == '#') {
        hex_str = hex_str.substr(1);
    }
    
    if (hex_str.length() == 3) {
        // Short form: #RGB -> #RRGGBB
        hex_str = std::string(2, hex_str[0]) + 
                  std::string(2, hex_str[1]) + 
                  std::string(2, hex_str[2]);
    }
    
    if (hex_str.length() != 6) {
        return Color(); // Return black for invalid input
    }
    
    unsigned int r, g, b;
    std::stringstream ss;
    ss << std::hex << hex_str.substr(0, 2);
    ss >> r;
    ss.clear();
    ss.str("");
    ss << std::hex << hex_str.substr(2, 2);
    ss >> g;
    ss.clear();
    ss.str("");
    ss << std::hex << hex_str.substr(4, 2);
    ss >> b;
    
    return Color(r / 255.0, g / 255.0, b / 255.0);
}

Color Color::from_hsv(double h, double s, double v) {
    double c = v * s;
    double x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
    double m = v - c;
    
    double r, g, b;
    
    if (h >= 0 && h < 60) {
        r = c; g = x; b = 0;
    } else if (h >= 60 && h < 120) {
        r = x; g = c; b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0; g = c; b = x;
    } else if (h >= 180 && h < 240) {
        r = 0; g = x; b = c;
    } else if (h >= 240 && h < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    return Color(r + m, g + m, b + m);
}

Color Color::from_name(const std::string& name) {
    return ColorLookup::lookup_color(name);
}

std::string Color::to_hex() const {
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(r * 255)
       << std::setw(2) << static_cast<int>(g * 255)
       << std::setw(2) << static_cast<int>(b * 255);
    return ss.str();
}

void Color::to_hsv(double& h, double& s, double& v) const {
    double max_val = std::max({r, g, b});
    double min_val = std::min({r, g, b});
    double delta = max_val - min_val;
    
    v = max_val;
    s = (max_val == 0) ? 0 : delta / max_val;
    
    if (delta == 0) {
        h = 0;
    } else if (max_val == r) {
        h = 60 * std::fmod((g - b) / delta, 6);
    } else if (max_val == g) {
        h = 60 * ((b - r) / delta + 2);
    } else {
        h = 60 * ((r - g) / delta + 4);
    }
    
    if (h < 0) h += 360;
}

Color ColorLookup::lookup_color(const std::string& color_spec) {
    if (color_map_.empty()) {
        init_color_map();
    }
    
    std::string spec = color_spec;
    std::transform(spec.begin(), spec.end(), spec.begin(), ::tolower);
    
    // Try hex color first
    if (spec[0] == '#') {
        return Color::from_hex(spec);
    }
    
    // Try named color
    auto it = color_map_.find(spec);
    if (it != color_map_.end()) {
        return it->second;
    }
    
    // Try RGB format: rgb(r,g,b)
    if (spec.substr(0, 4) == "rgb(") {
        // Simple RGB parsing - could be enhanced
        return Color(); // Return black for now
    }
    
    // Default to black
    return Color();
}

void ColorLookup::init_color_map() {
    // Basic X11 color names
    color_map_["black"] = Color(0, 0, 0);
    color_map_["white"] = Color(1, 1, 1);
    color_map_["red"] = Color(1, 0, 0);
    color_map_["green"] = Color(0, 0.5, 0);
    color_map_["blue"] = Color(0, 0, 1);
    color_map_["yellow"] = Color(1, 1, 0);
    color_map_["cyan"] = Color(0, 1, 1);
    color_map_["magenta"] = Color(1, 0, 1);
    color_map_["gray"] = Color(0.5, 0.5, 0.5);
    color_map_["grey"] = Color(0.5, 0.5, 0.5);
    color_map_["orange"] = Color(1, 0.647, 0);
    color_map_["purple"] = Color(0.5, 0, 0.5);
    color_map_["brown"] = Color(0.647, 0.165, 0.165);
    color_map_["pink"] = Color(1, 0.753, 0.796);
    color_map_["lightblue"] = Color(0.678, 0.847, 0.902);
    color_map_["lightgreen"] = Color(0.565, 0.933, 0.565);
    color_map_["lightgray"] = Color(0.827, 0.827, 0.827);
    color_map_["lightgrey"] = Color(0.827, 0.827, 0.827);
    color_map_["darkblue"] = Color(0, 0, 0.545);
    color_map_["darkgreen"] = Color(0, 0.392, 0);
    color_map_["darkgray"] = Color(0.663, 0.663, 0.663);
    color_map_["darkgrey"] = Color(0.663, 0.663, 0.663);
    color_map_["transparent"] = Color(0, 0, 0, 0);
}

} // namespace xdot
} // namespace xdot_cpp