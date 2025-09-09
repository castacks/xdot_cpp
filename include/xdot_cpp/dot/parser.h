#pragma once

#include "lexer.h"
#include <memory>
#include <map>
#include <vector>

namespace xdot_cpp {
namespace dot {

struct Attribute {
    std::string name;
    std::string value;
    
    Attribute(const std::string& n = "", const std::string& v = "") : name(n), value(v) {}
};

using AttributeList = std::vector<Attribute>;

struct Node {
    std::string id;
    AttributeList attributes;
    
    explicit Node(const std::string& node_id = "") : id(node_id) {}
};

struct Edge {
    std::string source;
    std::string target;
    AttributeList attributes;
    
    Edge(const std::string& src = "", const std::string& tgt = "") : source(src), target(tgt) {}
};

struct Subgraph {
    std::string id;
    AttributeList attributes;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Edge>> edges;
    std::vector<std::shared_ptr<Subgraph>> subgraphs;
    
    explicit Subgraph(const std::string& subgraph_id = "") : id(subgraph_id) {}
};

struct Graph {
    enum Type { GRAPH, DIGRAPH };
    
    Type type;
    bool strict;
    std::string id;
    AttributeList attributes;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Edge>> edges;
    std::vector<std::shared_ptr<Subgraph>> subgraphs;
    
    Graph() : type(DIGRAPH), strict(false) {}
};

class DotParser {
public:
    explicit DotParser(const std::string& text);
    
    std::shared_ptr<Graph> parse();
    
private:
    DotLexer lexer_;
    Token current_token_;
    
    void consume(TokenType expected_type);
    void advance();
    
    std::shared_ptr<Graph> parse_graph();
    std::shared_ptr<Subgraph> parse_subgraph();
    std::shared_ptr<Node> parse_node();
    std::shared_ptr<Edge> parse_edge(const std::string& source_id);
    AttributeList parse_attributes();
    std::string parse_id();
    
    bool is_node_statement();
    bool is_edge_statement();
    bool is_attribute_statement();
};

} // namespace dot
} // namespace xdot_cpp