#include "mainwindow.h"
#include "CanvasWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>
#include <QComboBox>

extern void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode);
extern void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor);
extern void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage,
                          const std::string &outputPath, int frameCount, double blendFactor);


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createMenu();
    createToolBar();
    applyModernStyle();
    canvas = new CanvasWidget(this);


    QWidget *filterPanel = createFilterSidePanel();
    QWidget *morphPanel  =  createMorphPanel();
    QWidget *mergePanel = createMergePanel();

    tabWidget->addTab(filterPanel, "Filters");
    tabWidget->addTab(morphPanel, "Morph");
    tabWidget->addTab(mergePanel,"Merge");
    tabWidget->setMinimumWidth(350);
    tabWidget->setMaximumWidth(500);
    QSplitter *splitter = new QSplitter(this);

    splitter->addWidget(canvas);
    splitter->addWidget(tabWidget);

    splitter->setSizes({750, 250});

    setCentralWidget(splitter);

    setWindowTitle("Mini Photoshop");
    resize(1000, 700);


    QToolBar *cropBar = addToolBar("Crop Preview");

    QWidget *editBar = new QWidget(cropBar);
    QHBoxLayout *editLayout = new QHBoxLayout(editBar);
    applyButton  =  new QPushButton("Apply", editBar);
    cancelButton =  new QPushButton("Cancel", editBar);
    cropBar->addWidget(editBar);
    cropBar->setVisible(false);

    editLayout->addWidget(applyButton);
    editLayout->addWidget(cancelButton);

    connect(canvas, &CanvasWidget::previewModeChanged, this, [editBar](bool enabled){
        editBar->setVisible(enabled);
    });

    connect(canvas, &CanvasWidget::previewModeChanged, this, [this,cropBar](bool enabled) {
        tb->setEnabled(!enabled);
        menuBar->setEnabled(!enabled);
        tabWidget->setEnabled(!enabled);

        cropBar ->setEnabled(enabled);
        cropBar ->setVisible(enabled);
    });

    connect(applyButton, &QPushButton::clicked, this, [this]() {
        QRect r = canvas->m_cropRect.toRect(); // Added .toRect()

        connect(applyButton, &QPushButton::clicked, this, [this]() {
            canvas->applyCrop();

            canvas->setPreviewMode(false);
            canvas->setTool(CanvasWidget::ToolMode::None);
        });

        canvas->setPreviewMode(false);
        canvas->setTool(CanvasWidget::ToolMode::None);
    });

    connect(cancelButton, &QPushButton::clicked, this, [this]() {
        canvas->setPreviewMode(false);
        canvas->setTool(CanvasWidget::ToolMode::None);
    });


}

