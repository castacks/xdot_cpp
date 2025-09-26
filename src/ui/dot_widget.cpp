#include "xdot_cpp/ui/dot_widget.h"
#include "xdot_cpp/ui/graphics_items.h"
#include "xdot_cpp/dot/parser.h"
#include "xdot_cpp/xdot/xdot_parser.h"
#include <QApplication>
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPainterPath>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <cmath>
#include <unordered_set>

namespace xdot_cpp {
namespace ui {

// QtRenderer implementation
QtRenderer::QtRenderer(QPainter* painter) : painter_(painter) {}

void QtRenderer::draw_ellipse(const xdot::Point& center, double width, double height, const xdot::Pen& pen) {
    painter_->setPen(create_qpen(pen));
    painter_->setBrush(create_qbrush(pen));
    
    QRectF rect(center.x - width/2, center.y - height/2, width, height);
    painter_->drawEllipse(rect);
}

void QtRenderer::draw_polygon(const std::vector<xdot::Point>& points, const xdot::Pen& pen) {
    if (points.empty()) return;
    
    painter_->setPen(create_qpen(pen));
    painter_->setBrush(create_qbrush(pen));
    
    QPolygonF polygon;
    for (const auto& point : points) {
        polygon << QPointF(point.x, point.y);
    }
    
    painter_->drawPolygon(polygon);
}

void QtRenderer::draw_polyline(const std::vector<xdot::Point>& points, const xdot::Pen& pen) {
    if (points.empty()) return;
    
    painter_->setPen(create_qpen(pen));
    painter_->setBrush(Qt::NoBrush);
    
    QPolygonF polyline;
    for (const auto& point : points) {
        polyline << QPointF(point.x, point.y);
    }
    
    painter_->drawPolyline(polyline);
}

void QtRenderer::draw_bezier(const std::vector<xdot::Point>& control_points, const xdot::Pen& pen) {
    if (control_points.size() < 4) return;
    
    painter_->setPen(create_qpen(pen));
    painter_->setBrush(Qt::NoBrush);
    
    QPainterPath path;
    path.moveTo(control_points[0].x, control_points[0].y);
    
    // Draw cubic bezier curves
    for (size_t i = 1; i + 2 < control_points.size(); i += 3) {
        path.cubicTo(control_points[i].x, control_points[i].y,
                     control_points[i+1].x, control_points[i+1].y,
                     control_points[i+2].x, control_points[i+2].y);
    }
    
    painter_->drawPath(path);
}

void QtRenderer::draw_text(const xdot::Point& position, const std::string& text, const xdot::Pen& pen) {
    painter_->setPen(create_qpen(pen));
    QFont font = create_qfont(pen);
    painter_->setFont(font);
    
    QString qtext = QString::fromStdString(text);
    QFontMetrics metrics(font);
    QRect text_rect = metrics.boundingRect(qtext);
    
    // Calculate centered position
    // position is the center point, so we need to offset by half the text dimensions
    double centered_x = position.x - text_rect.width() / 2.0;
    double centered_y = position.y + text_rect.height() / 2.0 - metrics.descent();
    
    painter_->drawText(QPointF(centered_x, centered_y), qtext);
}

void QtRenderer::draw_image(const xdot::Point& position, double width, double height, const std::string& path) {
    QPixmap pixmap(QString::fromStdString(path));
    if (!pixmap.isNull()) {
        // Enable smooth transformation for high-quality image scaling
        painter_->setRenderHint(QPainter::SmoothPixmapTransform, true);
        QRectF rect(position.x, position.y, width, height);
        painter_->drawPixmap(rect, pixmap, pixmap.rect());
    }
}

QPen QtRenderer::create_qpen(const xdot::Pen& pen) {
    QPen qpen(create_qcolor(pen.color));
    qpen.setWidthF(pen.line_width);
    
    switch (pen.line_style) {
        case xdot::LineStyle::SOLID:
            qpen.setStyle(Qt::SolidLine);
            break;
        case xdot::LineStyle::DASHED:
            qpen.setStyle(Qt::DashLine);
            break;
        case xdot::LineStyle::DOTTED:
            qpen.setStyle(Qt::DotLine);
            break;
    }
    
    if (!pen.dash_pattern.empty()) {
        QVector<qreal> pattern;
        for (double dash : pen.dash_pattern) {
            pattern << dash;
        }
        qpen.setDashPattern(pattern);
    }
    
    return qpen;
}

QBrush QtRenderer::create_qbrush(const xdot::Pen& pen) {
    if (pen.filled) {
        return QBrush(create_qcolor(pen.fill_color));
    } else {
        return QBrush(Qt::NoBrush);
    }
}

QFont QtRenderer::create_qfont(const xdot::Pen& pen) {
    QFont font(QString::fromStdString(pen.font_family));
    font.setPointSizeF(pen.font_size);
    return font;
}

QColor QtRenderer::create_qcolor(const xdot::Color& color) {
    return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

// DotWidget implementation
DotWidget::DotWidget(QWidget* parent)
    : QGraphicsView(parent), scene_(nullptr), dragging_(false), zoom_factor_(1.0) {
    setup_scene();
    setDragMode(QGraphicsView::NoDrag);
    
    // Enable high-quality rendering for vector graphics
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // Optimize viewport updates for vector graphics
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
    // Disable caching to fix text rendering issues
    setCacheMode(QGraphicsView::CacheNone);
    
    // Optimize transformations for smooth zooming
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, false);
}

DotWidget::~DotWidget() {
    clear_scene_cache();
}

void DotWidget::set_graph(std::shared_ptr<xdot::GraphElement> graph) {
    graph_ = graph;
    update_scene();
}

void DotWidget::set_dot_code(const std::string& dot_code) {
    dot_code_ = dot_code;
    
    // Convert DOT to xdot format using Graphviz
    try {
        // Write DOT code to temporary file
        QString temp_file = QDir::temp().filePath("xdot_temp.dot");
        QFile file(temp_file);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // qDebug() << "Failed to create temporary file";
            return;
        }
        
        // Modify DOT code to account for coordinate system difference between Graphviz (Y up) and Qt (Y down)
        std::string modified_dot_code = dot_code;
        size_t rankdir_pos = modified_dot_code.find("rankdir");
        
        if (rankdir_pos != std::string::npos) {
            // Find the value after rankdir=
            size_t equals_pos = modified_dot_code.find('=', rankdir_pos);
            if (equals_pos != std::string::npos) {
                size_t value_start = equals_pos + 1;
                // Skip whitespace
                while (value_start < modified_dot_code.length() && std::isspace(modified_dot_code[value_start])) {
                    value_start++;
                }
                
                // Check if it's TB or BT and flip it
                if (modified_dot_code.substr(value_start, 2) == "TB") {
                    modified_dot_code.replace(value_start, 2, "BT");
                    // qDebug() << "Flipped rankdir from TB to BT";
                } else if (modified_dot_code.substr(value_start, 2) == "BT") {
                    modified_dot_code.replace(value_start, 2, "TB");
                    // qDebug() << "Flipped rankdir from BT to TB";
                } else {
                    // qDebug() << "DOT code contains rankdir but not TB/BT (probably LR/RL)";
                }
            }
        } else {
            // No rankdir specified, inject BT for top-down layout
            size_t brace_pos = modified_dot_code.find('{');
            if (brace_pos != std::string::npos) {
                modified_dot_code.insert(brace_pos + 1, "\n    rankdir=BT;");
                // qDebug() << "Injected rankdir=BT into DOT code";
            }
        }
        
        // Inject spacing parameters to prevent node overlapping
        size_t brace_pos = modified_dot_code.find('{');
        if (brace_pos != std::string::npos) {
            // Check if nodesep already exists
            if (modified_dot_code.find("nodesep") == std::string::npos) {
                modified_dot_code.insert(brace_pos + 1, "\n    nodesep=0.8;");
                // qDebug() << "Injected nodesep=0.8 into DOT code";
            }
            // Check if ranksep already exists
            if (modified_dot_code.find("ranksep") == std::string::npos) {
                modified_dot_code.insert(brace_pos + 1, "\n    ranksep=0.8;");
                // qDebug() << "Injected ranksep=0.8 into DOT code";
            }
        }
        
        QTextStream out(&file);
        out << QString::fromStdString(modified_dot_code);
        file.close();
        
        // Run dot to generate xdot format
        QProcess process;
        process.start("dot", QStringList() << "-Txdot" << temp_file);
        process.waitForFinished();
        
        if (process.exitCode() != 0) {
            // qDebug() << "Graphviz dot command failed:" << process.readAllStandardError();
            QFile::remove(temp_file);
            return;
        }
        
        QString xdot_output = process.readAllStandardOutput();
        QFile::remove(temp_file);
        
        // qDebug() << "Generated xdot format successfully";
        // qDebug() << "XDot output length:" << xdot_output.length();
        
        // Parse the xdot format using the full parser
        // qDebug() << "About to parse xdot format...";
        try {
            // First parse the DOT structure
            dot::DotParser dot_parser(xdot_output.toStdString());
            auto dot_graph = dot_parser.parse();
            // qDebug() << "Parsed DOT structure successfully";
            
            // Then parse the xdot drawing commands
            xdot::XDotParser xdot_parser(dot_graph);
            graph_ = xdot_parser.parse();
            // qDebug() << "Parsed xdot successfully";
            update_scene();
            // qDebug() << "Scene updated successfully.";
        } catch (const std::exception& e) {
            // qDebug() << "Error parsing xdot:" << e.what();
        }
        
    } catch (const std::exception& e) {
        // qDebug() << "Error converting DOT to xdot:" << e.what();
    }
}

void DotWidget::set_xdot_code(const std::string& xdot_code) {
    // Parse xdot code directly
    try {
        // qDebug() << "Starting xdot parsing...";
        xdot::XDotAttrParser parser(xdot_code);
        // qDebug() << "Created parser, about to parse...";
        auto shapes = parser.parse();
        // qDebug() << "Parsed" << shapes.size() << "shapes";
        
        // Create a simple graph element with background shapes
        graph_ = std::make_shared<xdot::GraphElement>();
        for (auto& shape : shapes) {
            graph_->add_background_shape(shape);
        }
        // qDebug() << "Added shapes to graph, about to update scene...";
        
        update_scene();
        // qDebug() << "Scene updated successfully.";
    } catch (const std::exception& e) {
        // qDebug() << "Error parsing xdot code:" << e.what();
    }
}

void DotWidget::zoom_to_fit() {
    if (!graph_) return;

    xdot::BoundingBox bbox = graph_->bounding_box();
    if (bbox.width() > 0 && bbox.height() > 0) {
        QRectF scene_rect(bbox.x1, bbox.y1, bbox.width(), bbox.height());
        fitInView(scene_rect, Qt::KeepAspectRatio);
        zoom_factor_ = transform().m11();
        emit zoom_changed(zoom_factor_);
    }
}

void DotWidget::zoom_in() {
    const double scale_factor = 1.25;
    scale(scale_factor, scale_factor);
    zoom_factor_ *= scale_factor;
    emit zoom_changed(zoom_factor_);
}

void DotWidget::zoom_out() {
    const double scale_factor = 0.8;
    scale(scale_factor, scale_factor);
    zoom_factor_ *= scale_factor;
    emit zoom_changed(zoom_factor_);
}

void DotWidget::reset_zoom() {
    resetTransform();
    zoom_factor_ = 1.0;
    emit zoom_changed(zoom_factor_);
}

void DotWidget::set_zoom_factor(double factor, bool emit_signal) {
    if (factor <= 0.0) return;

    resetTransform();
    scale(factor, factor);
    zoom_factor_ = factor;
    if (emit_signal) {
        emit zoom_changed(zoom_factor_);
    }
}

void DotWidget::reload() {
    if (!dot_code_.empty()) {
        set_dot_code(dot_code_);
    } else {
        update_scene();
    }
}

void DotWidget::find_node(const std::string& node_id) {
    if (!graph_) return;
    
    graph_->clear_highlights();
    graph_->highlight_node(node_id);
    
    // Find the node and center on it
    for (const auto& node : graph_->nodes()) {
        if (node->id() == node_id) {
            xdot::BoundingBox bbox = node->bounding_box();
            QRectF rect(bbox.x1, bbox.y1, bbox.width(), bbox.height());
            centerOn(rect.center());
            break;
        }
    }
    
    update_scene();
}

void DotWidget::wheelEvent(QWheelEvent* event) {
    // Zoom with scroll wheel (no Ctrl required)
    const double scale_factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
    scale(scale_factor, scale_factor);
    zoom_factor_ *= scale_factor;
    emit zoom_changed(zoom_factor_);
    event->accept();
}

void DotWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        xdot::Point graph_pos = qt_to_graph_coords(event->pos());
        
