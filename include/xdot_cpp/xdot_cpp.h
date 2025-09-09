#pragma once

/**
 * @file xdot_cpp.h
 * @brief Main header file for the xdot_cpp library
 * 
 * This library provides C++ classes for parsing and rendering Graphviz DOT files
 * using the xdot format. It includes both a core library (no GUI dependencies)
 * and Qt-based widgets for interactive viewing.
 */

#include "dot/lexer.h"
#include "dot/parser.h"
#include "xdot/xdot_parser.h"
#include "xdot/pen.h"
#include "xdot/color.h"
#include "xdot/elements.h"
#include "xdot/graph.h"

namespace xdot_cpp {

/**
 * @brief Library version information
 */
struct Version {
    static constexpr int MAJOR = 1;
    static constexpr int MINOR = 0;
    static constexpr int PATCH = 0;
    
    static std::string toString() {
        return std::to_string(MAJOR) + "." + 
               std::to_string(MINOR) + "." + 
               std::to_string(PATCH);
    }
};

// Legacy version constant for compatibility
constexpr const char* VERSION = "1.0.0";

} // namespace xdot_cpp