void MainWindow::createMenu() {
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *openAction = fileMenu->addAction("&Open Image");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    QAction *saveAction = fileMenu->addAction("&Save Image");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar->addMenu("&Edit");

    QAction *resetAction = editMenu->addAction("&Reset to Original");
    resetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(resetAction, &QAction::triggered, this,
            [this]() { canvas->resetImage(); });

    // UNDO ACTION
    QAction *undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo); // Ctrl+Z
    connect(undoAction, &QAction::triggered, this, [this]() { canvas->undo(); });

    // REDO ACTION
    QAction *redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcut(QKeySequence::Redo); // Ctrl+Shift+Z
    connect(redoAction, &QAction::triggered, this, [this]() { canvas->redo(); });

    QMenu *filtersMenu = menuBar->addMenu("&Filters");

    QAction *grayscaleAction = filtersMenu->addAction("&Grayscale");
    grayscaleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(grayscaleAction, &QAction::triggered, this,
            [this]() { canvas->applyGrayScaleFilter(); });

    QAction *bnwAction = filtersMenu->addAction("&Black & White");
    bnwAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(bnwAction, &QAction::triggered, this, [this]() {
        canvas->applyBlackAndWhiteFilter(128, CanvasWidget::FilterMode::Increment);
        canvas->commitChanges();
    });

    QAction *invertAction = filtersMenu->addAction("&Invert Colors");
    invertAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    connect(invertAction, &QAction::triggered, this,
            [this]() { canvas->applyInversionFilter(); });

    QAction *frameAction = filtersMenu->addAction("Image Frame");

    connect(frameAction, &QAction::triggered, this,
            [this]() {
                canvas->saveState();
                QDialog dialog(this);
                dialog.setWindowTitle("Frame");

                QVBoxLayout *layout = new QVBoxLayout(
                    &dialog);
                // Frame
                QGroupBox *frameGroup = new QGroupBox("Add Frame", &dialog);
                QGridLayout *frameLayout = new QGridLayout(frameGroup);
                frameLayout->setSpacing(8);
                frameLayout->setContentsMargins(10, 22, 10, 10);

                frameLayout->addWidget(new QLabel("Thickness:"), 0, 0);
                QSpinBox *frameThicknessSpinBox = new QSpinBox(frameGroup);
                frameThicknessSpinBox->setRange(1, 200);
                frameThicknessSpinBox->setValue(20);
                frameThicknessSpinBox->setSuffix("px");
                frameLayout->addWidget(frameThicknessSpinBox, 0, 1, 1, 3);

                frameLayout->addWidget(new QLabel("R:"), 1, 0);
                QSpinBox *frameRSpinBox = new QSpinBox(frameGroup);
                frameRSpinBox->setRange(0, 255);
                frameLayout->addWidget(frameRSpinBox, 1, 1);

                frameLayout->addWidget(new QLabel("G:"), 1, 2);
                QSpinBox *frameGSpinBox = new QSpinBox(frameGroup);
                frameGSpinBox->setRange(0, 255);
                frameLayout->addWidget(frameGSpinBox, 1, 3);

                frameLayout->addWidget(new QLabel("B:"), 2, 0);
                QSpinBox *frameBSpinBox = new QSpinBox(frameGroup);
                frameBSpinBox->setRange(0, 255);
                frameLayout->addWidget(frameBSpinBox, 2, 1);

                QPushButton *btnFrame = new QPushButton("Apply Frame", frameGroup);

                frameLayout->addWidget(btnFrame, 2, 2, 1, 2);

                layout->addWidget(frameGroup);

                auto applyFramePreview = [this, frameThicknessSpinBox, frameRSpinBox, frameGSpinBox, frameBSpinBox]() {
                    int thickness = frameThicknessSpinBox->value();
                    int r = frameRSpinBox->value();
                    int g = frameGSpinBox->value();
                    int b = frameBSpinBox->value();
                    canvas->applyFrameFilter(thickness, r, g, b);
                };

                connect(frameThicknessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, applyFramePreview);
                connect(frameRSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, applyFramePreview);
                connect(frameGSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, applyFramePreview);
                connect(frameBSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, applyFramePreview);

                connect(btnFrame, &QPushButton::clicked, &dialog, [this, &dialog]() {
                    canvas->commitChanges();
                    dialog.accept();
                });

                connect(&dialog, &QDialog::rejected, this,
                        [this]() { canvas->cancelChanges(); });
                dialog.exec();
    });

    QAction *reflectVAction = filtersMenu->addAction("&Veritical Flip");
    connect(reflectVAction, &QAction::triggered, this,
            [this]() { canvas->applyVeriticalReflection(); });

    QAction *reflectHAction = filtersMenu->addAction("&Horizontal Flip");
    connect(reflectHAction, &QAction::triggered, this,
            [this]() { canvas->applyHorizontalReflection(); });

    filtersMenu->addSeparator();

    QAction *edgesAction = filtersMenu->addAction("&Edge Detection");
    connect(edgesAction, &QAction::triggered, this,
            [this]() { canvas->applyEdgeDetection(); });

    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this,[this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("About");
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("<h2 style='color: #2563eb;'>Image Processing Studio</h2>");
        msgBox.setInformativeText(
            "<p style='font-size: 11pt;'><b>Team Members:</b><br>"
            "• Marwan Mohamed Hassan (20240735)<br>"
            "• Mohamed Talat Sayed (20240734)<br>"
            "• Seif eldin Hatem Mahmoud (20242424)</p>"
            "<p style='font-size: 10pt; color: #808080;'>Section: S4</p>");
        msgBox.setStyleSheet("QMessageBox { background-color: #252525; }");
        msgBox.exec();
    });
}