        if (graph_) {
            auto node = graph_->find_node_at(graph_pos);
            if (node) {
                emit node_clicked(QString::fromStdString(node->id()), 
                                QString::fromStdString(node->url()));
                event->accept();
                return;
            }
            
            auto edge = graph_->find_edge_at(graph_pos);
            if (edge) {
                emit edge_clicked(QString::fromStdString(edge->source()),
                                QString::fromStdString(edge->target()),
                                QString::fromStdString(edge->url()));
                event->accept();
                return;
            }
        }
        
        emit background_clicked();
        
        // Start panning
        dragging_ = true;
        last_pan_point_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void DotWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        QPoint delta = event->pos() - last_pan_point_;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        last_pan_point_ = event->pos();
        event->accept();
    } else {
        // Highlight elements under cursor
        highlight_element_at_position(event->pos());
        QGraphicsView::mouseMoveEvent(event);
    }
}

void DotWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void DotWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoom_in();
            break;
        case Qt::Key_Minus:
            zoom_out();
            break;
        case Qt::Key_0:
            reset_zoom();
            break;
        case Qt::Key_F:
            zoom_to_fit();
            break;
        case Qt::Key_R:
            reload();
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void DotWidget::paintEvent(QPaintEvent* event) {
    QGraphicsView::paintEvent(event);
}

