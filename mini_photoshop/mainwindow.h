#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include "Image_Class.h"

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
    QTabWidget *tabWidget = new QTabWidget();

    void createMenu();

    void applyModernStyle();
    void createToolBar();
    void createSliders();
    QWidget* createFilterSidePanel();
    QWidget* createMorphPanel();
    QWidget* createMergePanel();

    void loadTargetImage();
    void loadWeightsImage();
    void loadTargetMergeImage();

    QLabel *targetImageLabel;
    QLabel *weightsImageLabel;
    QLabel *targetMergeLabel;
    QLabel *outputMergeLabel;

    std::unique_ptr<Image> targetImage;
    std::unique_ptr<Image> weightsImage;
    std::unique_ptr<Image> targetMergeImage;
    std::unique_ptr<Image> outputMergeImage;

    QToolBar *tb;
    QMenuBar *menuBar;
    QPushButton *applyButton;
    QPushButton *cancelButton;
    QPixmap imageToPixmap(const Image &img);

    void applyMergeFilter (double alpha = 0.4, char mode ='f');

    CanvasWidget *canvas = nullptr;
};