void MainWindow::createToolBar() {

    QAction *resizeAct = new QAction("Resize", this);
    resizeAct->setToolTip("Resize the image");
    resizeAct->setCheckable(false);

    connect(resizeAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Resize); });

    QAction *selectAct = new QAction("Select", this);
    selectAct->setToolTip("Select pixels to modify");
    selectAct->setCheckable(false);

    connect(selectAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Select); });

    QAction *moveAct = new QAction("Move", this);
    moveAct->setToolTip("move the image freely in the canvas without ctrl");
    moveAct->setCheckable(false);

    connect(moveAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Move); });

    QAction *lightenAct = new QAction("Birghten Image", this);
    lightenAct->setToolTip("Enhance reds and greens");
    lightenAct->setCheckable(false);

    connect(lightenAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Adjust the green level");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("reds & greens :", &dialog);

        slider->setRange(-100, 100);
        slider->setValue(20);
        canvas->applyYellowFilter(slider->value(),
                                  CanvasWidget::FilterMode::Preview);
        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyYellowFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *purpleAct = new QAction("Purple Image", this);
    purpleAct->setToolTip("Enhance reds and blues");
    purpleAct->setCheckable(false);

    connect(purpleAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("reds and blues");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel(":", &dialog);

        slider->setRange(-100, 100);
        slider->setValue(30);
        canvas->applyPurpleFilter(slider->value(),
                                  CanvasWidget::FilterMode::Preview);
        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyPurpleFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *blackAndWhiteAct = new QAction("Black and white image", this);
    blackAndWhiteAct->setToolTip("Make a classic image");
    blackAndWhiteAct->setCheckable(false);

    connect(blackAndWhiteAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Black & White ");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Threshold:", &dialog);

        slider->setRange(0, 255);
        slider->setValue(128);

        canvas->applyBlackAndWhiteFilter(slider->value(),
                                         CanvasWidget::FilterMode::Preview);
        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyBlackAndWhiteFilter(value,
                                             CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *blurAct = new QAction("Blur", this);
    blurAct->setToolTip("Make the image blury");
    blurAct->setCheckable(false);

    connect(blurAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Adjust Blur");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Blur strength:", &dialog);

        slider->setRange(1, 15);
        slider->setValue(3);
        canvas->applyBlurFilter(slider->value(), CanvasWidget::FilterMode::Preview);
        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyBlurFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *brightAct = new QAction("Brightness", this);
    brightAct->setToolTip("Control the brightness of the image");
    brightAct->setCheckable(false);

    connect(brightAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Adjust Brightness");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Brightness:", &dialog);

        slider->setRange(-100, 100);
        slider->setValue(0);

        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyLightOrDarkFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });


    QAction *oilAct = new QAction("Oil Paint", this);
    oilAct->setToolTip("Simulate oil painting of the image");
    oilAct->setCheckable(false);

    connect(oilAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Oil Piant");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("paint level:", &dialog);

        slider->setRange(1,20);
        slider->setValue(0);

        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applyOilPaintFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *skewAct = new QAction("Skew", this);
    skewAct->setToolTip("Transform the image into the 3d prespective");
    skewAct->setCheckable(false);

    connect(skewAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Adjust Skew");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Angle:", &dialog);

        slider->setRange(0, 89);
        slider->setValue(0);

        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);

        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            canvas->applySkewFilter(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *cropAct =  new QAction("Crop",this);
    cropAct ->setToolTip("Crop The Image");
    cropAct ->setCheckable(false);
    connect(cropAct, &QAction::triggered, this, [this](){
        canvas->saveState();
        canvas->setTool(CanvasWidget::ToolMode::Crop);
        canvas->setPreviewMode(true);
    });
    // Add to toolbar
    tb = addToolBar("Tools");
    tb->addAction(moveAct);
    tb->addAction(selectAct);
    tb->addAction(resizeAct);
    tb->addAction(cropAct);
    tb->addAction(blackAndWhiteAct);
    tb->addAction(lightenAct);
    tb->addAction(purpleAct);
    tb->addAction(blurAct);
    tb->addAction(brightAct);
    tb->addAction(oilAct);
    tb->addAction(skewAct);
}

QWidget *MainWindow::createFilterSidePanel() {
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);

    QGroupBox *basicGroup = new QGroupBox("Basic Filters", contentWidget);
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    QPushButton *btnGrayscale = new QPushButton("Grayscale", basicGroup);
    connect(btnGrayscale, &QPushButton::clicked, this,
            [this]() { canvas->applyGrayScaleFilter(); });
    basicLayout->addWidget(btnGrayscale, 0, 0);

    QPushButton *btnBnW = new QPushButton("Black & White", basicGroup);
    connect(btnBnW, &QPushButton::clicked, this, [this]() {
        canvas->applyBlackAndWhiteFilter(128, CanvasWidget::FilterMode::Increment);
        canvas->commitChanges();
    });
    basicLayout->addWidget(btnBnW, 0, 1);

    QPushButton *btnInvert = new QPushButton("Invert Color", basicGroup);
    connect(btnInvert, &QPushButton::clicked, this,
            [this]() { canvas->applyInversionFilter(); });
    basicLayout->addWidget(btnInvert, 1, 0);

    QPushButton *btnRedScale = new QPushButton("Red Scale", basicGroup);
    connect(btnRedScale, &QPushButton::clicked, this,
            [this]() { canvas->applyInfraRedFilter(); });
    basicLayout->addWidget(btnRedScale, 1, 1);

    QPushButton *btnHFlip = new QPushButton("Horizontal Flip", basicGroup);
    connect(btnHFlip, &QPushButton::clicked, this,
            [this]() { canvas->applyHorizontalReflection(); });
    basicLayout->addWidget(btnHFlip, 2, 0);

    QPushButton *btnVFlip = new QPushButton("Veritical Flip", basicGroup);
    connect(btnVFlip, &QPushButton::clicked, this,
            [this]() { canvas->applyVeriticalReflection(); });
    basicLayout->addWidget(btnVFlip, 2, 1);

    QPushButton *btnEdges = new QPushButton("Edge Detection", basicGroup);
    connect(btnEdges, &QPushButton::clicked, this,
            [this]() { canvas->applyEdgeDetection(); });
    basicLayout->addWidget(btnEdges, 3, 0, 1, 2);

    QPushButton *btnNoise = new QPushButton("Tv Noise", basicGroup);
    connect(btnNoise, &QPushButton::clicked, this,
            [this]() { canvas->applyTvNoiseFilter(); });
    basicLayout->addWidget(btnNoise, 4, 0, 1, 2);
    layout->addWidget(basicGroup);

    QGroupBox *rotateGroup = new QGroupBox("Rotation", contentWidget);
    QVBoxLayout *rotateLayout = new QVBoxLayout(rotateGroup);
    rotateLayout->setSpacing(8);
    rotateLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *rotateControlLayout = new QHBoxLayout();

    rotateControlLayout->addWidget(new QLabel("Degrees:"));

    QSpinBox *rotateSpinInput = new QSpinBox();

    rotateSpinInput->setRange(0, 360);
    rotateSpinInput->setSingleStep(90);
    rotateSpinInput->setSuffix("°");
    rotateControlLayout->addWidget(rotateSpinInput, 1);
    rotateLayout->addLayout(rotateControlLayout);
    connect(rotateSpinInput, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int degrees) { canvas->applyRotateFilter(degrees); });

    layout->addWidget(rotateGroup);

    QGroupBox *brightDarkGroup = new QGroupBox("Brightness", contentWidget);

    QVBoxLayout *brightDarkColumn = new QVBoxLayout(brightDarkGroup);

    brightDarkColumn->setSpacing(8);
    brightDarkColumn->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *brightRow = new QHBoxLayout();
    brightRow->addWidget(new QLabel("Lighten:"));
    QSpinBox *brightBox = new QSpinBox();

    brightBox->setRange(0, 100);

    brightRow->addWidget(brightBox, 1);
    QPushButton *btnBright = new QPushButton("Apply", brightDarkGroup);
    connect(btnBright, &QPushButton::clicked, this, [this, brightBox]() {
        int percent = brightBox->value();
        canvas->applyLightOrDarkFilter(percent,
                                       CanvasWidget::FilterMode::Increment);
        canvas->commitChanges();
    });
    brightRow->addWidget(btnBright, 1);

    brightDarkColumn->addLayout(brightRow);

    QHBoxLayout *darkRow = new QHBoxLayout();
    darkRow->addWidget(new QLabel("Darken:"));
    QSpinBox *darkBox = new QSpinBox();

    darkBox->setRange(0, 100);

    darkRow->addWidget(darkBox, 1);
    QPushButton *btnDark = new QPushButton("Apply", brightDarkGroup);
    darkRow->addWidget(btnDark, 1);

    connect(btnDark, &QPushButton::clicked, this, [this, darkBox]() {
        int percent = darkBox->value();
        canvas->applyLightOrDarkFilter(-percent,
                                       CanvasWidget::FilterMode::Increment);
        canvas->commitChanges();
    });
    brightDarkColumn->addLayout(darkRow);

    layout->addWidget(brightDarkGroup);

    QGroupBox *blurGroup = new QGroupBox("Blur Effect", contentWidget);
    QVBoxLayout *blurLayout = new QVBoxLayout(blurGroup);
    blurLayout->setSpacing(8);
    blurLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *blurControlLayout = new QHBoxLayout();
    blurControlLayout->addWidget(new QLabel("Kernel:"));
    QSpinBox *blurSpinBox = new QSpinBox(blurGroup);
    blurSpinBox->setRange(3, 21);
    blurSpinBox->setSingleStep(2);
    blurSpinBox->setValue(5);
    blurControlLayout->addWidget(blurSpinBox, 1);
    blurLayout->addLayout(blurControlLayout);

    QPushButton *btnBlur = new QPushButton("Apply Blur", blurGroup);
    blurLayout->addWidget(btnBlur);

    connect(btnBlur, &QPushButton::clicked, this, [this, blurSpinBox]() {
        canvas->applyBlurFilter(blurSpinBox->value(),
                                CanvasWidget::FilterMode::Increment);
        canvas->commitChanges();
    });

    layout->addWidget(blurGroup);


    // Resize
    QGroupBox *resizeGroup = new QGroupBox("Resize Image", contentWidget);
    QGridLayout *resizeLayout = new QGridLayout(resizeGroup);
    resizeLayout->setSpacing(8);
    resizeLayout->setContentsMargins(10, 22, 10, 10);

    resizeLayout->addWidget(new QLabel("Width:"), 0, 0);
    QSpinBox *resizeWidthSpinBox = new QSpinBox(resizeGroup);
    resizeWidthSpinBox->setRange(5, 10000);
    resizeWidthSpinBox->setValue(800);
    resizeWidthSpinBox->setSuffix("px");
    resizeLayout->addWidget(resizeWidthSpinBox, 0, 1);

    resizeLayout->addWidget(new QLabel("Height:"), 1, 0);
    QSpinBox *resizeHeightSpinBox = new QSpinBox(resizeGroup);
    resizeHeightSpinBox->setRange(5, 10000);
    resizeHeightSpinBox->setValue(600);
    resizeHeightSpinBox->setSuffix("px");

    resizeLayout->addWidget(resizeHeightSpinBox, 1, 1);

    connect(resizeWidthSpinBox, &QSpinBox::valueChanged, this,
            [this](int val) { canvas->applyResizeTool(val, 0); });

    connect(resizeHeightSpinBox, &QSpinBox::valueChanged, this,
            [this](int val) { canvas->applyResizeTool(0, val); });

    layout->addWidget(resizeGroup);

    QGroupBox *panelContainer = new QGroupBox(this);
    QVBoxLayout *panelLayout = new QVBoxLayout(panelContainer);

    panelLayout->addWidget(scrollArea);

    layout->addStretch();

    scrollArea->setWidget(contentWidget);


    return panelContainer;
}

