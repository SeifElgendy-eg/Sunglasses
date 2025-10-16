#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QProgressBar>
#include <QComboBox>
#include <QStatusBar>
#include <QString>
#include <memory>
#include "CanvasWidget.h"
#include "Image_Class.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File operations
    void onLoadImage();
    void onSaveImage();
    void onResetImage();

    // History
    void onUndo();
    void onRedo();

    // Canvas tool operations
    void onSelectTool();
    void onResizeTool();

    // Canvas filter operations - Reflection
    void onCanvasVerticalReflection();
    void onCanvasHorizontalReflection();

    // Canvas filter operations - Color
    void onCanvasYellowFilter();
    void onCanvasPurpleFilter();
    void onCanvasInfraRedFilter();

    // Canvas history
    void onCanvasUndo();
    void onCanvasRedo();
    void onCanvasReset();

    // Image filter operations
    void onGrayscale();
    void onBlackAndWhite();
    void onInvert();
    void onReflect();
    void onRotate();
    void onLighten();
    void onDarken();
    void onCrop();
    void onFrame();
    void onEdges();
    void onBlur();
    void onResize();

    // Morph operations
    void onLoadTargetImage();
    void onLoadWeightsImage();
    void onMorph();
    void onMorphAnimated();
    void onBlendFactorChanged(double value);

    //Merge operations
    void onLoadTargetMergeImage();
    void onMerge();
    void onMergeBlendFactorChanged(double value);
    void onMergeModeChanged(const QString &text);

private:
    // UI Setup
    void setupUI();
    void applyModernStyle();
    void createMenuBar();
    void createToolBar();
    void createCentralWidget();
    void createFilterPanel();
    void createMorphPanel();
    void createMergePanel();

    // Utilities
    void updateImageDisplay();
    void updateStatusBar(const QString &message);
    QPixmap imageToPixmap(const Image &img);

    // Canvas Widget
    CanvasWidget *canvas;

    // Central widgets
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QProgressBar *progressBar;

    // Filter panel widgets
    QGroupBox *filterPanel;

    // Canvas filter buttons
    QPushButton *btnCanvasVertReflect;
    QPushButton *btnCanvasHorzReflect;
    QPushButton *btnCanvasYellow;
    QPushButton *btnCanvasPurple;
    QPushButton *btnCanvasInfraRed;
    QPushButton *btnCanvasUndo;
    QPushButton *btnCanvasRedo;
    QPushButton *btnCanvasReset;

    // Canvas filter controls
    QSpinBox *canvasColorIntensity;

    // Basic filter buttons
    QPushButton *btnGrayscale;
    QPushButton *btnBnW;
    QPushButton *btnInvert;
    QPushButton *btnReflect;
    QPushButton *btnEdges;

    // Rotation controls
    QPushButton *btnRotate;
    QSpinBox *rotateSpinBox;

    // Brightness controls
    QPushButton *btnLighten;
    QPushButton *btnDarken;
    QSpinBox *lightenSpinBox;
    QSpinBox *darkenSpinBox;

    // Blur controls
    QPushButton *btnBlur;
    QSpinBox *blurSpinBox;

    // Crop controls
    QPushButton *btnCrop;
    QSpinBox *cropXSpinBox;
    QSpinBox *cropYSpinBox;
    QSpinBox *cropWidthSpinBox;
    QSpinBox *cropHeightSpinBox;

    // Frame controls
    QPushButton *btnFrame;
    QSpinBox *frameThicknessSpinBox;
    QSpinBox *frameRSpinBox;
    QSpinBox *frameGSpinBox;
    QSpinBox *frameBSpinBox;

    // Resize controls
    QPushButton *btnResize;
    QSpinBox *resizeWidthSpinBox;
    QSpinBox *resizeHeightSpinBox;

    // Morph panel widgets
    QGroupBox *morphPanel;
    QPushButton *btnLoadTarget;
    QPushButton *btnLoadWeights;
    QPushButton *btnMorph;
    QPushButton *btnMorphAnimated;
    QLabel *targetImageLabel;
    QLabel *weightsImageLabel;
    QDoubleSpinBox *blendFactorSpinBox;
    QSpinBox *animateFramesSpinBox;

    // Merge panel widgets
    QGroupBox *mergePanel;
    QPushButton *btnMergeTarget;
    QPushButton *btnMerge;
    QLabel *targetMergeLabel;
    QDoubleSpinBox *mergeBlendFactorSpinBox;
    QComboBox *mergeModeComboBox;

    // Image data
    std::unique_ptr<Image> currentImage;
    std::unique_ptr<Image> originalImage;
    std::unique_ptr<Image> targetImage;
    std::unique_ptr<Image> weightsImage;
    std::unique_ptr<Image> targetMergeImage;

    // File paths
    QString currentFilePath;
    QString targetFilePath;
    QString weightsFilePath;
};

#endif
