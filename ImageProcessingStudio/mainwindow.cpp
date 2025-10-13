#include "mainwindow.h"
#include "Image_Class.h"
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QApplication>
#include <QGroupBox>
#include <QSplitter>

extern void grayscale(Image &image);
extern void bnw(Image &image);
extern void invert(Image &image);
extern void reflect(Image &image);
extern void rotate(Image &image, int degrees);
extern void dnl(Image &image, int percent);
extern bool crop(Image &image, int x, int y, int width, int height);
extern bool frame(Image &image, int thickness, int r, int g, int b, char style);
extern void edges(Image &image);
extern void blur(Image &image, int kernelSize);
extern void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode);
extern Image resizeImageInMemory(Image &image, int newWidth, int newHeight);
extern void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor);
extern void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage,
                          const std::string &outputPath, int frameCount, double blendFactor);


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    updateStatusBar("Ready");
    resize(1400, 900);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Image Processing Studio");

    createMenuBar();
    createToolBar();
    createCentralWidget();
    createFilterPanel();
    createMorphPanel();
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *openAction = fileMenu->addAction("&Open Image");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onLoadImage);

    QAction *saveAction = fileMenu->addAction("&Save Image");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveImage);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar->addMenu("&Edit");

    QAction *resetAction = editMenu->addAction("&Reset to Original");
    resetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(resetAction, &QAction::triggered, this, &MainWindow::onResetImage);

    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About",
                           "Image Processing Studio\n\n"
                           "Team:\n"
                           "Marwan Mohamed Hassan (20240735)\n"
                           "Mohamed Talat Sayed (20240734)\n"
                           "Seifeldeen Hatem Moahmed (20242424)\n\n"
                           "Section: s12");
    });
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = new QToolBar(this);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QPushButton *btnOpen = new QPushButton("Open", this);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    toolBar->addWidget(btnOpen);

    QPushButton *btnSave = new QPushButton("Save", this);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImage);
    toolBar->addWidget(btnSave);

    toolBar->addSeparator();

    QPushButton *btnReset = new QPushButton("Reset", this);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::onResetImage);
    toolBar->addWidget(btnReset);

    toolBar->addSeparator();
    progressBar = new QProgressBar(this);
    progressBar->setMaximumWidth(200);
    progressBar->setVisible(false);
    toolBar->addWidget(progressBar);
}

void MainWindow::createCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #2b2b2b; border: 2px solid #555; }");
    imageLabel->setMinimumSize(400, 400);
    imageLabel->setText("No image loaded\n\nClick 'Open' or use File > Open Image");
    imageLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #aaa; font-size: 14pt; }");

    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(600);

    mainLayout->addWidget(scrollArea, 3);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    mainLayout->addWidget(splitter, 1);

    statusBar()->showMessage("Ready");
}