QWidget *MainWindow::createMorphPanel(){


    QScrollArea *morphScroll = new QScrollArea();
    morphScroll->setWidgetResizable(true);
    morphScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *morphContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(morphContent);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    // Target Image
    QGroupBox *targetGroup = new QGroupBox("Target Image", morphContent);
    QVBoxLayout *targetLayout = new QVBoxLayout(targetGroup);
    targetLayout->setSpacing(10);
    targetLayout->setContentsMargins(10, 22, 10, 10);

    QPushButton *btnLoadTarget = new QPushButton("Load Target Image", targetGroup);
    btnLoadTarget->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnLoadTarget, &QPushButton::clicked, this, [this](){
        loadTargetImage();
    });
    targetLayout->addWidget(btnLoadTarget);

    targetImageLabel = new QLabel("No target image loaded", targetGroup);
    targetImageLabel->setAlignment(Qt::AlignCenter);
    targetImageLabel->setMinimumHeight(140);
    targetImageLabel->setStyleSheet(
        "QLabel { "
        "border: 2px dashed #404040; "
        "background: #1a1a1a; "
        "color: #808080; "
        "border-radius: 6px; "
        "padding: 20px; }");
    targetLayout->addWidget(targetImageLabel);

    layout->addWidget(targetGroup);

    // Weights Image
    QGroupBox *weightsGroup = new QGroupBox("Weights Image (Optional)", morphContent);
    QVBoxLayout *weightsLayout = new QVBoxLayout(weightsGroup);
    weightsLayout->setSpacing(10);
    weightsLayout->setContentsMargins(10, 22, 10, 10);

   QPushButton* btnLoadWeights = new QPushButton("Load Weights Image", weightsGroup);
    btnLoadWeights->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
   connect(btnLoadWeights, &QPushButton::clicked, this,[this](){
        MainWindow::loadWeightsImage();
    });
    weightsLayout->addWidget(btnLoadWeights);

    weightsImageLabel = new QLabel("No weights image loaded\n(Uniform weights will be used)", weightsGroup);
    weightsImageLabel->setAlignment(Qt::AlignCenter);
    weightsImageLabel->setMinimumHeight(120);
    weightsImageLabel->setStyleSheet(
        "QLabel { "
        "border: 2px dashed #404040; "
        "background: #1a1a1a; "
        "color: #808080; "
        "border-radius: 6px; "
        "padding: 20px; }");
    weightsLayout->addWidget(weightsImageLabel);

    layout->addWidget(weightsGroup);

    // Blend Settings
    QGroupBox *blendGroup = new QGroupBox("Blend Settings", morphContent);
    QVBoxLayout *blendLayout = new QVBoxLayout(blendGroup);
    blendLayout->setSpacing(10);
    blendLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *blendControlLayout = new QHBoxLayout();
    blendControlLayout->addWidget(new QLabel("Blend Factor:"));
    QDoubleSpinBox *blendFactorSpinBox = new QDoubleSpinBox(blendGroup);
    blendFactorSpinBox->setRange(0.0, 1.0);
    blendFactorSpinBox->setSingleStep(0.1);
    blendFactorSpinBox->setValue(0.5);
    blendFactorSpinBox->setDecimals(2);


    connect(blendFactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [](){});
    blendControlLayout->addWidget(blendFactorSpinBox, 1);
    blendLayout->addLayout(blendControlLayout);

    QLabel *blendInfo = new QLabel("0.0 = Match target  |  1.0 = Keep source", blendGroup);
    blendInfo->setStyleSheet("QLabel { color: #808080; font-size: 9pt; font-style: italic; }");
    blendInfo->setWordWrap(true);
    blendLayout->addWidget(blendInfo);

    layout->addWidget(blendGroup);

    // Animation
    QGroupBox *animGroup = new QGroupBox("Animation", morphContent);
    QVBoxLayout *animLayout = new QVBoxLayout(animGroup);
    animLayout->setSpacing(10);
    animLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *frameLayout = new QHBoxLayout();
    frameLayout->addWidget(new QLabel("Frames:"));
    QSpinBox *animateFramesSpinBox = new QSpinBox(animGroup);
    animateFramesSpinBox->setRange(2, 100);
    animateFramesSpinBox->setValue(30);
    frameLayout->addWidget(animateFramesSpinBox, 1);
    animLayout->addLayout(frameLayout);

   QPushButton *btnMorphAnimated = new QPushButton("Create Animated GIF", animGroup);
    btnMorphAnimated->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
   connect(btnMorphAnimated, &QPushButton::clicked, this, [](){});
    animLayout->addWidget(btnMorphAnimated);

    layout->addWidget(animGroup);

    // Main Morph Button
    QPushButton *btnMorph = new QPushButton("Apply Morph", morphContent);
    btnMorph->setStyleSheet("QPushButton { background-color: #2563EB; font-weight: 600; padding: 16px; font-size: 11pt; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnMorph, &QPushButton::clicked, this, [](){});
    layout->addWidget(btnMorph);

    QGroupBox *morphContainer = new QGroupBox(this);
    QVBoxLayout *morphLayout = new QVBoxLayout(morphContainer);

    morphLayout->addWidget(morphScroll);

    layout->addStretch();

    morphScroll->setWidget(morphContent);

    return morphContainer;

}

