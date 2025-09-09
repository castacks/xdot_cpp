#pragma once

#include <string>
#include <map>

namespace xdot_cpp {
namespace xdot {

struct Color {
    double r, g, b, a;
    
    Color(double red = 0.0, double green = 0.0, double blue = 0.0, double alpha = 1.0)
        : r(red), g(green), b(blue), a(alpha) {}
    
    static Color from_hex(const std::string& hex);
    static Color from_hsv(double h, double s, double v);
    static Color from_name(const std::string& name);
    
    std::string to_hex() const;
    void to_hsv(double& h, double& s, double& v) const;
};

class ColorLookup {
public:
    static Color lookup_color(const std::string& color_spec);
    
private:
    static std::map<std::string, Color> color_map_;
    static void init_color_map();
};

} // namespace xdot
} // namespace xdot_cpp