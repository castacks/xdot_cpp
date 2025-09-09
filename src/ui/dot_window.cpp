#include "xdot_cpp/ui/dot_window.h"
#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QTextStream>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QShortcut>
#include <QPushButton>
#include <QDebug>

namespace xdot_cpp {
namespace ui {

DotWindow::DotWindow(QWidget* parent)
    : QMainWindow(parent), toolbar_visible_(true), find_widget_visible_(false) {
    
    // Create central widget
    dot_widget_ = new DotWidget(this);
    setCentralWidget(dot_widget_);
    
    // Create UI components
    create_actions();
    create_menus();
    create_toolbar();
    create_status_bar();
    create_find_widget();
    setup_shortcuts();
    
    // Connect signals
    connect(dot_widget_, &DotWidget::node_clicked, this, &DotWindow::on_node_clicked);
    connect(dot_widget_, &DotWidget::edge_clicked, this, &DotWindow::on_edge_clicked);
    connect(dot_widget_, &DotWidget::background_clicked, this, &DotWindow::on_background_clicked);
    
    // Set window properties
    setWindowTitle("xdot_cpp - Graphviz Viewer");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    update_window_title();
}

void DotWindow::set_dot_code(const std::string& dot_code) {
    dot_widget_->set_dot_code(dot_code);
    status_label_->setText("DOT code loaded");
}

void DotWindow::set_xdot_code(const std::string& xdot_code) {
    dot_widget_->set_xdot_code(xdot_code);
    status_label_->setText("xdot code loaded");
}

void DotWindow::load_file(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + filename);
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    current_filename_ = filename;
    
    // Determine file type and process accordingly
    if (filename.endsWith(".dot") || filename.endsWith(".gv")) {
        // For now, skip Graphviz processing and parse as regular DOT
        // TODO: Fix Graphviz integration
        set_dot_code(content.toStdString());
    } else {
        // Assume it's already xdot format
        set_xdot_code(content.toStdString());
    }
    
    update_window_title();
    status_label_->setText("Loaded: " + QFileInfo(filename).fileName());
}

void DotWindow::open_file() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Open Graph File",
        "",
        "Graph files (*.dot *.gv *.xdot);;All files (*)"
    );
    
    if (!filename.isEmpty()) {
        load_file(filename);
    }
}

void DotWindow::reload_file() {
    if (!current_filename_.isEmpty()) {
        load_file(current_filename_);
    } else {
        dot_widget_->reload();
    }
}

void DotWindow::save_as() {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Graph",
        "",
        "PNG files (*.png);;SVG files (*.svg);;PDF files (*.pdf)"
    );
    
    if (!filename.isEmpty()) {
        // This would implement saving functionality
        // For now, just show a message
        QMessageBox::information(this, "Save", "Save functionality not yet implemented");
    }
}

void DotWindow::print() {
    // This would implement printing functionality
    QMessageBox::information(this, "Print", "Print functionality not yet implemented");
}

void DotWindow::quit() {
    close();
}

void DotWindow::zoom_in() {
    dot_widget_->zoom_in();
}

void DotWindow::zoom_out() {
    dot_widget_->zoom_out();
}

void DotWindow::zoom_to_fit() {
    dot_widget_->zoom_to_fit();
}

void DotWindow::reset_zoom() {
    dot_widget_->reset_zoom();
}

void DotWindow::find() {
    if (find_widget_visible_) {
        hide_find_widget();
    } else {
        show_find_widget();
    }
}

void DotWindow::find_next() {
    // This would implement find next functionality
    status_label_->setText("Find next not yet implemented");
}

void DotWindow::find_previous() {
    // This would implement find previous functionality
    status_label_->setText("Find previous not yet implemented");
}

void DotWindow::toggle_toolbar() {
    toolbar_visible_ = !toolbar_visible_;
    toolbar_->setVisible(toolbar_visible_);
    toggle_toolbar_action_->setText(toolbar_visible_ ? "Hide Toolbar" : "Show Toolbar");
}

void DotWindow::about() {
    QMessageBox::about(this, "About xdot_cpp",
        "<h2>xdot_cpp</h2>"
        "<p>A C++/Qt viewer for Graphviz dot files</p>"
        "<p>Based on the Python xdot.py implementation</p>"
        "<p>Version 1.0</p>");
}

void DotWindow::closeEvent(QCloseEvent* event) {
    event->accept();
}

void DotWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        if (find_widget_visible_) {
            hide_find_widget();
            event->accept();
            return;
        }
    }
    
    QMainWindow::keyPressEvent(event);
}