QWidget *MainWindow::createMergePanel(){

    QGroupBox *mergePanel = new QGroupBox("Merge", this);

    QScrollArea *mergeScroll = new QScrollArea();
    mergeScroll->setWidgetResizable(true);
    mergeScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *mergeContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(mergeContent);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    // Target Image Load
    QGroupBox *targetGroup = new QGroupBox("Target Image (Image 2)", mergeContent);
    QVBoxLayout *targetLayout = new QVBoxLayout(targetGroup);
    targetLayout->setSpacing(10);
    targetLayout->setContentsMargins(10, 22, 10, 10);

    QPushButton *btnMergeTarget = new QPushButton("Load Target Image", targetGroup);
    connect(btnMergeTarget, &QPushButton::clicked, this,[this](){
        loadTargetMergeImage();
    });
    targetLayout->addWidget(btnMergeTarget);

    targetMergeLabel = new QLabel("No image loaded", targetGroup);
    targetMergeLabel->setAlignment(Qt::AlignCenter);
    targetMergeLabel->setWordWrap(true);
    targetMergeLabel->setMinimumHeight(100);
    targetMergeLabel->setStyleSheet(
        "QLabel { "
        "border: 2px dashed #404040; "
        "background: #1a1a1a; "
        "color: #808080; "
        "border-radius: 6px; "
        "padding: 20px; }");
    targetLayout->addWidget(targetMergeLabel);

    layout->addWidget(targetGroup);

    QGroupBox *outputGroup = new QGroupBox("Output",mergeContent);
    QVBoxLayout *previewLayout = new QVBoxLayout(outputGroup);
    outputMergeLabel = new QLabel("No Merge applied", targetGroup);
    outputMergeLabel->setAlignment(Qt::AlignCenter);
    outputMergeLabel->setWordWrap(true);
    outputMergeLabel->setMinimumHeight(100);
    outputMergeLabel->setStyleSheet(
        "QLabel { "
        "border: 2px dashed #404040; "
        "background: #1a1a1a; "
        "color: #808080; "
        "border-radius: 6px; "
        "padding: 20px; }");
    previewLayout->addWidget(outputMergeLabel);

    layout->addWidget(outputGroup);

    // Merge Controls
    QGroupBox *controlsGroup = new QGroupBox("Blend Controls", mergeContent);
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsGroup);
    controlsLayout->setSpacing(10);
    controlsLayout->setContentsMargins(10, 22, 10, 10);

    // Alpha / Blend Factor
    QHBoxLayout *alphaLayout = new QHBoxLayout();
    alphaLayout->addWidget(new QLabel("Blend Factor:"));
    QDoubleSpinBox *mergeBlendFactorSpinBox = new QDoubleSpinBox(controlsGroup);
    mergeBlendFactorSpinBox->setRange(0.0, 1.0);
    mergeBlendFactorSpinBox->setSingleStep(0.01);
    mergeBlendFactorSpinBox->setValue(0.5);
    mergeBlendFactorSpinBox->setDecimals(2);

    alphaLayout->addWidget(mergeBlendFactorSpinBox, 1);
    controlsLayout->addLayout(alphaLayout);

    QLabel *alphaInfo = new QLabel("0.0 = Only Image 2  |  1.0 = Only Image 1", controlsGroup);
    alphaInfo->setStyleSheet("QLabel { color: #808080; font-size: 9pt; font-style: italic; }");
    alphaInfo->setWordWrap(true);
    controlsLayout->addWidget(alphaInfo);

    // Merge Mode (Output Size)
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Output Size:"));
    QComboBox *mergeModeComboBox = new QComboBox(controlsGroup);
    mergeModeComboBox->addItem("Intersection (min)", QVariant('i'));
    mergeModeComboBox->addItem("Image 1 (main)", QVariant('f'));
    mergeModeComboBox->addItem("Image 2 (target)", QVariant('s'));

    modeLayout->addWidget(mergeModeComboBox, 1);
    controlsLayout->addLayout(modeLayout);


    auto applyMerge = [this, mergeBlendFactorSpinBox, mergeModeComboBox]() {

        double alpha = mergeBlendFactorSpinBox ->value();
        char mode = mergeModeComboBox-> currentData().toChar().toLatin1();

        applyMergeFilter(alpha,mode);
    };

    connect(mergeBlendFactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,applyMerge);
    connect(mergeModeComboBox, &QComboBox::currentTextChanged,
            this,applyMerge);
    layout->addWidget(controlsGroup);

    // Apply Button
    QPushButton *btnMerge = new QPushButton("Apply Merge", mergeContent);
    connect(btnMerge, &QPushButton::clicked, this, [this](){
        applyMergeFilter(0.5,'f');
    });
    layout->addWidget(btnMerge);

    layout->addStretch();

    mergeScroll->setWidget(mergeContent);

    QVBoxLayout *mergePanelLayout = new QVBoxLayout(mergePanel);
    mergePanelLayout->setContentsMargins(0, 0, 0, 0);
    mergePanelLayout->addWidget(mergeScroll);

    return mergePanel;

}