void DotWidget::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
}

void DotWidget::update_scene() {
    if (scene_) {
        // Clear the cache since scene_->clear() will remove all items
        clear_scene_cache();
        render_graph();
    }
}

void DotWidget::setup_scene() {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    scene_->setBackgroundBrush(QBrush(Qt::white));
}

void DotWidget::render_graph() {
    // Use optimized rendering with caching
    render_graph_optimized();
}

void DotWidget::render_graph_optimized() {
    if (!graph_) {
        clear_scene_cache();
        return;
    }

    xdot::BoundingBox bbox = graph_->bounding_box();
    if (bbox.width() <= 0 || bbox.height() <= 0) {
        clear_scene_cache();
        return;
    }

    // Update background shapes
    update_background_items(graph_->background_shapes());

    // Track which cached items are still valid
    std::unordered_set<std::string> current_nodes;
    std::unordered_set<std::string> current_edges;

    // Update/create node items
    for (const auto& node : graph_->nodes()) {
        current_nodes.insert(node->id());
        update_node_item(node->id(), node);
    }

    // Update/create edge items
    for (const auto& edge : graph_->edges()) {
        std::string edge_key = make_edge_key(edge->source(), edge->target());
        current_edges.insert(edge_key);
        update_edge_item(edge_key, edge);
    }

    // Remove cached items that are no longer present
    auto node_it = cached_node_items_.begin();
    while (node_it != cached_node_items_.end()) {
        if (current_nodes.find(node_it->first) == current_nodes.end()) {
            scene_->removeItem(node_it->second);
            delete node_it->second;
            node_it = cached_node_items_.erase(node_it);
        } else {
            ++node_it;
        }
    }

    auto edge_it = cached_edge_items_.begin();
    while (edge_it != cached_edge_items_.end()) {
        if (current_edges.find(edge_it->first) == current_edges.end()) {
            scene_->removeItem(edge_it->second);
            delete edge_it->second;
            edge_it = cached_edge_items_.erase(edge_it);
        } else {
            ++edge_it;
        }
    }

    // Set scene rectangle with some padding
    scene_->setSceneRect(bbox.x1 - 10, bbox.y1 - 10, bbox.width() + 20, bbox.height() + 20);
}

