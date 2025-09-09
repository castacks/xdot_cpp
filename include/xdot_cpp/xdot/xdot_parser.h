#pragma once

#include "elements.h"
#include "pen.h"
#include "../dot/parser.h"
#include <string>
#include <vector>
#include <memory>

namespace xdot_cpp {
namespace xdot {

class XDotAttrParser {
public:
    XDotAttrParser(const std::string& xdot_data, bool broken_backslashes = false);
    
    std::vector<std::shared_ptr<Shape>> parse();
    
private:
    std::string data_;
    size_t pos_;
    Pen current_pen_;
    
    bool has_more() const;
    char current_char() const;
    void advance();
    void skip_whitespace();
    
    std::string read_code();
    int read_int();
    double read_float();
    Point read_point();
    std::string read_text();
    std::vector<Point> read_polygon();
    Color read_color();
    
    Point transform(double x, double y) const;
    
    void handle_ellipse();
    void handle_polygon();
    void handle_polyline();
    void handle_bezier();
    void handle_text();
    void handle_image();
    void handle_style();
    void handle_color();
    void handle_fill_color();
    void handle_font();
    
    std::vector<std::shared_ptr<Shape>> shapes_;
};

class XDotParser {
public:
    explicit XDotParser(std::shared_ptr<dot::Graph> graph);
    
    std::shared_ptr<class GraphElement> parse();
    
private:
    std::shared_ptr<dot::Graph> graph_;
    
    std::vector<std::shared_ptr<Shape>> parse_xdot_attr(const std::string& xdot_data);
    std::string get_attribute_value(const dot::AttributeList& attrs, const std::string& name, const std::string& default_value = "");
};

} // namespace xdot
} // namespace xdot_cpp