#include "xdot_cpp/xdot/elements.h"
#include <algorithm>
#include <cmath>

namespace xdot_cpp {
namespace xdot {

// BoundingBox implementation
bool BoundingBox::contains(const Point& p) const {
    return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
}

bool BoundingBox::intersects(const BoundingBox& other) const {
    return !(x2 < other.x1 || x1 > other.x2 || y2 < other.y1 || y1 > other.y2);
}

// EllipseShape implementation
EllipseShape::EllipseShape(const Point& center, double width, double height, const Pen& pen)
    : center_(center), width_(width), height_(height) {
    pen_ = pen;
}

BoundingBox EllipseShape::bounding_box() const {
    double half_width = width_ / 2.0;
    double half_height = height_ / 2.0;
    return BoundingBox(center_.x - half_width, center_.y - half_height,
                       center_.x + half_width, center_.y + half_height);
}

bool EllipseShape::contains_point(const Point& p) const {
    double dx = (p.x - center_.x) / (width_ / 2.0);
    double dy = (p.y - center_.y) / (height_ / 2.0);
    return (dx * dx + dy * dy) <= 1.0;
}

void EllipseShape::draw(Renderer* renderer) const {
    renderer->draw_ellipse(center_, width_, height_, pen_);
}

// PolygonShape implementation
PolygonShape::PolygonShape(const std::vector<Point>& points, const Pen& pen)
    : points_(points) {
    pen_ = pen;
}

BoundingBox PolygonShape::bounding_box() const {
    if (points_.empty()) {
        return BoundingBox();
    }
    
    double min_x = points_[0].x, max_x = points_[0].x;
    double min_y = points_[0].y, max_y = points_[0].y;
    
    for (const auto& point : points_) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    
    return BoundingBox(min_x, min_y, max_x, max_y);
}

bool PolygonShape::contains_point(const Point& p) const {
    if (points_.size() < 3) return false;
    
    // Ray casting algorithm
    bool inside = false;
    size_t j = points_.size() - 1;
    
    for (size_t i = 0; i < points_.size(); i++) {
        if (((points_[i].y > p.y) != (points_[j].y > p.y)) &&
            (p.x < (points_[j].x - points_[i].x) * (p.y - points_[i].y) / 
                   (points_[j].y - points_[i].y) + points_[i].x)) {
            inside = !inside;
        }
        j = i;
    }
    
    return inside;
}

void PolygonShape::draw(Renderer* renderer) const {
    renderer->draw_polygon(points_, pen_);
}

// PolylineShape implementation
PolylineShape::PolylineShape(const std::vector<Point>& points, const Pen& pen)
    : points_(points) {
    pen_ = pen;
}

BoundingBox PolylineShape::bounding_box() const {
    if (points_.empty()) {
        return BoundingBox();
    }
    
    double min_x = points_[0].x, max_x = points_[0].x;
    double min_y = points_[0].y, max_y = points_[0].y;
    
    for (const auto& point : points_) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    
    return BoundingBox(min_x, min_y, max_x, max_y);
}

bool PolylineShape::contains_point(const Point& p) const {
    // For polylines, check if point is close to any line segment
    const double tolerance = pen_.line_width + 2.0;
    
    for (size_t i = 0; i < points_.size() - 1; i++) {
        const Point& p1 = points_[i];
        const Point& p2 = points_[i + 1];
        
        // Distance from point to line segment
        double A = p.x - p1.x;
        double B = p.y - p1.y;
        double C = p2.x - p1.x;
        double D = p2.y - p1.y;
        
        double dot = A * C + B * D;
        double len_sq = C * C + D * D;
        
        if (len_sq == 0) continue; // Degenerate line segment
        
        double param = dot / len_sq;
        
        double xx, yy;
        if (param < 0) {
            xx = p1.x;
            yy = p1.y;
        } else if (param > 1) {
            xx = p2.x;
            yy = p2.y;
        } else {
            xx = p1.x + param * C;
            yy = p1.y + param * D;
        }
        
        double dx = p.x - xx;
        double dy = p.y - yy;
        double distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= tolerance) {
            return true;
        }
    }
    
    return false;
}

void PolylineShape::draw(Renderer* renderer) const {
    renderer->draw_polyline(points_, pen_);
}

// BezierShape implementation
BezierShape::BezierShape(const std::vector<Point>& control_points, const Pen& pen)
    : control_points_(control_points) {
    pen_ = pen;
}

BoundingBox BezierShape::bounding_box() const {
    if (control_points_.empty()) {
        return BoundingBox();
    }
    
    double min_x = control_points_[0].x, max_x = control_points_[0].x;
    double min_y = control_points_[0].y, max_y = control_points_[0].y;
    
    for (const auto& point : control_points_) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    
    return BoundingBox(min_x, min_y, max_x, max_y);
}

bool BezierShape::contains_point(const Point& p) const {
    // Simplified: check if point is close to control points
    const double tolerance = pen_.line_width + 5.0;
    
    for (const auto& cp : control_points_) {
        double dx = p.x - cp.x;
        double dy = p.y - cp.y;
        if (std::sqrt(dx * dx + dy * dy) <= tolerance) {
            return true;
        }
    }
    
    return false;
}

void BezierShape::draw(Renderer* renderer) const {
    renderer->draw_bezier(control_points_, pen_);
}

// TextShape implementation
TextShape::TextShape(const Point& position, const std::string& text, const Pen& pen)
    : position_(position), text_(text) {
    pen_ = pen;
}

BoundingBox TextShape::bounding_box() const {
    // Simple text bounding box estimation
    double text_width = text_.length() * pen_.font_size * 0.6;
    double text_height = pen_.font_size;
    
    // Center the bounding box around the position
    double half_width = text_width / 2.0;
    double half_height = text_height / 2.0;
    
    return BoundingBox(position_.x - half_width, position_.y - half_height,
                       position_.x + half_width, position_.y + half_height);
}

bool TextShape::contains_point(const Point& p) const {
    return bounding_box().contains(p);
}

void TextShape::draw(Renderer* renderer) const {
    renderer->draw_text(position_, text_, pen_);
}

// ImageShape implementation
ImageShape::ImageShape(const Point& position, double width, double height, const std::string& path)
    : position_(position), width_(width), height_(height), image_path_(path) {}

BoundingBox ImageShape::bounding_box() const {
    return BoundingBox(position_.x, position_.y, 
                       position_.x + width_, position_.y + height_);
}

bool ImageShape::contains_point(const Point& p) const {
    return bounding_box().contains(p);
}

void ImageShape::draw(Renderer* renderer) const {
    renderer->draw_image(position_, width_, height_, image_path_);
}

} // namespace xdot
} // namespace xdot_cpp