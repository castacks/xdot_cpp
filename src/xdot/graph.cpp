#include "xdot_cpp/xdot/graph.h"
#include <algorithm>

namespace xdot_cpp {
namespace xdot {

// GraphNode implementation
GraphNode::GraphNode(const std::string& id, const std::vector<std::shared_ptr<Shape>>& shapes)
    : id_(id), shapes_(shapes), highlighted_(false) {}

BoundingBox GraphNode::bounding_box() const {
    if (shapes_.empty()) {
        return BoundingBox();
    }
    
    BoundingBox bbox = shapes_[0]->bounding_box();
    for (size_t i = 1; i < shapes_.size(); i++) {
        BoundingBox shape_bbox = shapes_[i]->bounding_box();
        bbox.x1 = std::min(bbox.x1, shape_bbox.x1);
        bbox.y1 = std::min(bbox.y1, shape_bbox.y1);
        bbox.x2 = std::max(bbox.x2, shape_bbox.x2);
        bbox.y2 = std::max(bbox.y2, shape_bbox.y2);
    }
    
    return bbox;
}

bool GraphNode::contains_point(const Point& p) const {
    for (const auto& shape : shapes_) {
        if (shape->contains_point(p)) {
            return true;
        }
    }
    return false;
}

// GraphEdge implementation
GraphEdge::GraphEdge(const std::string& source, const std::string& target, 
                     const std::vector<std::shared_ptr<Shape>>& shapes)
    : source_(source), target_(target), shapes_(shapes), highlighted_(false) {}

BoundingBox GraphEdge::bounding_box() const {
    if (shapes_.empty()) {
        return BoundingBox();
    }
    
    BoundingBox bbox = shapes_[0]->bounding_box();
    for (size_t i = 1; i < shapes_.size(); i++) {
        BoundingBox shape_bbox = shapes_[i]->bounding_box();
        bbox.x1 = std::min(bbox.x1, shape_bbox.x1);
        bbox.y1 = std::min(bbox.y1, shape_bbox.y1);
        bbox.x2 = std::max(bbox.x2, shape_bbox.x2);
        bbox.y2 = std::max(bbox.y2, shape_bbox.y2);
    }
    
    return bbox;
}

bool GraphEdge::contains_point(const Point& p) const {
    for (const auto& shape : shapes_) {
        if (shape->contains_point(p)) {
            return true;
        }
    }
    return false;
}

// GraphElement implementation
GraphElement::GraphElement() {}

void GraphElement::add_node(std::shared_ptr<GraphNode> node) {
    nodes_.push_back(node);
    node_map_[node->id()] = node;
}

void GraphElement::add_edge(std::shared_ptr<GraphEdge> edge) {
    edges_.push_back(edge);
}

void GraphElement::add_background_shape(std::shared_ptr<Shape> shape) {
    background_shapes_.push_back(shape);
}

BoundingBox GraphElement::bounding_box() const {
    if (nodes_.empty() && edges_.empty() && background_shapes_.empty()) {
        return BoundingBox();
    }
    
    BoundingBox bbox;
    bool first = true;
    
    // Include nodes
    for (const auto& node : nodes_) {
        BoundingBox node_bbox = node->bounding_box();
        if (first) {
            bbox = node_bbox;
            first = false;
        } else {
            bbox.x1 = std::min(bbox.x1, node_bbox.x1);
            bbox.y1 = std::min(bbox.y1, node_bbox.y1);
            bbox.x2 = std::max(bbox.x2, node_bbox.x2);
            bbox.y2 = std::max(bbox.y2, node_bbox.y2);
        }
    }
    
    // Include edges
    for (const auto& edge : edges_) {
        BoundingBox edge_bbox = edge->bounding_box();
        if (first) {
            bbox = edge_bbox;
            first = false;
        } else {
            bbox.x1 = std::min(bbox.x1, edge_bbox.x1);
            bbox.y1 = std::min(bbox.y1, edge_bbox.y1);
            bbox.x2 = std::max(bbox.x2, edge_bbox.x2);
            bbox.y2 = std::max(bbox.y2, edge_bbox.y2);
        }
    }
    
    // Include background shapes
    for (const auto& shape : background_shapes_) {
        BoundingBox shape_bbox = shape->bounding_box();
        if (first) {
            bbox = shape_bbox;
            first = false;
        } else {
            bbox.x1 = std::min(bbox.x1, shape_bbox.x1);
            bbox.y1 = std::min(bbox.y1, shape_bbox.y1);
            bbox.x2 = std::max(bbox.x2, shape_bbox.x2);
            bbox.y2 = std::max(bbox.y2, shape_bbox.y2);
        }
    }
    
    return bbox;
}

std::shared_ptr<GraphNode> GraphElement::find_node_at(const Point& p) const {
    // Search in reverse order to find topmost node
    for (auto it = nodes_.rbegin(); it != nodes_.rend(); ++it) {
        if ((*it)->contains_point(p)) {
            return *it;
        }
    }
    return nullptr;
}

std::shared_ptr<GraphEdge> GraphElement::find_edge_at(const Point& p) const {
    // Search in reverse order to find topmost edge
    for (auto it = edges_.rbegin(); it != edges_.rend(); ++it) {
        if ((*it)->contains_point(p)) {
            return *it;
        }
    }
    return nullptr;
}

void GraphElement::clear_highlights() {
    for (auto& node : nodes_) {
        node->set_highlighted(false);
    }
    for (auto& edge : edges_) {
        edge->set_highlighted(false);
    }
}

void GraphElement::highlight_node(const std::string& node_id) {
    auto it = node_map_.find(node_id);
    if (it != node_map_.end()) {
        it->second->set_highlighted(true);
    }
}

void GraphElement::highlight_edge(const std::string& source, const std::string& target) {
    for (auto& edge : edges_) {
        if (edge->source() == source && edge->target() == target) {
            edge->set_highlighted(true);
            break;
        }
    }
}

} // namespace xdot
} // namespace xdot_cpp