void MainWindow::createFilterPanel()
{
    filterPanel = new QGroupBox("Filters", this);
    QVBoxLayout *layout = new QVBoxLayout(filterPanel);

    QGroupBox *basicGroup = new QGroupBox("Basic Filters", filterPanel);
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    btnGrayscale = new QPushButton("Grayscale", basicGroup);
    connect(btnGrayscale, &QPushButton::clicked, this, &MainWindow::onGrayscale);
    basicLayout->addWidget(btnGrayscale, 0, 0);

    btnBnW = new QPushButton("Black & White", basicGroup);
    connect(btnBnW, &QPushButton::clicked, this, &MainWindow::onBlackAndWhite);
    basicLayout->addWidget(btnBnW, 0, 1);

    btnInvert = new QPushButton("Invert", basicGroup);
    connect(btnInvert, &QPushButton::clicked, this, &MainWindow::onInvert);
    basicLayout->addWidget(btnInvert, 1, 0);

    btnReflect = new QPushButton("Reflect", basicGroup);
    connect(btnReflect, &QPushButton::clicked, this, &MainWindow::onReflect);
    basicLayout->addWidget(btnReflect, 1, 1);

    btnEdges = new QPushButton("Edge Detection", basicGroup);
    connect(btnEdges, &QPushButton::clicked, this, &MainWindow::onEdges);
    basicLayout->addWidget(btnEdges, 2, 0, 1, 2);

    layout->addWidget(basicGroup);

    QGroupBox *rotateGroup = new QGroupBox("Rotate", filterPanel);
    QHBoxLayout *rotateLayout = new QHBoxLayout(rotateGroup);
    rotateSpinBox = new QSpinBox(rotateGroup);
    rotateSpinBox->setRange(0, 360);
    rotateSpinBox->setSingleStep(90);
    rotateSpinBox->setValue(90);
    rotateSpinBox->setSuffix("°");
    btnRotate = new QPushButton("Apply", rotateGroup);
    connect(btnRotate, &QPushButton::clicked, this, &MainWindow::onRotate);
    rotateLayout->addWidget(rotateSpinBox);
    rotateLayout->addWidget(btnRotate);
    layout->addWidget(rotateGroup);

    QGroupBox *brightnessGroup = new QGroupBox("Brightness", filterPanel);
    QGridLayout *brightnessLayout = new QGridLayout(brightnessGroup);

    brightnessLayout->addWidget(new QLabel("Lighten %:"), 0, 0);
    lightenSpinBox = new QSpinBox(brightnessGroup);
    lightenSpinBox->setRange(1, 200);
    lightenSpinBox->setValue(20);
    brightnessLayout->addWidget(lightenSpinBox, 0, 1);
    btnLighten = new QPushButton("Apply", brightnessGroup);
    connect(btnLighten, &QPushButton::clicked, this, &MainWindow::onLighten);
    brightnessLayout->addWidget(btnLighten, 0, 2);

    brightnessLayout->addWidget(new QLabel("Darken %:"), 1, 0);
    darkenSpinBox = new QSpinBox(brightnessGroup);
    darkenSpinBox->setRange(1, 200);
    darkenSpinBox->setValue(20);
    brightnessLayout->addWidget(darkenSpinBox, 1, 1);
    btnDarken = new QPushButton("Apply", brightnessGroup);
    connect(btnDarken, &QPushButton::clicked, this, &MainWindow::onDarken);
    brightnessLayout->addWidget(btnDarken, 1, 2);

    layout->addWidget(brightnessGroup);

    QGroupBox *blurGroup = new QGroupBox("Blur", filterPanel);
    QHBoxLayout *blurLayout = new QHBoxLayout(blurGroup);
    blurLayout->addWidget(new QLabel("Kernel Size:"));
    blurSpinBox = new QSpinBox(blurGroup);
    blurSpinBox->setRange(3, 21);
    blurSpinBox->setSingleStep(2);
    blurSpinBox->setValue(5);
    btnBlur = new QPushButton("Apply", blurGroup);
    connect(btnBlur, &QPushButton::clicked, this, &MainWindow::onBlur);
    blurLayout->addWidget(blurSpinBox);
    blurLayout->addWidget(btnBlur);
    layout->addWidget(blurGroup);

    QGroupBox *cropGroup = new QGroupBox("Crop", filterPanel);
    QGridLayout *cropLayout = new QGridLayout(cropGroup);

    cropLayout->addWidget(new QLabel("X:"), 0, 0);
    cropXSpinBox = new QSpinBox(cropGroup);
    cropXSpinBox->setRange(0, 10000);
    cropLayout->addWidget(cropXSpinBox, 0, 1);

    cropLayout->addWidget(new QLabel("Y:"), 0, 2);
    cropYSpinBox = new QSpinBox(cropGroup);
    cropYSpinBox->setRange(0, 10000);
    cropLayout->addWidget(cropYSpinBox, 0, 3);

    cropLayout->addWidget(new QLabel("Width:"), 1, 0);
    cropWidthSpinBox = new QSpinBox(cropGroup);
    cropWidthSpinBox->setRange(1, 10000);
    cropWidthSpinBox->setValue(500);
    cropLayout->addWidget(cropWidthSpinBox, 1, 1);

    cropLayout->addWidget(new QLabel("Height:"), 1, 2);
    cropHeightSpinBox = new QSpinBox(cropGroup);
    cropHeightSpinBox->setRange(1, 10000);
    cropHeightSpinBox->setValue(500);
    cropLayout->addWidget(cropHeightSpinBox, 1, 3);

    btnCrop = new QPushButton("Apply Crop", cropGroup);
    connect(btnCrop, &QPushButton::clicked, this, &MainWindow::onCrop);
    cropLayout->addWidget(btnCrop, 2, 0, 1, 4);

    layout->addWidget(cropGroup);

    QGroupBox *frameGroup = new QGroupBox("Frame", filterPanel);
    QGridLayout *frameLayout = new QGridLayout(frameGroup);

    frameLayout->addWidget(new QLabel("Thickness:"), 0, 0);
    frameThicknessSpinBox = new QSpinBox(frameGroup);
    frameThicknessSpinBox->setRange(1, 200);
    frameThicknessSpinBox->setValue(20);
    frameLayout->addWidget(frameThicknessSpinBox, 0, 1, 1, 3);

    frameLayout->addWidget(new QLabel("R:"), 1, 0);
    frameRSpinBox = new QSpinBox(frameGroup);
    frameRSpinBox->setRange(0, 255);
    frameLayout->addWidget(frameRSpinBox, 1, 1);

    frameLayout->addWidget(new QLabel("G:"), 1, 2);
    frameGSpinBox = new QSpinBox(frameGroup);
    frameGSpinBox->setRange(0, 255);
    frameLayout->addWidget(frameGSpinBox, 1, 3);

    frameLayout->addWidget(new QLabel("B:"), 2, 0);
    frameBSpinBox = new QSpinBox(frameGroup);
    frameBSpinBox->setRange(0, 255);
    frameLayout->addWidget(frameBSpinBox, 2, 1);

    btnFrame = new QPushButton("Apply Frame", frameGroup);
    connect(btnFrame, &QPushButton::clicked, this, &MainWindow::onFrame);
    frameLayout->addWidget(btnFrame, 2, 2, 1, 2);

    layout->addWidget(frameGroup);

    QGroupBox *resizeGroup = new QGroupBox("Resize", filterPanel);
    QGridLayout *resizeLayout = new QGridLayout(resizeGroup);

    resizeLayout->addWidget(new QLabel("Width:"), 0, 0);
    resizeWidthSpinBox = new QSpinBox(resizeGroup);
    resizeWidthSpinBox->setRange(1, 10000);
    resizeWidthSpinBox->setValue(800);
    resizeLayout->addWidget(resizeWidthSpinBox, 0, 1);

    resizeLayout->addWidget(new QLabel("Height:"), 1, 0);
    resizeHeightSpinBox = new QSpinBox(resizeGroup);
    resizeHeightSpinBox->setRange(1, 10000);
    resizeHeightSpinBox->setValue(600);
    resizeLayout->addWidget(resizeHeightSpinBox, 1, 1);

    btnResize = new QPushButton("Apply Resize", resizeGroup);
    connect(btnResize, &QPushButton::clicked, this, &MainWindow::onResize);
    resizeLayout->addWidget(btnResize, 2, 0, 1, 2);

    layout->addWidget(resizeGroup);

    layout->addStretch();

    QSplitter *mainSplitter = qobject_cast<QSplitter*>(
        centralWidget()->layout()->itemAt(1)->widget());
    if (mainSplitter) {
        mainSplitter->addWidget(filterPanel);
    }
}