void DotWidget::render_shapes(const std::vector<std::shared_ptr<xdot::Shape>>& shapes, QPainter* painter) {
    QtRenderer renderer(painter);
    for (const auto& shape : shapes) {
        shape->draw(&renderer);
    }
}

std::shared_ptr<xdot::GraphNode> DotWidget::find_node_at_position(const QPoint& pos) {
    if (!graph_) return nullptr;
    
    xdot::Point graph_pos = qt_to_graph_coords(pos);
    return graph_->find_node_at(graph_pos);
}

std::shared_ptr<xdot::GraphEdge> DotWidget::find_edge_at_position(const QPoint& pos) {
    if (!graph_) return nullptr;
    
    xdot::Point graph_pos = qt_to_graph_coords(pos);
    return graph_->find_edge_at(graph_pos);
}

void DotWidget::highlight_element_at_position(const QPoint& pos) {
    if (!graph_) return;
    
    auto node = find_node_at_position(pos);
    auto edge = find_edge_at_position(pos);
    
    bool changed = false;
    
    if (node != highlighted_node_) {
        highlighted_node_ = node;
        changed = true;
    }
    
    if (edge != highlighted_edge_) {
        highlighted_edge_ = edge;
        changed = true;
    }
    
    if (changed) {
        setCursor(node || edge ? Qt::PointingHandCursor : Qt::ArrowCursor);
        // Could update visual highlighting here
    }
}

