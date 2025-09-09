#pragma once

#include "../xdot/elements.h"
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <memory>

namespace xdot_cpp {
namespace ui {

class GraphicsShapeItem : public QGraphicsItem {
public:
    explicit GraphicsShapeItem(std::shared_ptr<xdot::Shape> shape, QGraphicsItem* parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    
    std::shared_ptr<xdot::Shape> shape() const { return shape_; }
    
protected:
    bool contains(const QPointF& point) const override;
    
private:
    std::shared_ptr<xdot::Shape> shape_;
    mutable QRectF bounding_rect_;
    mutable bool bounding_rect_valid_;
    
    void update_bounding_rect() const;
    QRectF shape_to_qt_rect(const xdot::BoundingBox& bbox) const;
    QPointF shape_to_qt_point(const xdot::Point& point) const;
};

class GraphicsNodeItem : public QGraphicsItem {
public:
    explicit GraphicsNodeItem(std::shared_ptr<xdot::GraphNode> node, QGraphicsItem* parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    
    std::shared_ptr<xdot::GraphNode> node() const { return node_; }
    
    void set_highlighted(bool highlighted);
    bool is_highlighted() const { return highlighted_; }
    
protected:
    bool contains(const QPointF& point) const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
private:
    std::shared_ptr<xdot::GraphNode> node_;
    std::vector<std::unique_ptr<GraphicsShapeItem>> shape_items_;
    bool highlighted_;
    mutable QRectF bounding_rect_;
    mutable bool bounding_rect_valid_;
    
    void create_shape_items();
    void update_bounding_rect() const;
};

class GraphicsEdgeItem : public QGraphicsItem {
public:
    explicit GraphicsEdgeItem(std::shared_ptr<xdot::GraphEdge> edge, QGraphicsItem* parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    
    std::shared_ptr<xdot::GraphEdge> edge() const { return edge_; }
    
    void set_highlighted(bool highlighted);
    bool is_highlighted() const { return highlighted_; }
    
protected:
    bool contains(const QPointF& point) const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
private:
    std::shared_ptr<xdot::GraphEdge> edge_;
    std::vector<std::unique_ptr<GraphicsShapeItem>> shape_items_;
    bool highlighted_;
    mutable QRectF bounding_rect_;
    mutable bool bounding_rect_valid_;
    
    void create_shape_items();
    void update_bounding_rect() const;
};

} // namespace ui
} // namespace xdot_cpp