void DotWindow::on_node_clicked(const QString& node_id, const QString& url) {
    status_label_->setText("Node clicked: " + node_id);
    
    if (!url.isEmpty()) {
        // Open URL if provided
        QDesktopServices::openUrl(QUrl(url));
    }
}

void DotWindow::on_edge_clicked(const QString& source, const QString& target, const QString& url) {
    status_label_->setText("Edge clicked: " + source + " -> " + target);
    
    if (!url.isEmpty()) {
        // Open URL if provided
        QDesktopServices::openUrl(QUrl(url));
    }
}

void DotWindow::on_background_clicked() {
    status_label_->setText("Background clicked");
}

void DotWindow::on_find_text_changed(const QString& text) {
    if (!text.isEmpty()) {
        dot_widget_->find_node(text.toStdString());
    }
}

void DotWindow::on_find_return_pressed() {
    QString text = find_line_edit_->text();
    if (!text.isEmpty()) {
        dot_widget_->find_node(text.toStdString());
        status_label_->setText("Searching for: " + text);
    }
}

void DotWindow::create_actions() {
    // File actions
    open_action_ = new QAction("&Open...", this);
    open_action_->setShortcut(QKeySequence::Open);
    open_action_->setStatusTip("Open a graph file");
    connect(open_action_, &QAction::triggered, this, &DotWindow::open_file);
    
    reload_action_ = new QAction("&Reload", this);
    reload_action_->setShortcut(QKeySequence::Refresh);
    reload_action_->setStatusTip("Reload the current file");
    connect(reload_action_, &QAction::triggered, this, &DotWindow::reload_file);
    
    save_as_action_ = new QAction("Save &As...", this);
    save_as_action_->setShortcut(QKeySequence::SaveAs);
    save_as_action_->setStatusTip("Save the graph as an image");
    connect(save_as_action_, &QAction::triggered, this, &DotWindow::save_as);
    
    print_action_ = new QAction("&Print...", this);
    print_action_->setShortcut(QKeySequence::Print);
    print_action_->setStatusTip("Print the graph");
    connect(print_action_, &QAction::triggered, this, &DotWindow::print);
    
    quit_action_ = new QAction("&Quit", this);
    quit_action_->setShortcut(QKeySequence::Quit);
    quit_action_->setStatusTip("Quit the application");
    connect(quit_action_, &QAction::triggered, this, &DotWindow::quit);
    
    // View actions
    zoom_in_action_ = new QAction("Zoom &In", this);
    zoom_in_action_->setShortcut(QKeySequence::ZoomIn);
    zoom_in_action_->setStatusTip("Zoom in");
    connect(zoom_in_action_, &QAction::triggered, this, &DotWindow::zoom_in);
    
    zoom_out_action_ = new QAction("Zoom &Out", this);
    zoom_out_action_->setShortcut(QKeySequence::ZoomOut);
    zoom_out_action_->setStatusTip("Zoom out");
    connect(zoom_out_action_, &QAction::triggered, this, &DotWindow::zoom_out);
    
    zoom_to_fit_action_ = new QAction("Zoom to &Fit", this);
    zoom_to_fit_action_->setShortcut(QKeySequence("Ctrl+0"));
    zoom_to_fit_action_->setStatusTip("Zoom to fit the entire graph");
    connect(zoom_to_fit_action_, &QAction::triggered, this, &DotWindow::zoom_to_fit);
    
    reset_zoom_action_ = new QAction("&Reset Zoom", this);
    reset_zoom_action_->setShortcut(QKeySequence("Ctrl+1"));
    reset_zoom_action_->setStatusTip("Reset zoom to 100%");
    connect(reset_zoom_action_, &QAction::triggered, this, &DotWindow::reset_zoom);
    
    find_action_ = new QAction("&Find...", this);
    find_action_->setShortcut(QKeySequence::Find);
    find_action_->setStatusTip("Find a node");
    connect(find_action_, &QAction::triggered, this, &DotWindow::find);
    
    toggle_toolbar_action_ = new QAction("Hide Toolbar", this);
    toggle_toolbar_action_->setStatusTip("Toggle toolbar visibility");
    connect(toggle_toolbar_action_, &QAction::triggered, this, &DotWindow::toggle_toolbar);
    
    // Help actions
    about_action_ = new QAction("&About", this);
    about_action_->setStatusTip("About this application");
    connect(about_action_, &QAction::triggered, this, &DotWindow::about);
}