void DotWidget::clear_highlights() {
    if (graph_) {
        graph_->clear_highlights();
        update_scene();
    }
}

xdot::Point DotWidget::qt_to_graph_coords(const QPoint& qt_point) {
    QPointF scene_point = mapToScene(qt_point);
    return xdot::Point(scene_point.x(), scene_point.y());
}

QPoint DotWidget::graph_to_qt_coords(const xdot::Point& graph_point) {
    QPointF scene_point(graph_point.x, graph_point.y);
    QPoint viewport_point = mapFromScene(scene_point);
    return viewport_point;
}

void DotWidget::clear_scene_cache() {
    // Remove all cached node items
    for (auto& pair : cached_node_items_) {
        scene_->removeItem(pair.second);
        delete pair.second;
    }
    cached_node_items_.clear();

    // Remove all cached edge items
    for (auto& pair : cached_edge_items_) {
        scene_->removeItem(pair.second);
        delete pair.second;
    }
    cached_edge_items_.clear();

    // Remove all cached background items
    for (auto& pair : cached_background_items_) {
        scene_->removeItem(pair.second);
        delete pair.second;
    }
    cached_background_items_.clear();
}

void DotWidget::update_node_item(const std::string& node_id, std::shared_ptr<xdot::GraphNode> node) {
    auto it = cached_node_items_.find(node_id);
    if (it != cached_node_items_.end()) {
        // Update existing item
        // For now, we'll recreate the item. A more sophisticated approach
        // would compare the node's shapes and only update if they changed.
        scene_->removeItem(it->second);
        delete it->second;
    }

    // Create new item
    auto node_item = new GraphicsNodeItem(node);
    scene_->addItem(node_item);
    cached_node_items_[node_id] = node_item;
}

void DotWidget::update_edge_item(const std::string& edge_key, std::shared_ptr<xdot::GraphEdge> edge) {
    auto it = cached_edge_items_.find(edge_key);
    if (it != cached_edge_items_.end()) {
        // Update existing item
        scene_->removeItem(it->second);
        delete it->second;
    }

    // Create new item
    auto edge_item = new GraphicsEdgeItem(edge);
    scene_->addItem(edge_item);
    cached_edge_items_[edge_key] = edge_item;
}

void DotWidget::update_background_items(const std::vector<std::shared_ptr<xdot::Shape>>& shapes) {
    // For simplicity, clear all background items and recreate
    // A more sophisticated approach would compare shapes
    for (auto& pair : cached_background_items_) {
        scene_->removeItem(pair.second);
        delete pair.second;
    }
    cached_background_items_.clear();

    // Add new background shapes
    for (const auto& shape : shapes) {
        auto shape_item = new GraphicsShapeItem(shape);
        scene_->addItem(shape_item);
        cached_background_items_[shape.get()] = shape_item;
    }
}

std::string DotWidget::make_edge_key(const std::string& source, const std::string& target) {
    return source + "->" + target;
}

} // namespace ui
} // namespace xdot_cpp

