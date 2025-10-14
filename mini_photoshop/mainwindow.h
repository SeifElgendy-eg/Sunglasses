#pragma once
#include <QMainWindow>

class CanvasWidget; // forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void openImage();
    void saveImage();
    void exitApp();

private:
    void createMenu();
    QWidget* createFilterSidePanel();
    void applyModernStyle();
    void createToolBar();
    void createSliders();

    CanvasWidget *canvas = nullptr;
};