void DotWindow::create_menus() {
    menu_bar_ = menuBar();
    
    // File menu
    file_menu_ = menu_bar_->addMenu("&File");
    file_menu_->addAction(open_action_);
    file_menu_->addAction(reload_action_);
    file_menu_->addSeparator();
    file_menu_->addAction(save_as_action_);
    file_menu_->addAction(print_action_);
    file_menu_->addSeparator();
    file_menu_->addAction(quit_action_);
    
    // View menu
    view_menu_ = menu_bar_->addMenu("&View");
    view_menu_->addAction(zoom_in_action_);
    view_menu_->addAction(zoom_out_action_);
    view_menu_->addAction(zoom_to_fit_action_);
    view_menu_->addAction(reset_zoom_action_);
    view_menu_->addSeparator();
    view_menu_->addAction(find_action_);
    view_menu_->addSeparator();
    view_menu_->addAction(toggle_toolbar_action_);
    
    // Help menu
    help_menu_ = menu_bar_->addMenu("&Help");
    help_menu_->addAction(about_action_);
}

void DotWindow::create_toolbar() {
    toolbar_ = addToolBar("Main");
    toolbar_->addAction(open_action_);
    toolbar_->addAction(reload_action_);
    toolbar_->addSeparator();
    toolbar_->addAction(zoom_in_action_);
    toolbar_->addAction(zoom_out_action_);
    toolbar_->addAction(zoom_to_fit_action_);
    toolbar_->addAction(reset_zoom_action_);
    toolbar_->addSeparator();
    toolbar_->addAction(find_action_);
}

void DotWindow::create_status_bar() {
    status_bar_ = statusBar();
    status_label_ = new QLabel("Ready");
    status_bar_->addWidget(status_label_);
}

void DotWindow::create_find_widget() {
    find_widget_ = new QWidget(this);
    find_widget_->setVisible(false);
    
    QHBoxLayout* layout = new QHBoxLayout(find_widget_);
    layout->addWidget(new QLabel("Find node:"));
    
    find_line_edit_ = new QLineEdit(find_widget_);
    layout->addWidget(find_line_edit_);
    
    QPushButton* close_button = new QPushButton("×", find_widget_);
    close_button->setMaximumWidth(30);
    layout->addWidget(close_button);
    
    connect(find_line_edit_, &QLineEdit::textChanged, this, &DotWindow::on_find_text_changed);
    connect(find_line_edit_, &QLineEdit::returnPressed, this, &DotWindow::on_find_return_pressed);
    connect(close_button, &QPushButton::clicked, this, &DotWindow::hide_find_widget);
    
    // Add find widget to main layout
    QWidget* central_widget = new QWidget(this);
    QVBoxLayout* main_layout = new QVBoxLayout(central_widget);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addWidget(find_widget_);
    main_layout->addWidget(dot_widget_);
    
    setCentralWidget(central_widget);
}

void DotWindow::setup_shortcuts() {
    // Additional shortcuts not covered by actions
    new QShortcut(QKeySequence("F5"), this, SLOT(reload_file()));
    new QShortcut(QKeySequence("Escape"), this, SLOT(hide_find_widget()));
}

void DotWindow::update_window_title() {
    QString title = "xdot_cpp - Graphviz Viewer";
    if (!current_filename_.isEmpty()) {
        title += " - " + QFileInfo(current_filename_).fileName();
    }
    setWindowTitle(title);
}

void DotWindow::show_find_widget() {
    find_widget_->setVisible(true);
    find_widget_visible_ = true;
    find_line_edit_->setFocus();
    find_line_edit_->selectAll();
}

void DotWindow::hide_find_widget() {
    find_widget_->setVisible(false);
    find_widget_visible_ = false;
    dot_widget_->setFocus();
}

std::string DotWindow::run_graphviz_filter(const std::string& dot_code, const std::string& filter) {
    QProcess process;
    process.start(QString::fromStdString(filter), QStringList() << "-Txdot");
    
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start Graphviz process";
        return "";
    }
    
    process.write(dot_code.c_str());
    process.closeWriteChannel();
    
    if (!process.waitForFinished(10000)) { // 10 second timeout
        qDebug() << "Graphviz process timed out";
        process.kill();
        return "";
    }
    
    if (process.exitCode() != 0) {
        qDebug() << "Graphviz process failed:" << process.readAllStandardError();
        return "";
    }
    
    QByteArray output = process.readAllStandardOutput();
    return output.toStdString();
}

} // namespace ui
} // namespace xdot_cpp

