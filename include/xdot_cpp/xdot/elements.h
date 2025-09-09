#pragma once

#include "pen.h"
#include <vector>
#include <memory>
#include <string>

namespace xdot_cpp {
namespace xdot {

struct Point {
    double x, y;
    Point(double x_val = 0.0, double y_val = 0.0) : x(x_val), y(y_val) {}
};

struct BoundingBox {
    double x1, y1, x2, y2;
    BoundingBox(double x1_val = 0.0, double y1_val = 0.0, double x2_val = 0.0, double y2_val = 0.0)
        : x1(x1_val), y1(y1_val), x2(x2_val), y2(y2_val) {}
    
    bool contains(const Point& p) const;
    bool intersects(const BoundingBox& other) const;
    double width() const { return x2 - x1; }
    double height() const { return y2 - y1; }
};

class Shape {
public:
    virtual ~Shape() = default;
    virtual BoundingBox bounding_box() const = 0;
    virtual bool contains_point(const Point& p) const = 0;
    virtual void draw(class Renderer* renderer) const = 0;
    
protected:
    Pen pen_;
};

class EllipseShape : public Shape {
public:
    EllipseShape(const Point& center, double width, double height, const Pen& pen);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
private:
    Point center_;
    double width_, height_;
};

class PolygonShape : public Shape {
public:
    PolygonShape(const std::vector<Point>& points, const Pen& pen);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
private:
    std::vector<Point> points_;
};

class PolylineShape : public Shape {
public:
    PolylineShape(const std::vector<Point>& points, const Pen& pen);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
private:
    std::vector<Point> points_;
};

class BezierShape : public Shape {
public:
    BezierShape(const std::vector<Point>& control_points, const Pen& pen);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
private:
    std::vector<Point> control_points_;
};

class TextShape : public Shape {
public:
    TextShape(const Point& position, const std::string& text, const Pen& pen);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
    const std::string& text() const { return text_; }
    
private:
    Point position_;
    std::string text_;
};

class ImageShape : public Shape {
public:
    ImageShape(const Point& position, double width, double height, const std::string& path);
    
    BoundingBox bounding_box() const override;
    bool contains_point(const Point& p) const override;
    void draw(Renderer* renderer) const override;
    
private:
    Point position_;
    double width_, height_;
    std::string image_path_;
};

class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual void draw_ellipse(const Point& center, double width, double height, const Pen& pen) = 0;
    virtual void draw_polygon(const std::vector<Point>& points, const Pen& pen) = 0;
    virtual void draw_polyline(const std::vector<Point>& points, const Pen& pen) = 0;
    virtual void draw_bezier(const std::vector<Point>& control_points, const Pen& pen) = 0;
    virtual void draw_text(const Point& position, const std::string& text, const Pen& pen) = 0;
    virtual void draw_image(const Point& position, double width, double height, const std::string& path) = 0;
};

} // namespace xdot
} // namespace xdot_cpp