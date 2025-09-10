cmake_minimum_required(VERSION 3.16)
project(xdot_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library sources
set(XDOT_LIB_SOURCES
    src/dot/lexer.cpp
    src/dot/parser.cpp
    src/xdot/xdot_parser.cpp
    src/xdot/color.cpp
    src/xdot/elements.cpp
    src/xdot/graph.cpp
)

# Library headers
set(XDOT_LIB_HEADERS
    include/xdot_cpp/dot/lexer.h
    include/xdot_cpp/dot/parser.h
    include/xdot_cpp/xdot/xdot_parser.h
    include/xdot_cpp/xdot/pen.h
    include/xdot_cpp/xdot/color.h
    include/xdot_cpp/xdot/elements.h
    include/xdot_cpp/xdot/graph.h
    include/xdot_cpp/xdot_cpp.h
)

# Qt widget sources
set(XDOT_WIDGET_SOURCES
    src/ui/dot_widget.cpp
    src/ui/dot_window.cpp
)

# Qt widget headers
set(XDOT_WIDGET_HEADERS
    include/xdot_cpp/ui/dot_widget.h
    include/xdot_cpp/ui/dot_window.h
)

# Create the core library (without Qt dependencies)
add_library(xdot_core STATIC ${XDOT_LIB_SOURCES} ${XDOT_LIB_HEADERS})
target_include_directories(xdot_core PUBLIC include)

# Create the Qt widget library
add_library(xdot_qt STATIC ${XDOT_WIDGET_SOURCES} ${XDOT_WIDGET_HEADERS})
target_link_libraries(xdot_qt PUBLIC xdot_core Qt5::Core Qt5::Widgets Qt5::Gui)
target_include_directories(xdot_qt PUBLIC include)

# Create the viewer application
add_executable(xdot_viewer src/main.cpp)
target_link_libraries(xdot_viewer xdot_qt)

# Install targets
install(TARGETS xdot_core xdot_qt xdot_viewer
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Examples are in the examples/ directory