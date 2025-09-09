#pragma once

#include "dot_widget.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace xdot_cpp {
namespace ui {

class DotWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit DotWindow(QWidget* parent = nullptr);
    
    void set_dot_code(const std::string& dot_code);
    void set_xdot_code(const std::string& xdot_code);
    void load_file(const QString& filename);
    
    DotWidget* dot_widget() const { return dot_widget_; }
    
public slots:
    void open_file();
    void reload_file();
    void save_as();
    void print();
    void quit();
    
    void zoom_in();
    void zoom_out();
    void zoom_to_fit();
    void reset_zoom();
    
    void find();
    void find_next();
    void find_previous();
    
    void toggle_toolbar();
    void about();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    
private slots:
    void on_node_clicked(const QString& node_id, const QString& url);
    void on_edge_clicked(const QString& source, const QString& target, const QString& url);
    void on_background_clicked();
    
    void on_find_text_changed(const QString& text);
    void on_find_return_pressed();
    void hide_find_widget();
    
private:
    DotWidget* dot_widget_;
    
    // Menus
    QMenuBar* menu_bar_;
    QMenu* file_menu_;
    QMenu* view_menu_;
    QMenu* help_menu_;
    
    // Toolbar
    QToolBar* toolbar_;
    bool toolbar_visible_;
    
    // Status bar
    QStatusBar* status_bar_;
    QLabel* status_label_;
    
    // Actions
    QAction* open_action_;
    QAction* reload_action_;
    QAction* save_as_action_;
    QAction* print_action_;
    QAction* quit_action_;
    
    QAction* zoom_in_action_;
    QAction* zoom_out_action_;
    QAction* zoom_to_fit_action_;
    QAction* reset_zoom_action_;
    
    QAction* find_action_;
    QAction* toggle_toolbar_action_;
    QAction* about_action_;
    
    // Find functionality
    QWidget* find_widget_;
    QLineEdit* find_line_edit_;
    bool find_widget_visible_;
    
    // File handling
    QString current_filename_;
    
    void create_actions();
    void create_menus();
    void create_toolbar();
    void create_status_bar();
    void create_find_widget();
    
    void setup_shortcuts();
    void update_window_title();
    void show_find_widget();
    
    std::string run_graphviz_filter(const std::string& dot_code, const std::string& filter = "dot");
};

} // namespace ui
} // namespace xdot_cpp