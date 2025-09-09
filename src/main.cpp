#include "xdot_cpp/ui/dot_window.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("xdot_cpp");
    app.setApplicationVersion("1.0");
    app.setApplicationDisplayName("xdot_cpp - Graphviz Viewer");
    app.setOrganizationName("xdot_cpp");
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("A C++/Qt viewer for Graphviz dot files");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add positional argument for file
    parser.addPositionalArgument("file", "Graph file to open (.dot, .gv, .xdot)");
    
    // Add options
    QCommandLineOption filterOption(QStringList() << "f" << "filter",
                                   "Graphviz filter to use (dot, neato, fdp, sfdp, twopi, circo)",
                                   "filter", "dot");
    parser.addOption(filterOption);
    
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                   "Output format (xdot, svg, png, pdf)",
                                   "format", "xdot");
    parser.addOption(outputOption);
    
    QCommandLineOption noGuiOption(QStringList() << "n" << "no-gui",
                                  "Run without GUI (convert only)");
    parser.addOption(noGuiOption);
    
    // Process arguments
    parser.process(app);
    
    const QStringList args = parser.positionalArguments();
    
    // Create main window
    std::cout << "Creating main window..." << std::endl;
    xdot_cpp::ui::DotWindow window;
    std::cout << "Main window created." << std::endl;
    
    // Load file if specified
    if (!args.isEmpty()) {
        QString filename = args.first();
        std::cout << "Loading file: " << filename.toStdString() << std::endl;
        
        if (parser.isSet(noGuiOption)) {
            // Command-line mode: convert file and exit
            std::cout << "Converting " << filename.toStdString() << "..." << std::endl;
            // This would implement command-line conversion
            return 0;
        } else {
            // GUI mode: load file in window
            window.load_file(filename);
            std::cout << "File loaded." << std::endl;
        }
    }
    
    // Show window and run application
    std::cout << "Showing window..." << std::endl;
    window.show();
    window.raise();
    window.activateWindow();
    std::cout << "Window shown. Starting event loop..." << std::endl;
    
    return app.exec();
}