void MainWindow::createMorphPanel()
{
    morphPanel = new QGroupBox("Morphing", this);
    QVBoxLayout *layout = new QVBoxLayout(morphPanel);

    QGroupBox *targetGroup = new QGroupBox("Target Image", morphPanel);
    QVBoxLayout *targetLayout = new QVBoxLayout(targetGroup);

    btnLoadTarget = new QPushButton("Load Target Image", targetGroup);
    connect(btnLoadTarget, &QPushButton::clicked, this, &MainWindow::onLoadTargetImage);
    targetLayout->addWidget(btnLoadTarget);

    targetImageLabel = new QLabel("No target image loaded", targetGroup);
    targetImageLabel->setAlignment(Qt::AlignCenter);
    targetImageLabel->setMinimumHeight(100);
    targetImageLabel->setStyleSheet("QLabel { border: 1px solid #ccc; background: #f0f0f0; }");
    targetLayout->addWidget(targetImageLabel);

    layout->addWidget(targetGroup);

    QGroupBox *weightsGroup = new QGroupBox("Weights Image (Optional)", morphPanel);
    QVBoxLayout *weightsLayout = new QVBoxLayout(weightsGroup);

    btnLoadWeights = new QPushButton("Load Weights Image", weightsGroup);
    connect(btnLoadWeights, &QPushButton::clicked, this, &MainWindow::onLoadWeightsImage);
    weightsLayout->addWidget(btnLoadWeights);

    weightsImageLabel = new QLabel("No weights image loaded\n(Uniform weights will be used)", weightsGroup);
    weightsImageLabel->setAlignment(Qt::AlignCenter);
    weightsImageLabel->setMinimumHeight(100);
    weightsImageLabel->setStyleSheet("QLabel { border: 1px solid #ccc; background: #f0f0f0; }");
    weightsLayout->addWidget(weightsImageLabel);

    layout->addWidget(weightsGroup);

    QGroupBox *blendGroup = new QGroupBox("Blend Settings", morphPanel);
    QVBoxLayout *blendLayout = new QVBoxLayout(blendGroup);

    QHBoxLayout *blendControlLayout = new QHBoxLayout();
    blendControlLayout->addWidget(new QLabel("Blend Factor:"));
    blendFactorSpinBox = new QDoubleSpinBox(blendGroup);
    blendFactorSpinBox->setRange(0.0, 1.0);
    blendFactorSpinBox->setSingleStep(0.1);
    blendFactorSpinBox->setValue(0.5);
    blendFactorSpinBox->setDecimals(2);
    connect(blendFactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onBlendFactorChanged);
    blendControlLayout->addWidget(blendFactorSpinBox);
    blendLayout->addLayout(blendControlLayout);

    QLabel *blendInfo = new QLabel("0.0 = Match target\n1.0 = Keep source", blendGroup);
    blendInfo->setStyleSheet("QLabel { color: #666; font-size: 9pt; }");
    blendLayout->addWidget(blendInfo);

    layout->addWidget(blendGroup);

    QGroupBox *animGroup = new QGroupBox("Animation", morphPanel);
    QVBoxLayout *animLayout = new QVBoxLayout(animGroup);

    QHBoxLayout *frameLayout = new QHBoxLayout();
    frameLayout->addWidget(new QLabel("Frames:"));
    animateFramesSpinBox = new QSpinBox(animGroup);
    animateFramesSpinBox->setRange(2, 100);
    animateFramesSpinBox->setValue(30);
    frameLayout->addWidget(animateFramesSpinBox);
    animLayout->addLayout(frameLayout);

    btnMorphAnimated = new QPushButton("Create Animated GIF", animGroup);
    btnMorphAnimated->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(btnMorphAnimated, &QPushButton::clicked, this, &MainWindow::onMorphAnimated);
    animLayout->addWidget(btnMorphAnimated);

    layout->addWidget(animGroup);

    btnMorph = new QPushButton("Apply Morph", morphPanel);
    btnMorph->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 10px; }");
    connect(btnMorph, &QPushButton::clicked, this, &MainWindow::onMorph);
    layout->addWidget(btnMorph);

    layout->addStretch();

    QSplitter *mainSplitter = qobject_cast<QSplitter*>(
        centralWidget()->layout()->itemAt(1)->widget());
    if (mainSplitter) {
        mainSplitter->addWidget(morphPanel);
    }
}