void MainWindow::openImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Image", QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (fileName.isEmpty())
        return;

    QImage img;
    if (!img.load(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to load image!");
        return;
    }

    canvas->setImage(img);
    setWindowTitle(
        QString("Mini Photoshop - %1").arg(QFileInfo(fileName).fileName()));
}

void MainWindow::saveImage() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Image", QString(), "PNG (*.png);;JPEG (*.jpg *.jpeg)");

    if (fileName.isEmpty())
        return;

    if (!canvas->image().save(fileName))
        QMessageBox::warning(this, "Error", "Could not save image!");
}

void MainWindow::applyModernStyle() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1a1a1a;
        }
        QGroupBox {
            font-weight: 600;
            font-size: 11pt;
            border: 1px solid #404040;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 18px;
            background-color: #252525;
            color: #e8e8e8;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 2px 10px;
            left: 10px;
            color: #e8e8e8;
        }
        QPushButton {
            background-color: #2563eb;
            color: white;
            border: none;
            padding: 10px 16px;
            border-radius: 5px;
            font-size: 10pt;
            font-weight: 500;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #3b82f6;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
        QPushButton:disabled {
            background-color: #404040;
            color: #808080;
        }
        QSpinBox, QDoubleSpinBox {
            background-color: #2d2d2d;
            color: #e8e8e8;
            border: 1px solid #404040;
            border-radius: 4px;
            padding: 6px 8px;
            font-size: 10pt;
            min-height: 28px;
            selection-background-color: #2563eb;
        }
        QSpinBox:focus, QDoubleSpinBox:focus {
            border: 1px solid #3b82f6;
        }
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            background-color: #404040;
            border-radius: 2px;
            width: 16px;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {
            background-color: #4a4a4a;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            background-color: #404040;
            border-radius: 2px;
            width: 16px;
        }
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #4a4a4a;
        }
        QLabel {
            color: #d0d0d0;
            font-size: 9.5pt;
        }
        QComboBox {
            background-color: #2d2d2d;
            color: #e8e8e8;
            border: 1px solid #404040;
            border-radius: 4px;
            padding: 6px 8px;
            min-height: 28px;
        }
        QComboBox:focus {
            border: 1px solid #3b82f6;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #d0d0d0;
            margin-right: 5px;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        QScrollBar:vertical {
            background-color: #1a1a1a;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #404040;
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #4a4a4a;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QToolBar {
            background-color: #252525;
            border-bottom: 1px solid #404040;
            spacing: 6px;
            padding: 8px;
        }
        QToolBar QPushButton {
            padding: 10px 24px;
            margin: 0px 4px;
            font-size: 11pt;
        }
        QMenuBar {
            background-color: #252525;
            color: #e8e8e8;
            border-bottom: 1px solid #404040;
            padding: 4px;
        }
        QMenuBar::item {
            padding: 8px 16px;
            background-color: transparent;
            border-radius: 4px;
        }
        QMenuBar::item:selected {
            background-color: #2563eb;
        }
        QMenu {
            background-color: #252525;
            color: #e8e8e8;
            border: 1px solid #404040;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item {
            padding: 8px 32px 8px 16px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: #2563eb;
        }
        QStatusBar {
            background-color: #252525;
            color: #d0d0d0;
            border-top: 1px solid #404040;
            padding: 4px;
        }
        QProgressBar {
            border: 1px solid #404040;
            border-radius: 4px;
            background-color: #2d2d2d;
            text-align: center;
            color: #e8e8e8;
            height: 20px;
        }
        QProgressBar::chunk {
            background-color: #2563eb;
            border-radius: 3px;
        }
        QTabWidget::pane {
            border: 1px solid #404040;
            border-radius: 6px;
            background-color: #1e1e1e;
            top: -1px;
        }
        QTabBar::tab {
            background-color: #2d2d2d;
            color: #b0b0b0;
            padding: 12px 24px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            border: 1px solid #404040;
            border-bottom: none;
        }
        QTabBar::tab:selected {
            background-color: #1e1e1e;
            color: #e8e8e8;
            border-bottom: 2px solid #2563eb;
        }
        QTabBar::tab:hover:!selected {
            background-color: #353535;
            color: #d0d0d0;
        }
    )");
}

