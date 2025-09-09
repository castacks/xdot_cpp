#include "xdot_cpp/xdot/xdot_parser.h"
#include "xdot_cpp/xdot/color.h"
#include "xdot_cpp/xdot/graph.h"
#include <sstream>
#include <cctype>
#include <cmath>

namespace xdot_cpp {
namespace xdot {

// XDotAttrParser implementation
XDotAttrParser::XDotAttrParser(const std::string& xdot_data, bool broken_backslashes)
    : data_(xdot_data), pos_(0) {
    // Handle broken backslashes if needed
    if (broken_backslashes) {
        // Replace double backslashes with single backslashes
        std::string fixed_data;
        for (size_t i = 0; i < data_.length(); i++) {
            if (data_[i] == '\\' && i + 1 < data_.length() && data_[i + 1] == '\\') {
                fixed_data += '\\';
                i++; // Skip the second backslash
            } else {
                fixed_data += data_[i];
            }
        }
        data_ = fixed_data;
    }
}

std::vector<std::shared_ptr<Shape>> XDotAttrParser::parse() {
    shapes_.clear();
    pos_ = 0;
    
    while (has_more()) {
        skip_whitespace();
        if (!has_more()) break;
        
        std::string code = read_code();
        
        if (code == "E") {
            handle_ellipse();
        } else if (code == "P") {
            handle_polygon();
        } else if (code == "L") {
            handle_polyline();
        } else if (code == "B") {
            handle_bezier();
        } else if (code == "T") {
            handle_text();
        } else if (code == "I") {
            handle_image();
        } else if (code == "S") {
            handle_style();
        } else if (code == "c") {
            handle_color();
        } else if (code == "C") {
            handle_fill_color();
        } else if (code == "F") {
            handle_font();
        }
        // Ignore unknown codes
    }
    
    return shapes_;
}

bool XDotAttrParser::has_more() const {
    return pos_ < data_.length();
}

char XDotAttrParser::current_char() const {
    if (pos_ >= data_.length()) return '\0';
    return data_[pos_];
}

void XDotAttrParser::advance() {
    if (pos_ < data_.length()) pos_++;
}

void XDotAttrParser::skip_whitespace() {
    while (has_more() && std::isspace(current_char())) {
        advance();
    }
}

std::string XDotAttrParser::read_code() {
    skip_whitespace();
    std::string code;
    while (has_more() && std::isalpha(current_char())) {
        code += current_char();
        advance();
    }
    return code;
}

int XDotAttrParser::read_int() {
    skip_whitespace();
    std::string num_str;
    bool negative = false;
    
    if (current_char() == '-') {
        negative = true;
        advance();
    }
    
    while (has_more() && std::isdigit(current_char())) {
        num_str += current_char();
        advance();
    }
    
    if (num_str.empty()) return 0;
    
    int result = std::stoi(num_str);
    return negative ? -result : result;
}

double XDotAttrParser::read_float() {
    skip_whitespace();
    std::string num_str;
    bool negative = false;
    
    if (current_char() == '-') {
        negative = true;
        advance();
    }
    
    while (has_more() && (std::isdigit(current_char()) || current_char() == '.')) {
        num_str += current_char();
        advance();
    }
    
    if (num_str.empty()) return 0.0;
    
    double result = std::stod(num_str);
    return negative ? -result : result;
}

Point XDotAttrParser::read_point() {
    double x = read_float();
    double y = read_float();
    return transform(x, y);
}

std::string XDotAttrParser::read_text() {
    skip_whitespace();
    int length = read_int();
    skip_whitespace();
    
    if (current_char() == '-') {
        advance(); // Skip the dash separator
        skip_whitespace();
    }
    
    std::string text;
    for (int i = 0; i < length && has_more(); i++) {
        text += current_char();
        advance();
    }
    
    return text;
}

std::vector<Point> XDotAttrParser::read_polygon() {
    int num_points = read_int();
    std::vector<Point> points;
    
    for (int i = 0; i < num_points; i++) {
        points.push_back(read_point());
    }
    
    return points;
}

Color XDotAttrParser::read_color() {
    std::string color_spec = read_text();
    return ColorLookup::lookup_color(color_spec);
}

Point XDotAttrParser::transform(double x, double y) const {
    // For now, just return the coordinates as-is
    // In a full implementation, this would apply coordinate transformations
    return Point(x, y);
}

void XDotAttrParser::handle_ellipse() {
    Point center = read_point();
    double width = read_float();
    double height = read_float();
    
    auto ellipse = std::make_shared<EllipseShape>(center, width, height, current_pen_);
    shapes_.push_back(ellipse);
}

void XDotAttrParser::handle_polygon() {
    std::vector<Point> points = read_polygon();
    
    auto polygon = std::make_shared<PolygonShape>(points, current_pen_);
    shapes_.push_back(polygon);
}

void XDotAttrParser::handle_polyline() {
    std::vector<Point> points = read_polygon(); // Same format as polygon
    
    auto polyline = std::make_shared<PolylineShape>(points, current_pen_);
    shapes_.push_back(polyline);
}

void XDotAttrParser::handle_bezier() {
    std::vector<Point> control_points = read_polygon(); // Same format as polygon
    
    auto bezier = std::make_shared<BezierShape>(control_points, current_pen_);
    shapes_.push_back(bezier);
}

void XDotAttrParser::handle_text() {
    Point position = read_point();
    int alignment = read_int(); // Text alignment (ignored for now)
    double width = read_float(); // Text width (ignored for now)
    std::string text = read_text();
    
    auto text_shape = std::make_shared<TextShape>(position, text, current_pen_);
    shapes_.push_back(text_shape);
}

void XDotAttrParser::handle_image() {
    Point position = read_point();
    double width = read_float();
    double height = read_float();
    std::string image_path = read_text();
    
    auto image = std::make_shared<ImageShape>(position, width, height, image_path);
    shapes_.push_back(image);
}

void XDotAttrParser::handle_style() {
    std::string style = read_text();
    
    // Parse style attributes (simplified)
    if (style.find("solid") != std::string::npos) {
        current_pen_.set_line_style(LineStyle::SOLID);
    } else if (style.find("dashed") != std::string::npos) {
        current_pen_.set_line_style(LineStyle::DASHED);
    } else if (style.find("dotted") != std::string::npos) {
        current_pen_.set_line_style(LineStyle::DOTTED);
    }
}

void XDotAttrParser::handle_color() {
    Color color = read_color();
    current_pen_.set_color(color);
}

void XDotAttrParser::handle_fill_color() {
    Color fill_color = read_color();
    current_pen_.set_fill_color(fill_color);
}

void XDotAttrParser::handle_font() {
    double font_size = read_float();
    std::string font_name = read_text();
    current_pen_.set_font(font_name, font_size);
}

// XDotParser implementation
XDotParser::XDotParser(std::shared_ptr<dot::Graph> graph) : graph_(graph) {}

std::shared_ptr<GraphElement> XDotParser::parse() {
    auto graph_element = std::make_shared<GraphElement>();
    
    // Parse graph background shapes from graph attributes
    std::string graph_draw = get_attribute_value(graph_->attributes, "_draw_");
    if (!graph_draw.empty()) {
        auto bg_shapes = parse_xdot_attr(graph_draw);
        for (auto& shape : bg_shapes) {
            graph_element->add_background_shape(shape);
        }
    }
    
    // Parse nodes
    for (const auto& node : graph_->nodes) {
        std::string node_draw = get_attribute_value(node->attributes, "_draw_");
        std::string node_ldraw = get_attribute_value(node->attributes, "_ldraw_");
        
        std::vector<std::shared_ptr<Shape>> node_shapes;
        
        // Parse node drawing commands
        if (!node_draw.empty()) {
            auto shapes = parse_xdot_attr(node_draw);
            node_shapes.insert(node_shapes.end(), shapes.begin(), shapes.end());
        }
        
        // Parse node label drawing commands
        if (!node_ldraw.empty()) {
            auto shapes = parse_xdot_attr(node_ldraw);
            node_shapes.insert(node_shapes.end(), shapes.begin(), shapes.end());
        }
        
        if (!node_shapes.empty()) {
            auto graph_node = std::make_shared<GraphNode>(node->id, node_shapes);
            
            // Set URL if present
            std::string url = get_attribute_value(node->attributes, "URL");
            if (!url.empty()) {
                graph_node->set_url(url);
            }
            
            graph_element->add_node(graph_node);
        }
    }
    
    // Parse edges
    for (const auto& edge : graph_->edges) {
        std::string edge_draw = get_attribute_value(edge->attributes, "_draw_");
        std::string edge_hdraw = get_attribute_value(edge->attributes, "_hdraw_");
        std::string edge_ldraw = get_attribute_value(edge->attributes, "_ldraw_");
        
        std::vector<std::shared_ptr<Shape>> edge_shapes;
        
        // Parse edge drawing commands
        if (!edge_draw.empty()) {
            auto shapes = parse_xdot_attr(edge_draw);
            edge_shapes.insert(edge_shapes.end(), shapes.begin(), shapes.end());
        }
        
        // Parse edge head drawing commands
        if (!edge_hdraw.empty()) {
            auto shapes = parse_xdot_attr(edge_hdraw);
            edge_shapes.insert(edge_shapes.end(), shapes.begin(), shapes.end());
        }
        
        // Parse edge label drawing commands
        if (!edge_ldraw.empty()) {
            auto shapes = parse_xdot_attr(edge_ldraw);
            edge_shapes.insert(edge_shapes.end(), shapes.begin(), shapes.end());
        }
        
        if (!edge_shapes.empty()) {
            auto graph_edge = std::make_shared<GraphEdge>(edge->source, edge->target, edge_shapes);
            
            // Set URL if present
            std::string url = get_attribute_value(edge->attributes, "URL");
            if (!url.empty()) {
                graph_edge->set_url(url);
            }
            
            graph_element->add_edge(graph_edge);
        }
    }
    
    return graph_element;
}

std::vector<std::shared_ptr<Shape>> XDotParser::parse_xdot_attr(const std::string& xdot_data) {
    XDotAttrParser parser(xdot_data);
    return parser.parse();
}

std::string XDotParser::get_attribute_value(const dot::AttributeList& attrs, 
                                           const std::string& name, 
                                           const std::string& default_value) {
    for (const auto& attr : attrs) {
        if (attr.name == name) {
            return attr.value;
        }
    }
    return default_value;
}

} // namespace xdot
} // namespace xdot_cpp