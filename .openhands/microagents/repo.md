# XDot C++ Repository

## Purpose

This repository contains **xdot_cpp**, a C++ library and interactive viewer for graphs written in Graphviz's DOT language. It is a C++/Qt port of the Python xdot.py library, providing native performance and cross-platform GUI capabilities for visualizing and interacting with DOT graphs.

The project enables:
- Interactive viewing of Graphviz DOT files with pan, zoom, and navigation
- Native parsing of DOT language syntax and xdot extended drawing commands
- Qt-based modern graphical interface with node/edge interaction
- Integration with Graphviz for automatic DOT-to-xdot conversion

## General Setup

### Build System
- **CMake 3.16+** with C++17 standard
- **Qt5** framework (Core, Widgets, Gui modules)
- **Graphviz** system dependency for DOT processing
- Produces static libraries (`libxdot_core.a`, `libxdot_qt.a`) and executable (`xdot_viewer`)

### Dependencies
- Qt5 for GUI rendering and event handling
- Graphviz for DOT file processing and layout generation
- C++17 STL for modern C++ features

### Build Process
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./xdot_viewer examples/simple.dot
```

## Repository Structure

### Core Architecture
```
├── include/xdot_cpp/          # Public API headers
│   ├── dot/                   # DOT language parsing
│   │   ├── lexer.h           # DOT tokenizer
│   │   └── parser.h          # DOT syntax parser
│   ├── xdot/                 # XDot format handling
│   │   ├── xdot_parser.h     # XDot command parser
│   │   ├── elements.h        # Drawing elements (shapes, text)
│   │   ├── color.h           # Color management
│   │   ├── pen.h             # Pen/stroke styles
│   │   └── graph.h           # Graph data structures
│   └── ui/                   # Qt user interface
│       ├── dot_widget.h      # Main graph rendering widget
│       ├── dot_window.h      # Application main window
│       └── graphics_items.h  # Qt graphics scene items
```

### Implementation
```
├── src/                      # Implementation files
│   ├── dot/                  # DOT parsing implementation
│   │   ├── lexer.cpp         # Tokenization logic
│   │   └── parser.cpp        # Recursive descent parser
│   ├── xdot/                 # XDot processing implementation
│   │   ├── xdot_parser.cpp   # XDot command parsing
│   │   ├── elements.cpp      # Drawing element implementations
│   │   ├── color.cpp         # Color parsing and conversion
│   │   └── graph.cpp         # Graph data structure management
│   ├── ui/                   # Qt UI implementation
│   │   ├── dot_widget.cpp    # QGraphicsView-based widget
│   │   └── dot_window.cpp    # Main window with menus/toolbars
│   └── main.cpp              # Application entry point
```

### Supporting Files
```
├── examples/                 # Sample DOT files for testing
│   ├── simple.dot           # Basic graph example
│   └── complex.dot          # Advanced graph with subgraphs
├── xdot.py-main/            # Python reference implementation
├── tests/                   # Test files and utilities
├── CMakeLists.txt           # Build configuration
└── README.md                # User documentation
```

### Key Components

1. **DOT Parser**: Complete lexer/parser for Graphviz DOT language syntax
2. **XDot Parser**: Processes xdot drawing commands for precise rendering
3. **Graphics Elements**: Shape, color, and style representations for Qt rendering
4. **Qt Widget**: Interactive QGraphicsView-based widget with zoom/pan
5. **Main Application**: Complete GUI application with file loading and menus

The architecture follows a layered approach: DOT parsing → XDot processing → Qt rendering, with clear separation between parsing logic and UI components.