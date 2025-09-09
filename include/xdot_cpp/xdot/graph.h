#pragma once

#include "elements.h"
#include "../dot/parser.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

namespace xdot_cpp {
namespace xdot {

class GraphNode {
public:
    GraphNode(const std::string& id, const std::vector<std::shared_ptr<Shape>>& shapes);
    
    const std::string& id() const { return id_; }
    const std::vector<std::shared_ptr<Shape>>& shapes() const { return shapes_; }
    BoundingBox bounding_box() const;
    bool contains_point(const Point& p) const;
    
    void set_url(const std::string& url) { url_ = url; }
    const std::string& url() const { return url_; }
    
    void set_highlighted(bool highlighted) { highlighted_ = highlighted; }
    bool is_highlighted() const { return highlighted_; }
    
private:
    std::string id_;
    std::vector<std::shared_ptr<Shape>> shapes_;
    std::string url_;
    bool highlighted_;
};

class GraphEdge {
public:
    GraphEdge(const std::string& source, const std::string& target, 
              const std::vector<std::shared_ptr<Shape>>& shapes);
    
    const std::string& source() const { return source_; }
    const std::string& target() const { return target_; }
    const std::vector<std::shared_ptr<Shape>>& shapes() const { return shapes_; }
    BoundingBox bounding_box() const;
    bool contains_point(const Point& p) const;
    
    void set_url(const std::string& url) { url_ = url; }
    const std::string& url() const { return url_; }
    
    void set_highlighted(bool highlighted) { highlighted_ = highlighted; }
    bool is_highlighted() const { return highlighted_; }
    
private:
    std::string source_;
    std::string target_;
    std::vector<std::shared_ptr<Shape>> shapes_;
    std::string url_;
    bool highlighted_;
};

class GraphElement {
public:
    GraphElement();
    
    void add_node(std::shared_ptr<GraphNode> node);
    void add_edge(std::shared_ptr<GraphEdge> edge);
    void add_background_shape(std::shared_ptr<Shape> shape);
    
    const std::vector<std::shared_ptr<GraphNode>>& nodes() const { return nodes_; }
    const std::vector<std::shared_ptr<GraphEdge>>& edges() const { return edges_; }
    const std::vector<std::shared_ptr<Shape>>& background_shapes() const { return background_shapes_; }
    
    BoundingBox bounding_box() const;
    
    std::shared_ptr<GraphNode> find_node_at(const Point& p) const;
    std::shared_ptr<GraphEdge> find_edge_at(const Point& p) const;
    
    void clear_highlights();
    void highlight_node(const std::string& node_id);
    void highlight_edge(const std::string& source, const std::string& target);
    
private:
    std::vector<std::shared_ptr<GraphNode>> nodes_;
    std::vector<std::shared_ptr<GraphEdge>> edges_;
    std::vector<std::shared_ptr<Shape>> background_shapes_;
    std::map<std::string, std::shared_ptr<GraphNode>> node_map_;
};

} // namespace xdot
} // namespace xdot_cpp