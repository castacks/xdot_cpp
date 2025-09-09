#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace xdot_cpp {
namespace dot {

enum class TokenType {
    EOF_TOKEN = -1,
    SKIP = -2,
    
    ID = 0,
    STR_ID = 1,
    HTML_ID = 2,
    EDGE_OP = 3,
    
    LSQUARE = 4,
    RSQUARE = 5,
    LCURLY = 6,
    RCURLY = 7,
    COMMA = 8,
    COLON = 9,
    SEMI = 10,
    EQUAL = 11,
    PLUS = 12,
    
    STRICT = 13,
    GRAPH = 14,
    DIGRAPH = 15,
    NODE = 16,
    EDGE = 17,
    SUBGRAPH = 18
};

struct Token {
    TokenType type;
    std::string text;
    size_t line;
    size_t column;
    
    Token(TokenType t = TokenType::EOF_TOKEN, const std::string& txt = "", size_t l = 0, size_t c = 0)
        : type(t), text(txt), line(l), column(c) {}
};

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, size_t line = 0, size_t column = 0);
    
    size_t line() const { return line_; }
    size_t column() const { return column_; }
    
private:
    size_t line_;
    size_t column_;
};

class DotLexer {
public:
    explicit DotLexer(const std::string& text);
    
    Token next_token();
    Token peek_token();
    bool has_more() const;
    
    size_t line() const { return line_; }
    size_t column() const { return column_; }
    
private:
    std::string text_;
    size_t pos_;
    size_t line_;
    size_t column_;
    Token peeked_token_;
    bool has_peeked_;
    
    char current_char() const;
    char peek_char(size_t offset = 1) const;
    void advance();
    void skip_whitespace();
    void skip_comment();
    
    Token read_string();
    Token read_html_string();
    Token read_identifier();
    Token read_number();
    
    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alnum(char c) const;
    bool is_whitespace(char c) const;
};

} // namespace dot
} // namespace xdot_cpp