void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            currentImage = std::make_unique<Image>(fileName.toStdString().c_str());
            originalImage = std::make_unique<Image>(*currentImage);
            currentFilePath = fileName;

            updateImageDisplay();
            updateStatusBar("Image loaded: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onSaveImage()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Image",
                                                    QString(), "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)");

    if (!fileName.isEmpty()) {
        try {
            currentImage->saveImage(fileName.toStdString().c_str());
            updateStatusBar("Image saved: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to save image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onLoadTargetImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Target Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            targetImage = std::make_unique<Image>(fileName.toStdString().c_str());
            targetFilePath = fileName;

            QPixmap pixmap = imageToPixmap(*targetImage);
            targetImageLabel->setPixmap(pixmap.scaled(
                targetImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

            updateStatusBar("Target image loaded: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load target image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onLoadWeightsImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Weights Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            weightsImage = std::make_unique<Image>(fileName.toStdString().c_str());
            weightsFilePath = fileName;

            QPixmap pixmap = imageToPixmap(*weightsImage);
            weightsImageLabel->setPixmap(pixmap.scaled(
                weightsImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

            updateStatusBar("Weights image loaded: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load weights image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onGrayscale()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    grayscale(*currentImage);
    updateImageDisplay();
    updateStatusBar("Grayscale filter applied");
}

void MainWindow::onBlackAndWhite()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    bnw(*currentImage);
    updateImageDisplay();
    updateStatusBar("Black & White filter applied");
}

void MainWindow::onInvert()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    invert(*currentImage);
    updateImageDisplay();
    updateStatusBar("Invert filter applied");
}

void MainWindow::onReflect()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    reflect(*currentImage);
    updateImageDisplay();
    updateStatusBar("Reflect filter applied");
}

void MainWindow::onRotate()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int degrees = rotateSpinBox->value();
    rotate(*currentImage, degrees);
    updateImageDisplay();
    updateStatusBar(QString("Rotated %1 degrees").arg(degrees));
}

void MainWindow::onLighten()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int percent = lightenSpinBox->value();
    dnl(*currentImage, percent);
    updateImageDisplay();
    updateStatusBar(QString("Lightened by %1%").arg(percent));
}

void MainWindow::onDarken()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int percent = darkenSpinBox->value();
    dnl(*currentImage, -percent);
    updateImageDisplay();
    updateStatusBar(QString("Darkened by %1%").arg(percent));
}

void MainWindow::onCrop()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int x = cropXSpinBox->value();
    int y = cropYSpinBox->value();
    int w = cropWidthSpinBox->value();
    int h = cropHeightSpinBox->value();

    if (crop(*currentImage, x, y, w, h)) {
        updateImageDisplay();
        updateStatusBar(QString("Cropped to %1x%2 at (%3,%4)").arg(w).arg(h).arg(x).arg(y));
    } else {
        QMessageBox::warning(this, "Error", "Crop failed - check parameters");
    }
}

void MainWindow::onFrame()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int thickness = frameThicknessSpinBox->value();
    int r = frameRSpinBox->value();
    int g = frameGSpinBox->value();
    int b = frameBSpinBox->value();

    if (frame(*currentImage, thickness, r, g, b, 's')) {
        updateImageDisplay();
        updateStatusBar(QString("Frame added: %1px RGB(%2,%3,%4)").arg(thickness).arg(r).arg(g).arg(b));
    } else {
        QMessageBox::warning(this, "Error", "Frame failed - check parameters");
    }
}

