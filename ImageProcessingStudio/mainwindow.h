#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QStatusBar>
#include "Image_Class.h"
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File operations
    void onLoadImage();
    void onSaveImage();
    void onLoadTargetImage();
    void onLoadWeightsImage();

    // Basic filters
    void onGrayscale();
    void onBlackAndWhite();
    void onInvert();
    void onReflect();
    void onRotate();
    void onLighten();
    void onDarken();

    // Advanced filters
    void onCrop();
    void onFrame();
    void onEdges();
    void onBlur();
    void onResize();
    void onMerge();

    // Morph operations
    void onMorph();
    void onMorphAnimated();
    void onBlendFactorChanged(double value);

    // UI helpers
    void onResetImage();
    void updateImageDisplay();
    void updateStatusBar(const QString &message);

private:
    // UI Setup
    void setupUI();
    void applyModernStyle();
    void createMenuBar();
    void createToolBar();
    void createCentralWidget();
    void createFilterPanel();
    void createMorphPanel();

    // Image management
    void displayImage(const Image &img);
    QPixmap imageToPixmap(const Image &img);

    // Member variables
    Ui::MainWindow *ui;

    // Images
    std::unique_ptr<Image> currentImage;
    std::unique_ptr<Image> originalImage;
    std::unique_ptr<Image> targetImage;
    std::unique_ptr<Image> weightsImage;
    QString currentFilePath;
    QString targetFilePath;
    QString weightsFilePath;

    // UI Components
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QProgressBar *progressBar;

    // Filter Panel
    QGroupBox *filterPanel;
    QPushButton *btnGrayscale;
    QPushButton *btnBnW;
    QPushButton *btnInvert;
    QPushButton *btnReflect;
    QPushButton *btnEdges;

    QSpinBox *rotateSpinBox;
    QPushButton *btnRotate;

    QSpinBox *lightenSpinBox;
    QPushButton *btnLighten;
    QSpinBox *darkenSpinBox;
    QPushButton *btnDarken;

    QSpinBox *blurSpinBox;
    QPushButton *btnBlur;

    // Crop controls
    QSpinBox *cropXSpinBox;
    QSpinBox *cropYSpinBox;
    QSpinBox *cropWidthSpinBox;
    QSpinBox *cropHeightSpinBox;
    QPushButton *btnCrop;

    // Frame controls
    QSpinBox *frameThicknessSpinBox;
    QSpinBox *frameRSpinBox;
    QSpinBox *frameGSpinBox;
    QSpinBox *frameBSpinBox;
    QPushButton *btnFrame;

    // Resize controls
    QSpinBox *resizeWidthSpinBox;
    QSpinBox *resizeHeightSpinBox;
    QPushButton *btnResize;

    // Morph Panel
    QGroupBox *morphPanel;
    QPushButton *btnLoadTarget;
    QPushButton *btnLoadWeights;
    QLabel *targetImageLabel;
    QLabel *weightsImageLabel;
    QDoubleSpinBox *blendFactorSpinBox;
    QSpinBox *animateFramesSpinBox;
    QPushButton *btnMorph;
    QPushButton *btnMorphAnimated;

    // Merge controls
    QPushButton *btnLoadMergeImage;
    QDoubleSpinBox *mergeAlphaSpinBox;
    QComboBox *mergeModeCombo;
    QPushButton *btnMerge;
    std::unique_ptr<Image> mergeImage;
};

#endif // MAINWINDOW_H
