#include "xdot_cpp/dot/parser.h"
#include <sstream>

namespace xdot_cpp {
namespace dot {

DotParser::DotParser(const std::string& text) : lexer_(text) {
    advance(); // Initialize current_token_
}

std::shared_ptr<Graph> DotParser::parse() {
    return parse_graph();
}

void DotParser::consume(TokenType expected_type) {
    if (current_token_.type != expected_type) {
        std::ostringstream oss;
        oss << "Expected token type " << static_cast<int>(expected_type) 
            << " but got " << static_cast<int>(current_token_.type);
        throw ParseError(oss.str(), current_token_.line, current_token_.column);
    }
    advance();
}

void DotParser::advance() {
    current_token_ = lexer_.next_token();
}

std::shared_ptr<Graph> DotParser::parse_graph() {
    auto graph = std::make_shared<Graph>();
    
    // Parse optional 'strict' keyword
    if (current_token_.type == TokenType::STRICT) {
        graph->strict = true;
        advance();
    }
    
    // Parse graph type
    if (current_token_.type == TokenType::GRAPH) {
        graph->type = Graph::GRAPH;
        advance();
    } else if (current_token_.type == TokenType::DIGRAPH) {
        graph->type = Graph::DIGRAPH;
        advance();
    } else {
        throw ParseError("Expected 'graph' or 'digraph'", current_token_.line, current_token_.column);
    }
    
    // Parse optional graph ID
    if (current_token_.type == TokenType::ID || current_token_.type == TokenType::STR_ID) {
        graph->id = parse_id();
    }
    
    consume(TokenType::LCURLY);
    
    // Parse statements
    while (current_token_.type != TokenType::RCURLY && current_token_.type != TokenType::EOF_TOKEN) {
        if (current_token_.type == TokenType::NODE) {
            // Node attribute statement
            advance();
            graph->attributes = parse_attributes();
        } else if (current_token_.type == TokenType::EDGE) {
            // Edge attribute statement
            advance();
            graph->attributes = parse_attributes();
        } else if (current_token_.type == TokenType::GRAPH) {
            // Graph attribute statement
            advance();
            auto attrs = parse_attributes();
            graph->attributes.insert(graph->attributes.end(), attrs.begin(), attrs.end());
        } else if (current_token_.type == TokenType::SUBGRAPH) {
            // Subgraph
            auto subgraph = parse_subgraph();
            graph->subgraphs.push_back(subgraph);
        } else if (current_token_.type == TokenType::ID || current_token_.type == TokenType::STR_ID) {
            // Node, edge, or attribute assignment statement
            std::string id = parse_id();
            
            if (current_token_.type == TokenType::EDGE_OP) {
                // Edge statement
                auto edge = parse_edge(id);
                graph->edges.push_back(edge);
            } else if (current_token_.type == TokenType::EQUAL) {
                // Graph attribute assignment: ID = ID
                advance(); // consume '='
                std::string value = parse_id();
                graph->attributes.push_back({id, value});
            } else {
                // Node statement
                auto node = std::make_shared<Node>(id);
                if (current_token_.type == TokenType::LSQUARE) {
                    node->attributes = parse_attributes();
                }
                graph->nodes.push_back(node);
            }
        } else {
            std::string token_str = current_token_.text.empty() ? 
                std::to_string(static_cast<int>(current_token_.type)) : current_token_.text;
            throw ParseError("Unexpected token in graph body: '" + token_str + "' at line " + 
                           std::to_string(current_token_.line), current_token_.line, current_token_.column);
        }
        
        // Optional semicolon
        if (current_token_.type == TokenType::SEMI) {
            advance();
        }
    }
    
    consume(TokenType::RCURLY);
    return graph;
}

std::shared_ptr<Subgraph> DotParser::parse_subgraph() {
    consume(TokenType::SUBGRAPH);
    
    auto subgraph = std::make_shared<Subgraph>();
    
    // Parse optional subgraph ID
    if (current_token_.type == TokenType::ID || current_token_.type == TokenType::STR_ID) {
        subgraph->id = parse_id();
    }
    
    consume(TokenType::LCURLY);
    
    // Parse subgraph body (similar to graph body)
    while (current_token_.type != TokenType::RCURLY && current_token_.type != TokenType::EOF_TOKEN) {
        if (current_token_.type == TokenType::SUBGRAPH) {
            auto nested_subgraph = parse_subgraph();
            subgraph->subgraphs.push_back(nested_subgraph);
        } else if (current_token_.type == TokenType::ID || current_token_.type == TokenType::STR_ID) {
            std::string id = parse_id();
            
            if (current_token_.type == TokenType::EDGE_OP) {
                auto edge = parse_edge(id);
                subgraph->edges.push_back(edge);
            } else {
                auto node = std::make_shared<Node>(id);
                if (current_token_.type == TokenType::LSQUARE) {
                    node->attributes = parse_attributes();
                }
                subgraph->nodes.push_back(node);
            }
        } else {
            throw ParseError("Unexpected token in subgraph body", current_token_.line, current_token_.column);
        }
        
        if (current_token_.type == TokenType::SEMI) {
            advance();
        }
    }
    
    consume(TokenType::RCURLY);
    return subgraph;
}

std::shared_ptr<Node> DotParser::parse_node() {
    std::string id = parse_id();
    auto node = std::make_shared<Node>(id);
    
    if (current_token_.type == TokenType::LSQUARE) {
        node->attributes = parse_attributes();
    }
    
    return node;
}

std::shared_ptr<Edge> DotParser::parse_edge(const std::string& source_id) {
    consume(TokenType::EDGE_OP);
    
    std::string target_id = parse_id();
    auto edge = std::make_shared<Edge>(source_id, target_id);
    
    if (current_token_.type == TokenType::LSQUARE) {
        edge->attributes = parse_attributes();
    }
    
    return edge;
}

AttributeList DotParser::parse_attributes() {
    AttributeList attributes;
    
    consume(TokenType::LSQUARE);
    
    while (current_token_.type != TokenType::RSQUARE && current_token_.type != TokenType::EOF_TOKEN) {
        std::string name = parse_id();
        
        if (current_token_.type == TokenType::EQUAL) {
            advance();
            std::string value = parse_id();
            attributes.emplace_back(name, value);
        } else {
            attributes.emplace_back(name, "");
        }
        
        if (current_token_.type == TokenType::COMMA || current_token_.type == TokenType::SEMI) {
            advance();
        }
    }
    
    consume(TokenType::RSQUARE);
    return attributes;
}

std::string DotParser::parse_id() {
    std::string id;
    
    if (current_token_.type == TokenType::ID || 
        current_token_.type == TokenType::STR_ID || 
        current_token_.type == TokenType::HTML_ID) {
        id = current_token_.text;
        advance();
    } else {
        throw ParseError("Expected identifier", current_token_.line, current_token_.column);
    }
    
    return id;
}

bool DotParser::is_node_statement() {
    // Look ahead to see if this is a node statement
    Token next = lexer_.peek_token();
    return next.type == TokenType::LSQUARE || next.type == TokenType::SEMI || next.type == TokenType::RCURLY;
}

bool DotParser::is_edge_statement() {
    Token next = lexer_.peek_token();
    return next.type == TokenType::EDGE_OP;
}

bool DotParser::is_attribute_statement() {
    return current_token_.type == TokenType::NODE || 
           current_token_.type == TokenType::EDGE || 
           current_token_.type == TokenType::GRAPH;
}

} // namespace dot
} // namespace xdot_cpp