void MainWindow::onEdges()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    edges(*currentImage);
    updateImageDisplay();
    updateStatusBar("Edge detection applied");
}

void MainWindow::onBlur()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int kernelSize = blurSpinBox->value();
    blur(*currentImage, kernelSize);
    updateImageDisplay();
    updateStatusBar(QString("Blur applied with kernel size %1").arg(kernelSize));
}

void MainWindow::onResize()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    int newWidth = resizeWidthSpinBox->value();
    int newHeight = resizeHeightSpinBox->value();

    try {
        *currentImage = resizeImageInMemory(*currentImage, newWidth, newHeight);
        updateImageDisplay();
        updateStatusBar(QString("Resized to %1x%2").arg(newWidth).arg(newHeight));
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Resize failed: %1").arg(e.what()));
    }
}

void MainWindow::onMerge()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    if (!mergeImage) {
        QMessageBox::warning(this, "Warning", "No merge image loaded");
        return;
    }

    float alpha = mergeAlphaSpinBox->value();
    QString mode = mergeModeCombo->currentText();
    char modeChar = mode[0].toLatin1();

    Image output;
    merge(*currentImage, *mergeImage, output, alpha, modeChar);
    *currentImage = output;

    updateImageDisplay();
    updateStatusBar(QString("Merged with alpha=%1, mode=%2").arg(alpha).arg(mode));
}

