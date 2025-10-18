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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void applyMorphFilter(double blendFactor);
    void applyMorphAnimated(int frameCount, double blendFactor);
    void applyMergeToCanvas();

    void openImage();
    void saveImage();
    void showApplyCancelButtons();
    void hideApplyCancelButtons();
    void exitApp();

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
    void onCropTool();

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
    void onApplyBnWFilter();
    void onPreviewBnWFilter(int threshold);
    void onApplyBlurFilter();
    void onPreviewBlurFilter(int kernelSize);
    void onApplyLightOrDarkFilter();
    void onPreviewLightOrDarkFilter(int percent);
    void onApplyOilPaintFilter();
    void onPreviewOilPaintFilter(int kernelSize);
    void onApplySkewFilter();
    void onPreviewSkewFilter(double degree);

private:
    void createMenu();
    void createToolBar();
    void applyModernStyle();
    QWidget* createFilterSidePanel();
    QWidget* createMorphPanel();
    QWidget* createMergePanel();
    void loadTargetImage();
    void loadWeightsImage();
    void loadTargetMergeImage();
    void applyMergeFilter(double alpha, char mode);
    QPixmap imageToPixmap(const Image &img);

    CanvasWidget *canvas;
    QTabWidget *tabWidget = new QTabWidget();
    QToolBar *tb;
    QMenuBar *menuBar;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    // Filter panel widgets
    QGroupBox *filterPanel;
    QSlider *bnwThresholdSlider;
    QPushButton *btnBnw;
    QSpinBox *blurSpinBox;
    QPushButton *btnBlur;
    QSlider *lightOrDarkSlider;
    QPushButton *btnLightOrDark;
    QSlider *oilPaintSlider;
    QPushButton *btnOilPaint;
    QDoubleSpinBox *skewSpinBox;
    QPushButton *btnSkew;


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
    QLabel *outputMergeLabel;
    QDoubleSpinBox *mergeBlendFactorSpinBox;
    QComboBox *mergeModeComboBox;

    // Image data
    std::unique_ptr<Image> targetImage;
    std::unique_ptr<Image> weightsImage;
    std::unique_ptr<Image> targetMergeImage;
    std::unique_ptr<Image> outputMergeImage;
};

#endif
