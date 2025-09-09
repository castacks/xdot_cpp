#pragma once

#include "color.h"
#include <string>
#include <vector>

namespace xdot_cpp {
namespace xdot {

enum class LineStyle {
    SOLID,
    DASHED,
    DOTTED
};

struct Pen {
    Color color;
    Color fill_color;
    double line_width;
    LineStyle line_style;
    std::vector<double> dash_pattern;
    std::string font_family;
    double font_size;
    
    Pen() : color(0, 0, 0), fill_color(1, 1, 1), line_width(1.0), 
            line_style(LineStyle::SOLID), font_family("Arial"), font_size(12.0) {}
    
    void set_color(const Color& c) { color = c; }
    void set_fill_color(const Color& c) { fill_color = c; }
    void set_line_width(double width) { line_width = width; }
    void set_line_style(LineStyle style) { line_style = style; }
    void set_dash_pattern(const std::vector<double>& pattern) { dash_pattern = pattern; }
    void set_font(const std::string& family, double size) { 
        font_family = family; 
        font_size = size; 
    }
};

} // namespace xdot
} // namespace xdot_cpp