#include "xdot_cpp/ui/graphics_items.h"
#include "xdot_cpp/ui/dot_widget.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QPainter>

namespace xdot_cpp {
namespace ui {

// GraphicsShapeItem implementation
GraphicsShapeItem::GraphicsShapeItem(std::shared_ptr<xdot::Shape> shape, QGraphicsItem* parent)
    : QGraphicsItem(parent), shape_(shape), bounding_rect_valid_(false) {
}

QRectF GraphicsShapeItem::boundingRect() const {
    if (!bounding_rect_valid_) {
        update_bounding_rect();
    }
    return bounding_rect_;
}

void GraphicsShapeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    if (!shape_) return;
    
    // Enable high-quality rendering
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // Use QtRenderer to render the shape with vector graphics
    QtRenderer renderer(painter);
    shape_->draw(&renderer);
}

bool GraphicsShapeItem::contains(const QPointF& point) const {
    if (!shape_) return false;
    
    xdot::Point xdot_point(point.x(), point.y());
    return shape_->contains_point(xdot_point);
}

void GraphicsShapeItem::update_bounding_rect() const {
    if (!shape_) {
        bounding_rect_ = QRectF();
        bounding_rect_valid_ = true;
        return;
    }
    
    xdot::BoundingBox bbox = shape_->bounding_box();
    bounding_rect_ = shape_to_qt_rect(bbox);
    bounding_rect_valid_ = true;
}

QRectF GraphicsShapeItem::shape_to_qt_rect(const xdot::BoundingBox& bbox) const {
    return QRectF(bbox.x1, bbox.y1, 
                  bbox.x2 - bbox.x1, 
                  bbox.y2 - bbox.y1);
}

QPointF GraphicsShapeItem::shape_to_qt_point(const xdot::Point& point) const {
    return QPointF(point.x, point.y);
}

// GraphicsNodeItem implementation
GraphicsNodeItem::GraphicsNodeItem(std::shared_ptr<xdot::GraphNode> node, QGraphicsItem* parent)
    : QGraphicsItem(parent), node_(node), highlighted_(false), bounding_rect_valid_(false) {
    setAcceptHoverEvents(true);
    create_shape_items();
}

QRectF GraphicsNodeItem::boundingRect() const {
    if (!bounding_rect_valid_) {
        update_bounding_rect();
    }
    return bounding_rect_;
}

void GraphicsNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // The child shape items will handle the actual painting
    // This method can be used for additional node-level rendering if needed
    
    if (highlighted_) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        QPen highlight_pen(Qt::red, 2.0);
        painter->setPen(highlight_pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void GraphicsNodeItem::set_highlighted(bool highlighted) {
    if (highlighted_ != highlighted) {
        highlighted_ = highlighted;
        update();
        if (node_) {
            node_->set_highlighted(highlighted);
        }
    }
}

bool GraphicsNodeItem::contains(const QPointF& point) const {
    if (!node_) return false;
    
    xdot::Point xdot_point(point.x(), point.y());
    return node_->bounding_box().contains(xdot_point);
}

void GraphicsNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Find the DotWidget in the scene hierarchy
        if (auto* dot_widget = qobject_cast<DotWidget*>(scene()->parent())) {
            emit dot_widget->node_clicked(QString::fromStdString(node_->id()),
                                        QString::fromStdString(node_->url()));
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void GraphicsNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    setCursor(Qt::PointingHandCursor);
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphicsNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverLeaveEvent(event);
}

void GraphicsNodeItem::create_shape_items() {
    if (!node_) return;
    
    shape_items_.clear();
    
    for (const auto& shape : node_->shapes()) {
        auto shape_item = std::make_unique<GraphicsShapeItem>(shape, this);
        shape_items_.push_back(std::move(shape_item));
    }
    
    bounding_rect_valid_ = false;
}

void GraphicsNodeItem::update_bounding_rect() const {
    if (!node_) {
        bounding_rect_ = QRectF();
        bounding_rect_valid_ = true;
        return;
    }
    
    xdot::BoundingBox bbox = node_->bounding_box();
    bounding_rect_ = QRectF(bbox.x1, bbox.y1, 
                           bbox.x2 - bbox.x1, 
                           bbox.y2 - bbox.y1);
    bounding_rect_valid_ = true;
}

// GraphicsEdgeItem implementation
GraphicsEdgeItem::GraphicsEdgeItem(std::shared_ptr<xdot::GraphEdge> edge, QGraphicsItem* parent)
    : QGraphicsItem(parent), edge_(edge), highlighted_(false), bounding_rect_valid_(false) {
    setAcceptHoverEvents(true);
    create_shape_items();
}

QRectF GraphicsEdgeItem::boundingRect() const {
    if (!bounding_rect_valid_) {
        update_bounding_rect();
    }
    return bounding_rect_;
}

void GraphicsEdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // The child shape items will handle the actual painting
    // This method can be used for additional edge-level rendering if needed
    
    if (highlighted_) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        QPen highlight_pen(Qt::blue, 2.0);
        painter->setPen(highlight_pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void GraphicsEdgeItem::set_highlighted(bool highlighted) {
    if (highlighted_ != highlighted) {
        highlighted_ = highlighted;
        update();
        if (edge_) {
            edge_->set_highlighted(highlighted);
        }
    }
}

bool GraphicsEdgeItem::contains(const QPointF& point) const {
    if (!edge_) return false;
    
    xdot::Point xdot_point(point.x(), point.y());
    return edge_->bounding_box().contains(xdot_point);
}

void GraphicsEdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Find the DotWidget in the scene hierarchy
        if (auto* dot_widget = qobject_cast<DotWidget*>(scene()->parent())) {
            emit dot_widget->edge_clicked(QString::fromStdString(edge_->source()),
                                        QString::fromStdString(edge_->target()),
                                        QString::fromStdString(edge_->url()));
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void GraphicsEdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    setCursor(Qt::PointingHandCursor);
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphicsEdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverLeaveEvent(event);
}

void GraphicsEdgeItem::create_shape_items() {
    if (!edge_) return;
    
    shape_items_.clear();
    
    for (const auto& shape : edge_->shapes()) {
        auto shape_item = std::make_unique<GraphicsShapeItem>(shape, this);
        shape_items_.push_back(std::move(shape_item));
    }
    
    bounding_rect_valid_ = false;
}

void GraphicsEdgeItem::update_bounding_rect() const {
    if (!edge_) {
        bounding_rect_ = QRectF();
        bounding_rect_valid_ = true;
        return;
    }
    
    xdot::BoundingBox bbox = edge_->bounding_box();
    bounding_rect_ = QRectF(bbox.x1, bbox.y1, 
                           bbox.x2 - bbox.x1, 
                           bbox.y2 - bbox.y1);
    bounding_rect_valid_ = true;
}

} // namespace ui
} // namespace xdot_cpp