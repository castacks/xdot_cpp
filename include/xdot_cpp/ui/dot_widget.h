#pragma once

#include "../xdot/graph.h"
#include "../xdot/elements.h"
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <memory>
#include <unordered_map>

namespace xdot_cpp {
namespace ui {

class QtRenderer : public xdot::Renderer {
public:
    explicit QtRenderer(QPainter* painter);
    
    void draw_ellipse(const xdot::Point& center, double width, double height, const xdot::Pen& pen) override;
    void draw_polygon(const std::vector<xdot::Point>& points, const xdot::Pen& pen) override;
    void draw_polyline(const std::vector<xdot::Point>& points, const xdot::Pen& pen) override;
    void draw_bezier(const std::vector<xdot::Point>& control_points, const xdot::Pen& pen) override;
    void draw_text(const xdot::Point& position, const std::string& text, const xdot::Pen& pen) override;
    void draw_image(const xdot::Point& position, double width, double height, const std::string& path) override;
    
private:
    QPainter* painter_;
    
    QPen create_qpen(const xdot::Pen& pen);
    QBrush create_qbrush(const xdot::Pen& pen);
    QFont create_qfont(const xdot::Pen& pen);
    QColor create_qcolor(const xdot::Color& color);
};

class DotWidget : public QGraphicsView {
    Q_OBJECT
    
public:
    explicit DotWidget(QWidget* parent = nullptr);
    ~DotWidget();
    
    void set_graph(std::shared_ptr<xdot::GraphElement> graph);
    void set_dot_code(const std::string& dot_code);
    void set_xdot_code(const std::string& xdot_code);
    
    void zoom_to_fit();
    void zoom_in();
    void zoom_out();
    void reset_zoom();

    double get_zoom_factor() const { return zoom_factor_; }
    void set_zoom_factor(double factor, bool emit_signal = true);
    
    void reload();
    void find_node(const std::string& node_id);
    
    std::shared_ptr<xdot::GraphElement> graph() const { return graph_; }
    
signals:
    void node_clicked(const QString& node_id, const QString& url);
    void edge_clicked(const QString& source, const QString& target, const QString& url);
    void background_clicked();
    void zoom_changed(double zoom_factor);
    
protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    void update_scene();
    
private:
    std::shared_ptr<xdot::GraphElement> graph_;
    std::string dot_code_;
    QGraphicsScene* scene_;
    
    bool dragging_;
    QPoint last_pan_point_;
    double zoom_factor_;
    
    std::shared_ptr<xdot::GraphNode> highlighted_node_;
    std::shared_ptr<xdot::GraphEdge> highlighted_edge_;

    // Scene item caching
    std::unordered_map<std::string, class GraphicsNodeItem*> cached_node_items_;
    std::unordered_map<std::string, class GraphicsEdgeItem*> cached_edge_items_;
    std::unordered_map<void*, class GraphicsShapeItem*> cached_background_items_;

    void setup_scene();
    void render_graph();
    void render_graph_optimized();
    void render_shapes(const std::vector<std::shared_ptr<xdot::Shape>>& shapes, QPainter* painter);
    
    std::shared_ptr<xdot::GraphNode> find_node_at_position(const QPoint& pos);
    std::shared_ptr<xdot::GraphEdge> find_edge_at_position(const QPoint& pos);
    
    void highlight_element_at_position(const QPoint& pos);
    void clear_highlights();

    // Cache management
    void clear_scene_cache();
    void update_node_item(const std::string& node_id, std::shared_ptr<xdot::GraphNode> node);
    void update_edge_item(const std::string& edge_key, std::shared_ptr<xdot::GraphEdge> edge);
    void update_background_items(const std::vector<std::shared_ptr<xdot::Shape>>& shapes);
    std::string make_edge_key(const std::string& source, const std::string& target);

    xdot::Point qt_to_graph_coords(const QPoint& qt_point);
    QPoint graph_to_qt_coords(const xdot::Point& graph_point);
};

} // namespace ui
} // namespace xdot_cpp