void MainWindow::onMorph()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    if (!targetImage) {
        QMessageBox::warning(this, "Warning", "No target image loaded");
        return;
    }

    if (!weightsImage) {
        weightsImage = std::make_unique<Image>(currentImage->width, currentImage->height);
        for (int row = 0; row < weightsImage->height; row++) {
            for (int col = 0; col < weightsImage->width; col++) {
                (*weightsImage)(col, row, 0) = 255;
                (*weightsImage)(col, row, 1) = 255;
                (*weightsImage)(col, row, 2) = 255;
            }
        }
    }

    double blendFactor = blendFactorSpinBox->value();

    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Indeterminate
    updateStatusBar("Morphing in progress...");

    QApplication::processEvents();

    try {
        morph(*currentImage, *targetImage, *weightsImage, blendFactor);
        updateImageDisplay();
        updateStatusBar("Morph completed successfully");
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Morph failed: %1").arg(e.what()));
        updateStatusBar("Morph failed");
    }

    progressBar->setVisible(false);
}

void MainWindow::onMorphAnimated()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    if (!targetImage) {
        QMessageBox::warning(this, "Warning", "No target image loaded");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Animated GIF",
                                                    "morph_animation.gif", "GIF (*.gif)");

    if (fileName.isEmpty()) {
        return;
    }

    if (!weightsImage) {
        weightsImage = std::make_unique<Image>(currentImage->width, currentImage->height);
        for (int row = 0; row < weightsImage->height; row++) {
            for (int col = 0; col < weightsImage->width; col++) {
                (*weightsImage)(col, row, 0) = 255;
                (*weightsImage)(col, row, 1) = 255;
                (*weightsImage)(col, row, 2) = 255;
            }
        }
    }

    double blendFactor = blendFactorSpinBox->value();
    int frames = animateFramesSpinBox->value();

    progressBar->setVisible(true);
    progressBar->setRange(0, 0);
    updateStatusBar("Creating animated GIF...");

    QApplication::processEvents();

    try {
        morphAnimated(*currentImage, *targetImage, *weightsImage,
                      fileName.toStdString(), frames, blendFactor);
        updateStatusBar("Animated GIF saved: " + fileName);
        QMessageBox::information(this, "Success",
                                 "Animated GIF created successfully!\n" + fileName);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Animation failed: %1").arg(e.what()));
        updateStatusBar("Animation failed");
    }

    progressBar->setVisible(false);
}

void MainWindow::onBlendFactorChanged(double value)
{
    updateStatusBar(QString("Blend factor: %1").arg(value, 0, 'f', 2));
}

void MainWindow::onResetImage()
{
    if (!originalImage) {
        QMessageBox::warning(this, "Warning", "No original image to reset to");
        return;
    }

    currentImage = std::make_unique<Image>(*originalImage);
    updateImageDisplay();
    updateStatusBar("Image reset to original");
}

void MainWindow::updateImageDisplay()
{
    if (!currentImage) {
        return;
    }

    QPixmap pixmap = imageToPixmap(*currentImage);

    QSize labelSize = scrollArea->viewport()->size();
    QPixmap scaled = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    imageLabel->setPixmap(scaled);
    imageLabel->resize(scaled.size());
}

void MainWindow::updateStatusBar(const QString &message)
{
    statusBar()->showMessage(message);
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