void MainWindow::loadTargetImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Target Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            targetImage = std::make_unique<Image>(fileName.toStdString().c_str());

            QPixmap pixmap = imageToPixmap(*targetImage);
            targetImageLabel->setPixmap(pixmap.scaled(
                targetImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            targetImageLabel->setStyleSheet(
                "QLabel { border: 2px solid #7c3aed; background: #252525; border-radius: 6px; }");

        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load target image: %1").arg(e.what()));
        }
    }
}

void MainWindow::loadWeightsImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Weights Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            weightsImage = std::make_unique<Image>(fileName.toStdString().c_str());

            QPixmap pixmap = imageToPixmap(*weightsImage);
            weightsImageLabel->setPixmap(pixmap.scaled(
                weightsImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            weightsImageLabel->setStyleSheet(
                "QLabel { border: 2px solid #6366f1; background: #252525; border-radius: 6px; }");

        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load weights image: %1").arg(e.what()));
        }
    }
}

void MainWindow::loadTargetMergeImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Target Merge Image", "",
                                                    "Image Files (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty())
    {
        try
        {
            targetMergeImage = std::make_unique<Image>(fileName.toStdString().c_str());

            QPixmap pixmap = imageToPixmap(*targetMergeImage);
            targetMergeLabel->setPixmap(pixmap.scaled(
                targetMergeLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            targetMergeLabel->setStyleSheet(
                "QLabel { border: 2px solid #6366f1; background: #252525; border-radius: 6px; }");
        }
        catch (const std::exception &e)
        {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load target image: %1").arg(e.what()));
            targetMergeImage.reset();
            targetMergeLabel->setText("No image loaded");
            targetMergeLabel->setStyleSheet(
                "QLabel { "
                "border: 2px dashed #404040; "
                "background: #1a1a1a; "
                "color: #808080; "
                "border-radius: 6px; "
                "padding: 20px; }");
        }
    }
}


void MainWindow::applyMergeFilter(double alpha, char mode )
{
    if (canvas->isMImageNull())
    {
        QMessageBox::warning(this, "Warning", "Please load a main image first.");
        return;
    }

    if (!targetMergeImage)
    {
        QMessageBox::warning(this, "Warning", "Please load a target merge image first.");
        return;
    }

    try
    {

        // Create copies to pass to the merge function
        Image image1 = canvas->qImageToImage(canvas ->image());
        Image image2 = *targetMergeImage;


        Image outputImage;

        // Call the merge function
        merge(image1, image2, outputImage, alpha, mode);

        outputMergeImage = std::make_unique<Image>(outputImage);

        QPixmap pixmap = imageToPixmap(*outputMergeImage);
        outputMergeLabel->setPixmap(pixmap.scaled(
            outputMergeLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        outputMergeLabel->setStyleSheet(
            "QLabel { border: 2px solid #7c3aed; background: #252525; border-radius: 6px; }");


        // // Update the canvas display
        // canvas->setImage(imageToQImage(*currentImage));

    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Error", QString("Merge operation failed: %1").arg(e.what()));
    }
}


QPixmap MainWindow::imageToPixmap(const Image &img)
{
    QImage qImg(img.width, img.height, QImage::Format_RGB888);

    for (int row = 0; row < img.height; row++) {
        for (int col = 0; col < img.width; col++) {
            unsigned char r = img(col, row, 0);
            unsigned char g = img(col, row, 1);
            unsigned char b = img(col, row, 2);
            qImg.setPixel(col, row, qRgb(r, g, b));
        }
    }

    return QPixmap::fromImage(qImg);
}

void MainWindow::exitApp() { close(); }
