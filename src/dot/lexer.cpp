#include "xdot_cpp/dot/lexer.h"
#include <cctype>
#include <unordered_map>

namespace xdot_cpp {
namespace dot {

ParseError::ParseError(const std::string& message, size_t line, size_t column)
    : std::runtime_error(message), line_(line), column_(column) {}

DotLexer::DotLexer(const std::string& text)
    : text_(text), pos_(0), line_(1), column_(1), has_peeked_(false) {}

Token DotLexer::next_token() {
    if (has_peeked_) {
        has_peeked_ = false;
        return peeked_token_;
    }
    
    skip_whitespace();
    
    if (pos_ >= text_.length()) {
        return Token(TokenType::EOF_TOKEN, "", line_, column_);
    }
    
    char c = current_char();
    size_t token_line = line_;
    size_t token_column = column_;
    
    // Single character tokens
    switch (c) {
        case '[':
            advance();
            return Token(TokenType::LSQUARE, "[", token_line, token_column);
        case ']':
            advance();
            return Token(TokenType::RSQUARE, "]", token_line, token_column);
        case '{':
            advance();
            return Token(TokenType::LCURLY, "{", token_line, token_column);
        case '}':
            advance();
            return Token(TokenType::RCURLY, "}", token_line, token_column);
        case ',':
            advance();
            return Token(TokenType::COMMA, ",", token_line, token_column);
        case ':':
            advance();
            return Token(TokenType::COLON, ":", token_line, token_column);
        case ';':
            advance();
            return Token(TokenType::SEMI, ";", token_line, token_column);
        case '=':
            advance();
            return Token(TokenType::EQUAL, "=", token_line, token_column);
        case '+':
            advance();
            return Token(TokenType::PLUS, "+", token_line, token_column);
    }
    
    // Edge operators
    if (c == '-') {
        if (peek_char() == '>') {
            advance();
            advance();
            return Token(TokenType::EDGE_OP, "->", token_line, token_column);
        } else if (peek_char() == '-') {
            advance();
            advance();
            return Token(TokenType::EDGE_OP, "--", token_line, token_column);
        }
    }
    
    // String literals
    if (c == '"') {
        return read_string();
    }
    
    // HTML strings
    if (c == '<') {
        return read_html_string();
    }
    
    // Numbers
    if (is_digit(c) || (c == '.' && is_digit(peek_char()))) {
        return read_number();
    }
    
    // Identifiers and keywords
    if (is_alpha(c) || c == '_') {
        return read_identifier();
    }
    
    // Unknown character
    throw ParseError("Unexpected character: " + std::string(1, c), line_, column_);
}

Token DotLexer::peek_token() {
    if (!has_peeked_) {
        peeked_token_ = next_token();
        has_peeked_ = true;
    }
    return peeked_token_;
}

bool DotLexer::has_more() const {
    return pos_ < text_.length();
}

char DotLexer::current_char() const {
    if (pos_ >= text_.length()) {
        return '\0';
    }
    return text_[pos_];
}

char DotLexer::peek_char(size_t offset) const {
    size_t peek_pos = pos_ + offset;
    if (peek_pos >= text_.length()) {
        return '\0';
    }
    return text_[peek_pos];
}

void DotLexer::advance() {
    if (pos_ < text_.length()) {
        if (text_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

void DotLexer::skip_whitespace() {
    while (pos_ < text_.length() && is_whitespace(current_char())) {
        if (current_char() == '/' && peek_char() == '/') {
            skip_comment();
        } else if (current_char() == '/' && peek_char() == '*') {
            // Skip C-style comments
            advance(); // skip '/'
            advance(); // skip '*'
            while (pos_ < text_.length() - 1) {
                if (current_char() == '*' && peek_char() == '/') {
                    advance(); // skip '*'
                    advance(); // skip '/'
                    break;
                }
                advance();
            }
        } else if (current_char() == '#') {
            skip_comment();
        } else {
            advance();
        }
    }
}

void DotLexer::skip_comment() {
    while (pos_ < text_.length() && current_char() != '\n') {
        advance();
    }
}

Token DotLexer::read_string() {
    size_t token_line = line_;
    size_t token_column = column_;
    std::string value;
    
    advance(); // skip opening quote
    
    while (pos_ < text_.length() && current_char() != '"') {
        if (current_char() == '\\' && peek_char() != '\0') {
            advance(); // skip backslash
            char escaped = current_char();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default: value += escaped; break;
            }
            advance();
        } else {
            value += current_char();
            advance();
        }
    }
    
    if (pos_ >= text_.length()) {
        throw ParseError("Unterminated string literal", token_line, token_column);
    }
    
    advance(); // skip closing quote
    return Token(TokenType::STR_ID, value, token_line, token_column);
}

Token DotLexer::read_html_string() {
    size_t token_line = line_;
    size_t token_column = column_;
    std::string value;
    int depth = 0;
    
    do {
        if (current_char() == '<') {
            depth++;
        } else if (current_char() == '>') {
            depth--;
        }
        value += current_char();
        advance();
    } while (pos_ < text_.length() && depth > 0);
    
    if (depth > 0) {
        throw ParseError("Unterminated HTML string", token_line, token_column);
    }
    
    return Token(TokenType::HTML_ID, value, token_line, token_column);
}

Token DotLexer::read_identifier() {
    size_t token_line = line_;
    size_t token_column = column_;
    std::string value;
    
    while (pos_ < text_.length() && (is_alnum(current_char()) || current_char() == '_')) {
        value += current_char();
        advance();
    }
    
    // Check if it's a keyword
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"strict", TokenType::STRICT},
        {"graph", TokenType::GRAPH},
        {"digraph", TokenType::DIGRAPH},
        {"node", TokenType::NODE},
        {"edge", TokenType::EDGE},
        {"subgraph", TokenType::SUBGRAPH}
    };
    
    auto it = keywords.find(value);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::ID;
    
    return Token(type, value, token_line, token_column);
}

Token DotLexer::read_number() {
    size_t token_line = line_;
    size_t token_column = column_;
    std::string value;
    
    // Read integer part
    while (pos_ < text_.length() && is_digit(current_char())) {
        value += current_char();
        advance();
    }
    
    // Read decimal part
    if (pos_ < text_.length() && current_char() == '.') {
        value += current_char();
        advance();
        while (pos_ < text_.length() && is_digit(current_char())) {
            value += current_char();
            advance();
        }
    }
    
    // Read exponent part
    if (pos_ < text_.length() && (current_char() == 'e' || current_char() == 'E')) {
        value += current_char();
        advance();
        if (pos_ < text_.length() && (current_char() == '+' || current_char() == '-')) {
            value += current_char();
            advance();
        }
        while (pos_ < text_.length() && is_digit(current_char())) {
            value += current_char();
            advance();
        }
    }
    
    return Token(TokenType::ID, value, token_line, token_column);
}

bool DotLexer::is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool DotLexer::is_digit(char c) const {
    return std::isdigit(c);
}

bool DotLexer::is_alnum(char c) const {
    return std::isalnum(c) || c == '_';
}

bool DotLexer::is_whitespace(char c) const {
    return std::isspace(c);
}

} // namespace dot
} // namespace xdot_cpp