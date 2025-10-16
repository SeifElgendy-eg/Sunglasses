#include "mainwindow.h"
#include "Image_Class.h"
#include "CanvasWidget.h"
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
#include <QStackedWidget>
#include <QTabWidget>
#include <QScreen>
#include <QSpinBox>

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

// Forward declare the conversion function from CanvasWidget
extern QImage imageToQImage(const Image &img);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), canvas(nullptr)
{
    setupUI();
    applyModernStyle();
    updateStatusBar("Ready");

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = qMin(1600, static_cast<int>(screenGeometry.width() * 0.85));
    int height = qMin(1000, static_cast<int>(screenGeometry.height() * 0.85));
    resize(width, height);
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
    createMergePanel();
}

void MainWindow::applyModernStyle()
{
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

    QAction *undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &MainWindow::onUndo);

    QAction *redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &MainWindow::onRedo);

    QMenu *filtersMenu = menuBar->addMenu("&Filters");

    QAction *grayscaleAction = filtersMenu->addAction("&Grayscale");
    grayscaleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(grayscaleAction, &QAction::triggered, this, &MainWindow::onGrayscale);

    QAction *bnwAction = filtersMenu->addAction("&Black && White");
    bnwAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(bnwAction, &QAction::triggered, this, &MainWindow::onBlackAndWhite);

    QAction *invertAction = filtersMenu->addAction("&Invert Colors");
    invertAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    connect(invertAction, &QAction::triggered, this, &MainWindow::onInvert);

    QAction *reflectAction = filtersMenu->addAction("&Reflect");
    connect(reflectAction, &QAction::triggered, this, &MainWindow::onReflect);

    filtersMenu->addSeparator();

    QAction *edgesAction = filtersMenu->addAction("&Edge Detection");
    connect(edgesAction, &QAction::triggered, this, &MainWindow::onEdges);

    QAction *blurAction = filtersMenu->addAction("Bl&ur");
    connect(blurAction, &QAction::triggered, this, &MainWindow::onBlur);

    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
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

void MainWindow::createToolBar()
{
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(24, 24));
    toolBar->setMinimumHeight(64);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QPushButton *btnOpen = new QPushButton("Open", this);
    btnOpen->setMinimumSize(110, 40);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    toolBar->addWidget(btnOpen);

    QPushButton *btnSave = new QPushButton("Save", this);
    btnSave->setMinimumSize(110, 40);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveImage);
    toolBar->addWidget(btnSave);

    toolBar->addSeparator();

    QPushButton *btnReset = new QPushButton("Reset", this);
    btnReset->setMinimumSize(110, 40);
    btnReset->setStyleSheet("QPushButton { background-color: #dc2626; } QPushButton:hover { background-color: #ef4444; } QPushButton:pressed { background-color: #b91c1c; }");
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::onResetImage);
    toolBar->addWidget(btnReset);

    toolBar->addSeparator();

    QPushButton *btnSelectTool = new QPushButton("Select", this);
    btnSelectTool->setMinimumSize(110, 40);
    connect(btnSelectTool, &QPushButton::clicked, this, &MainWindow::onSelectTool);
    toolBar->addWidget(btnSelectTool);

    QPushButton *btnResizeTool = new QPushButton("Resize Tool", this);
    btnResizeTool->setMinimumSize(110, 40);
    connect(btnResizeTool, &QPushButton::clicked, this, &MainWindow::onResizeTool);
    toolBar->addWidget(btnResizeTool);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    progressBar = new QProgressBar(this);
    progressBar->setMinimumHeight(28);
    progressBar->setMaximumWidth(220);
    progressBar->setVisible(false);
    toolBar->addWidget(progressBar);
}

void MainWindow::createCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    canvas = new CanvasWidget(this);
    canvas->setMinimumSize(600, 500);
    canvas->setStyleSheet(
        "QWidget { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #1e1e1e, stop:1 #1a1a1a); "
        "border: none; "
        "}");

    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(canvas);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(600);

    mainLayout->addWidget(scrollArea, 3);

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setMinimumWidth(400);
    tabWidget->setMaximumWidth(450);

    mainLayout->addWidget(tabWidget);

    statusBar()->showMessage("Ready");
}

void MainWindow::createFilterPanel()
{
    filterPanel = new QGroupBox(this);

    QScrollArea *filterScroll = new QScrollArea();
    filterScroll->setWidgetResizable(true);
    filterScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *filterContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(filterContent);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    // Canvas Filters Group
    QGroupBox *canvasGroup = new QGroupBox("Reflection", filterContent);
    QGridLayout *canvasLayout = new QGridLayout(canvasGroup);
    canvasLayout->setSpacing(8);
    canvasLayout->setContentsMargins(10, 22, 10, 10);

    btnCanvasVertReflect = new QPushButton("V. Reflect", canvasGroup);
    connect(btnCanvasVertReflect, &QPushButton::clicked, this, &MainWindow::onCanvasVerticalReflection);
    canvasLayout->addWidget(btnCanvasVertReflect, 1, 0);

    btnCanvasHorzReflect = new QPushButton("H. Reflect", canvasGroup);
    connect(btnCanvasHorzReflect, &QPushButton::clicked, this, &MainWindow::onCanvasHorizontalReflection);
    canvasLayout->addWidget(btnCanvasHorzReflect, 1, 1);

    layout->addWidget(canvasGroup);

    // Canvas Color Filters
    QGroupBox *colorGroup = new QGroupBox("Color Filters", filterContent);
    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setSpacing(8);
    colorLayout->setContentsMargins(10, 22, 10, 10);

    colorLayout->addWidget(new QLabel("Intensity:"), 0, 0);
    canvasColorIntensity = new QSpinBox(colorGroup);
    canvasColorIntensity->setRange(0, 255);
    canvasColorIntensity->setValue(50);
    colorLayout->addWidget(canvasColorIntensity, 0, 1);

    btnCanvasYellow = new QPushButton("Yellow", colorGroup);
    connect(btnCanvasYellow, &QPushButton::clicked, this, &MainWindow::onCanvasYellowFilter);
    colorLayout->addWidget(btnCanvasYellow, 1, 0);

    btnCanvasPurple = new QPushButton("Purple", colorGroup);
    connect(btnCanvasPurple, &QPushButton::clicked, this, &MainWindow::onCanvasPurpleFilter);
    colorLayout->addWidget(btnCanvasPurple, 1, 1);

    btnCanvasInfraRed = new QPushButton("Infrared", colorGroup);
    connect(btnCanvasInfraRed, &QPushButton::clicked, this, &MainWindow::onCanvasInfraRedFilter);
    colorLayout->addWidget(btnCanvasInfraRed, 2, 0, 1, 2);

    layout->addWidget(colorGroup);

    // Canvas Undo/Redo
    QGroupBox *canvasUndoGroup = new QGroupBox("History", filterContent);
    QVBoxLayout *canvasUndoLayout = new QVBoxLayout(canvasUndoGroup);
    canvasUndoLayout->setSpacing(8);
    canvasUndoLayout->setContentsMargins(10, 22, 10, 10);

    btnCanvasUndo = new QPushButton("Undo", canvasUndoGroup);
    connect(btnCanvasUndo, &QPushButton::clicked, this, &MainWindow::onCanvasUndo);
    canvasUndoLayout->addWidget(btnCanvasUndo);

    btnCanvasRedo = new QPushButton("Redo", canvasUndoGroup);
    connect(btnCanvasRedo, &QPushButton::clicked, this, &MainWindow::onCanvasRedo);
    canvasUndoLayout->addWidget(btnCanvasRedo);

    btnCanvasReset = new QPushButton("Reset", canvasUndoGroup);
    btnCanvasReset->setStyleSheet("QPushButton { background-color: #dc2626; } QPushButton:hover { background-color: #ef4444; }");
    connect(btnCanvasReset, &QPushButton::clicked, this, &MainWindow::onCanvasReset);
    canvasUndoLayout->addWidget(btnCanvasReset);

    layout->addWidget(canvasUndoGroup);

    // Basic Filters
    QGroupBox *basicGroup = new QGroupBox("Basic Filters", filterContent);
    QGridLayout *basicLayout = new QGridLayout(basicGroup);
    basicLayout->setSpacing(8);
    basicLayout->setContentsMargins(10, 22, 10, 10);

    btnGrayscale = new QPushButton("Grayscale", basicGroup);
    connect(btnGrayscale, &QPushButton::clicked, this, &MainWindow::onGrayscale);
    basicLayout->addWidget(btnGrayscale, 0, 0);

    btnBnW = new QPushButton("B&W", basicGroup);
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

    // Rotation
    QGroupBox *rotateGroup = new QGroupBox("Rotation", filterContent);
    QVBoxLayout *rotateLayout = new QVBoxLayout(rotateGroup);
    rotateLayout->setSpacing(8);
    rotateLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *rotateControlLayout = new QHBoxLayout();
    rotateControlLayout->addWidget(new QLabel("Degrees:"));
    rotateSpinBox = new QSpinBox(rotateGroup);
    rotateSpinBox->setRange(0, 360);
    rotateSpinBox->setSingleStep(90);
    rotateSpinBox->setValue(90);
    rotateSpinBox->setSuffix("°");
    rotateControlLayout->addWidget(rotateSpinBox, 1);
    rotateLayout->addLayout(rotateControlLayout);

    btnRotate = new QPushButton("Apply Rotation", rotateGroup);
    connect(btnRotate, &QPushButton::clicked, this, &MainWindow::onRotate);
    rotateLayout->addWidget(btnRotate);

    layout->addWidget(rotateGroup);

    // Brightness
    QGroupBox *brightnessGroup = new QGroupBox("Brightness", filterContent);
    QVBoxLayout *brightnessLayout = new QVBoxLayout(brightnessGroup);
    brightnessLayout->setSpacing(8);
    brightnessLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *lightenLayout = new QHBoxLayout();
    lightenLayout->addWidget(new QLabel("Lighten:"));
    lightenSpinBox = new QSpinBox(brightnessGroup);
    lightenSpinBox->setRange(1, 200);
    lightenSpinBox->setValue(20);
    lightenSpinBox->setSuffix("%");
    lightenLayout->addWidget(lightenSpinBox, 1);
    btnLighten = new QPushButton("Apply", brightnessGroup);
    btnLighten->setMaximumWidth(70);
    connect(btnLighten, &QPushButton::clicked, this, &MainWindow::onLighten);
    lightenLayout->addWidget(btnLighten);
    brightnessLayout->addLayout(lightenLayout);

    QHBoxLayout *darkenLayout = new QHBoxLayout();
    darkenLayout->addWidget(new QLabel("Darken:"));
    darkenSpinBox = new QSpinBox(brightnessGroup);
    darkenSpinBox->setRange(1, 200);
    darkenSpinBox->setValue(20);
    darkenSpinBox->setSuffix("%");
    darkenLayout->addWidget(darkenSpinBox, 1);
    btnDarken = new QPushButton("Apply", brightnessGroup);
    btnDarken->setMaximumWidth(70);
    connect(btnDarken, &QPushButton::clicked, this, &MainWindow::onDarken);
    darkenLayout->addWidget(btnDarken);
    brightnessLayout->addLayout(darkenLayout);

    layout->addWidget(brightnessGroup);

    // Blur
    QGroupBox *blurGroup = new QGroupBox("Blur Effect", filterContent);
    QVBoxLayout *blurLayout = new QVBoxLayout(blurGroup);
    blurLayout->setSpacing(8);
    blurLayout->setContentsMargins(10, 22, 10, 10);

    QHBoxLayout *blurControlLayout = new QHBoxLayout();
    blurControlLayout->addWidget(new QLabel("Kernel:"));
    blurSpinBox = new QSpinBox(blurGroup);
    blurSpinBox->setRange(3, 21);
    blurSpinBox->setSingleStep(2);
    blurSpinBox->setValue(5);
    blurControlLayout->addWidget(blurSpinBox, 1);
    blurLayout->addLayout(blurControlLayout);

    btnBlur = new QPushButton("Apply Blur", blurGroup);
    connect(btnBlur, &QPushButton::clicked, this, &MainWindow::onBlur);
    blurLayout->addWidget(btnBlur);

    layout->addWidget(blurGroup);

    // Crop
    QGroupBox *cropGroup = new QGroupBox("Crop Image", filterContent);
    QGridLayout *cropLayout = new QGridLayout(cropGroup);
    cropLayout->setSpacing(8);
    cropLayout->setContentsMargins(10, 22, 10, 10);

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

    // Frame
    QGroupBox *frameGroup = new QGroupBox("Add Frame", filterContent);
    QGridLayout *frameLayout = new QGridLayout(frameGroup);
    frameLayout->setSpacing(8);
    frameLayout->setContentsMargins(10, 22, 10, 10);

    frameLayout->addWidget(new QLabel("Thickness:"), 0, 0);
    frameThicknessSpinBox = new QSpinBox(frameGroup);
    frameThicknessSpinBox->setRange(1, 200);
    frameThicknessSpinBox->setValue(20);
    frameThicknessSpinBox->setSuffix("px");
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

    // Resize
    QGroupBox *resizeGroup = new QGroupBox("Resize Image", filterContent);
    QGridLayout *resizeLayout = new QGridLayout(resizeGroup);
    resizeLayout->setSpacing(8);
    resizeLayout->setContentsMargins(10, 22, 10, 10);

    resizeLayout->addWidget(new QLabel("Width:"), 0, 0);
    resizeWidthSpinBox = new QSpinBox(resizeGroup);
    resizeWidthSpinBox->setRange(1, 10000);
    resizeWidthSpinBox->setValue(800);
    resizeWidthSpinBox->setSuffix("px");
    resizeLayout->addWidget(resizeWidthSpinBox, 0, 1);

    resizeLayout->addWidget(new QLabel("Height:"), 1, 0);
    resizeHeightSpinBox = new QSpinBox(resizeGroup);
    resizeHeightSpinBox->setRange(1, 10000);
    resizeHeightSpinBox->setValue(600);
    resizeHeightSpinBox->setSuffix("px");
    resizeLayout->addWidget(resizeHeightSpinBox, 1, 1);

    btnResize = new QPushButton("Apply Resize", resizeGroup);
    connect(btnResize, &QPushButton::clicked, this, &MainWindow::onResize);
    resizeLayout->addWidget(btnResize, 2, 0, 1, 2);

    layout->addWidget(resizeGroup);

    layout->addStretch();

    filterScroll->setWidget(filterContent);

    QVBoxLayout *filterPanelLayout = new QVBoxLayout(filterPanel);
    filterPanelLayout->setContentsMargins(0, 0, 0, 0);
    filterPanelLayout->addWidget(filterScroll);

    QTabWidget *tabWidget = centralWidget()->findChild<QTabWidget*>();
    if (tabWidget) {
        tabWidget->addTab(filterPanel, "Filters");
    }
}

void MainWindow::createMorphPanel()
{
    morphPanel = new QGroupBox(this);

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

    btnLoadTarget = new QPushButton("Load Target Image", targetGroup);
    btnLoadTarget->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnLoadTarget, &QPushButton::clicked, this, &MainWindow::onLoadTargetImage);
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

    btnLoadWeights = new QPushButton("Load Weights Image", weightsGroup);
    btnLoadWeights->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnLoadWeights, &QPushButton::clicked, this, &MainWindow::onLoadWeightsImage);
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
    blendFactorSpinBox = new QDoubleSpinBox(blendGroup);
    blendFactorSpinBox->setRange(0.0, 1.0);
    blendFactorSpinBox->setSingleStep(0.1);
    blendFactorSpinBox->setValue(0.5);
    blendFactorSpinBox->setDecimals(2);
    connect(blendFactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onBlendFactorChanged);
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
    animateFramesSpinBox = new QSpinBox(animGroup);
    animateFramesSpinBox->setRange(2, 100);
    animateFramesSpinBox->setValue(30);
    frameLayout->addWidget(animateFramesSpinBox, 1);
    animLayout->addLayout(frameLayout);

    btnMorphAnimated = new QPushButton("Create Animated GIF", animGroup);
    btnMorphAnimated->setStyleSheet("QPushButton { background-color: #2563EB; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnMorphAnimated, &QPushButton::clicked, this, &MainWindow::onMorphAnimated);
    animLayout->addWidget(btnMorphAnimated);

    layout->addWidget(animGroup);

    // Main Morph Button
    btnMorph = new QPushButton("Apply Morph", morphContent);
    btnMorph->setStyleSheet("QPushButton { background-color: #2563EB; font-weight: 600; padding: 16px; font-size: 11pt; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnMorph, &QPushButton::clicked, this, &MainWindow::onMorph);
    layout->addWidget(btnMorph);

    layout->addStretch();

    morphScroll->setWidget(morphContent);

    QVBoxLayout *morphPanelLayout = new QVBoxLayout(morphPanel);
    morphPanelLayout->setContentsMargins(0, 0, 0, 0);
    morphPanelLayout->addWidget(morphScroll);

    QTabWidget *tabWidget = centralWidget()->findChild<QTabWidget*>();
    if (tabWidget) {
        tabWidget->addTab(morphPanel, "Morph");
    }
}

void MainWindow::createMergePanel()
{
    mergePanel = new QGroupBox("Merge", this);

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

    btnMergeTarget = new QPushButton("Load Target Image", targetGroup);
    connect(btnMergeTarget, &QPushButton::clicked, this, &MainWindow::onLoadTargetMergeImage);
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

    // Merge Controls
    QGroupBox *controlsGroup = new QGroupBox("Blend Controls", mergeContent);
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsGroup);
    controlsLayout->setSpacing(10);
    controlsLayout->setContentsMargins(10, 22, 10, 10);

    // Alpha / Blend Factor
    QHBoxLayout *alphaLayout = new QHBoxLayout();
    alphaLayout->addWidget(new QLabel("Blend Factor:"));
    mergeBlendFactorSpinBox = new QDoubleSpinBox(controlsGroup);
    mergeBlendFactorSpinBox->setRange(0.0, 1.0);
    mergeBlendFactorSpinBox->setSingleStep(0.01);
    mergeBlendFactorSpinBox->setValue(0.5);
    mergeBlendFactorSpinBox->setDecimals(2);
    connect(mergeBlendFactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onMergeBlendFactorChanged);
    alphaLayout->addWidget(mergeBlendFactorSpinBox, 1);
    controlsLayout->addLayout(alphaLayout);

    QLabel *alphaInfo = new QLabel("0.0 = Only Image 2  |  1.0 = Only Image 1", controlsGroup);
    alphaInfo->setStyleSheet("QLabel { color: #808080; font-size: 9pt; font-style: italic; }");
    alphaInfo->setWordWrap(true);
    controlsLayout->addWidget(alphaInfo);

    // Merge Mode (Output Size)
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Output Size:"));
    mergeModeComboBox = new QComboBox(controlsGroup);
    mergeModeComboBox->addItem("Intersection (min)", QVariant('i'));
    mergeModeComboBox->addItem("Image 1 (main)", QVariant('f'));
    mergeModeComboBox->addItem("Image 2 (target)", QVariant('s'));
    connect(mergeModeComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::onMergeModeChanged);
    modeLayout->addWidget(mergeModeComboBox, 1);
    controlsLayout->addLayout(modeLayout);

    layout->addWidget(controlsGroup);

    // Apply Button
    btnMerge = new QPushButton("Apply Merge", mergeContent);
    connect(btnMerge, &QPushButton::clicked, this, &MainWindow::onMerge);
    layout->addWidget(btnMerge);

    layout->addStretch();

    mergeScroll->setWidget(mergeContent);

    QVBoxLayout *mergePanelLayout = new QVBoxLayout(mergePanel);
    mergePanelLayout->setContentsMargins(0, 0, 0, 0);
    mergePanelLayout->addWidget(mergeScroll);

    QTabWidget *tabWidget = centralWidget()->findChild<QTabWidget*>();
    if (tabWidget) {
        tabWidget->addTab(mergePanel, "Merge");
    }
}

// Canvas Widget Slot Methods
void MainWindow::onSelectTool()
{
    if (canvas) {
        canvas->setTool(CanvasWidget::ToolMode::Select);
        updateStatusBar("Select tool activated - Click and drag to select area");
    }
}

void MainWindow::onResizeTool()
{
    if (canvas) {
        canvas->setTool(CanvasWidget::ToolMode::Resize);
        updateStatusBar("Resize tool activated - Drag handles to resize image");
    }
}

void MainWindow::onCanvasVerticalReflection()
{
    if (canvas) {
        canvas->applyVeriticalReflection();
        updateStatusBar("Vertical reflection applied to canvas");
    }
}

void MainWindow::onCanvasHorizontalReflection()
{
    if (canvas) {
        canvas->applyHorizontalReflection();
        updateStatusBar("Horizontal reflection applied to canvas");
    }
}

void MainWindow::onCanvasYellowFilter()
{
    if (canvas) {
        canvas->applyYellowFilter(canvasColorIntensity->value());
        updateStatusBar("Yellow filter applied to canvas");
    }
}

void MainWindow::onCanvasPurpleFilter()
{
    if (canvas) {
        canvas->applyPurpleFilter(canvasColorIntensity->value());
        updateStatusBar("Purple filter applied to canvas");
    }
}

void MainWindow::onCanvasInfraRedFilter()
{
    if (canvas) {
        canvas->applyInfraRedFilter();
        updateStatusBar("Infrared filter applied to canvas");
    }
}

void MainWindow::onCanvasUndo()
{
    if (canvas) {
        canvas->undo();
        updateStatusBar("Undo applied");
    }
}

void MainWindow::onCanvasRedo()
{
    if (canvas) {
        canvas->redo();
        updateStatusBar("Redo applied");
    }
}

void MainWindow::onCanvasReset()
{
    if (canvas) {
        canvas->resetImage();
        updateStatusBar("Canvas reset to original");
    }
}

void MainWindow::onUndo()
{
    if (canvas) {
        canvas->undo();
        updateStatusBar("Undo applied");
    }
}

void MainWindow::onRedo()
{
    if (canvas) {
        canvas->redo();
        updateStatusBar("Redo applied");
    }
}

// Image loading and display
void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image",
                                                    QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        try {
            currentImage = std::make_unique<Image>(fileName.toStdString().c_str());
            originalImage = std::make_unique<Image>(*currentImage);
            currentFilePath = fileName;

            QImage qimg(fileName);
            if (!qimg.isNull()) {
                canvas->setImage(qimg);
            }

            updateStatusBar("Image loaded: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onSaveImage()
{
    if (!canvas || canvas->m_image.isNull()) {
        QMessageBox::warning(this, "Warning", "No image to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Image",
                                                    QString(), "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)");

    if (!fileName.isEmpty()) {
        try {
            if (canvas->m_image.save(fileName)) {
                // Update currentImage to match what was saved
                *currentImage = Image(fileName.toStdString().c_str());
                updateStatusBar("Image saved: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save image");
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to save image: %1").arg(e.what()));
        }
    }
}

void MainWindow::onResetImage()
{
    if (!originalImage) {
        QMessageBox::warning(this, "Warning", "No original image to reset to");
        return;
    }

    if (canvas) {
        canvas->resetImage();
        *currentImage = *originalImage;
        updateStatusBar("Image reset to original");
    }
}

// Basic filter operations
void MainWindow::onGrayscale()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    grayscale(*currentImage);
    canvas->setImage(imageToQImage(*currentImage));
    updateStatusBar("Grayscale filter applied");
}

void MainWindow::onBlackAndWhite()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    bnw(*currentImage);
    canvas->setImage(imageToQImage(*currentImage));
    updateStatusBar("Black & White filter applied");
}

void MainWindow::onInvert()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    invert(*currentImage);
    canvas->setImage(imageToQImage(*currentImage));
    updateStatusBar("Invert filter applied");
}

void MainWindow::onReflect()
{
    if (!currentImage) {
        QMessageBox::warning(this, "Warning", "No image loaded");
        return;
    }

    reflect(*currentImage);
    canvas->setImage(imageToQImage(*currentImage));
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
    canvas->setImage(imageToQImage(*currentImage));
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
    canvas->setImage(imageToQImage(*currentImage));
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
    canvas->setImage(imageToQImage(*currentImage));
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
        canvas->setImage(imageToQImage(*currentImage));
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
        canvas->setImage(imageToQImage(*currentImage));
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
    canvas->setImage(imageToQImage(*currentImage));
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
    canvas->setImage(imageToQImage(*currentImage));
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
        canvas->setImage(imageToQImage(*currentImage));
        updateStatusBar(QString("Resized to %1x%2").arg(newWidth).arg(newHeight));
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Resize failed: %1").arg(e.what()));
    }
}

// Morph operations
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
            targetImageLabel->setStyleSheet(
                "QLabel { border: 2px solid #7c3aed; background: #252525; border-radius: 6px; }");

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
            weightsImageLabel->setStyleSheet(
                "QLabel { border: 2px solid #6366f1; background: #252525; border-radius: 6px; }");

            updateStatusBar("Weights image loaded: " + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to load weights image: %1").arg(e.what()));
        }
    }
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
                weightsImage->setPixel(col, row, 0, 255);
                weightsImage->setPixel(col, row, 1, 255);
                weightsImage->setPixel(col, row, 2, 255);
            }
        }
    }

    double blendFactor = blendFactorSpinBox->value();

    progressBar->setVisible(true);
    progressBar->setRange(0, 0);
    updateStatusBar("⚙ Morphing in progress...");

    QApplication::processEvents();

    try {
        morph(*currentImage, *targetImage, *weightsImage, blendFactor);
        canvas->setImage(imageToQImage(*currentImage));
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
                weightsImage->setPixel(col, row, 0, 255);
                weightsImage->setPixel(col, row, 1, 255);
                weightsImage->setPixel(col, row, 2, 255);
            }
        }
    }

    double blendFactor = blendFactorSpinBox->value();
    int frames = animateFramesSpinBox->value();

    progressBar->setVisible(true);
    progressBar->setRange(0, 0);
    updateStatusBar("⚙ Creating animated GIF...");

    QApplication::processEvents();

    try {
        // Create a copy of currentImage for animation
        Image sourceCopy = *currentImage;
        morphAnimated(sourceCopy, *targetImage, *weightsImage,
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

// Merge functions
void MainWindow::onLoadTargetMergeImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Target Merge Image", "",
                                                    "Image Files (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty())
    {
        try
        {
            updateStatusBar("Loading target merge image...");
            targetMergeImage = std::make_unique<Image>(fileName.toStdString().c_str());

            QFileInfo fileInfo(fileName);
            targetMergeLabel->setText(fileInfo.fileName());
            targetMergeLabel->setStyleSheet(
                "QLabel { "
                "border: 2px dashed #404040; "
                "background: #1a1a1a; "
                "color: #e8e8e8; "
                "border-radius: 6px; "
                "padding: 20px; }");
            updateStatusBar("Target merge image loaded: " + fileName);
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
            updateStatusBar("Failed to load target merge image");
        }
    }
}

void MainWindow::onMerge()
{
    if (!currentImage)
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
        updateStatusBar("Applying Merge...");

        // Create copies to pass to the merge function
        Image image1 = *currentImage;
        Image image2 = *targetMergeImage;

        float alpha = static_cast<float>(mergeBlendFactorSpinBox->value());
        char mode = mergeModeComboBox->currentData().toChar().toLatin1();

        Image outputImage;

        // Call the merge function
        merge(image1, image2, outputImage, alpha, mode);

        // Update currentImage with the merged result
        *currentImage = outputImage;

        // Update the canvas display
        canvas->setImage(imageToQImage(*currentImage));

        updateStatusBar("Merge applied successfully.");
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Error", QString("Merge operation failed: %1").arg(e.what()));
        updateStatusBar("Merge failed");
    }
}

void MainWindow::onMergeBlendFactorChanged(double value)
{
    updateStatusBar(QString("Merge Blend Factor: %1 (1.0=Image1, 0.0=Image2)").arg(value, 0, 'f', 2));
}

void MainWindow::onMergeModeChanged(const QString &text)
{
    Q_UNUSED(text);
    char mode = mergeModeComboBox->currentData().toChar().toLatin1();
    QString modeStr;
    if (mode == 'i') modeStr = "Intersection (min size)";
    else if (mode == 'f') modeStr = "Image 1 (main image)";
    else if (mode == 's') modeStr = "Image 2 (target image)";

    updateStatusBar(QString("Merge Output Size Mode: %1").arg(modeStr));
}

// Utility functions
void MainWindow::updateImageDisplay()
{
    if (!currentImage) {
        return;
    }

    if (canvas) {
        canvas->setImage(imageToQImage(*currentImage));
    }
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
            unsigned char r = img.getPixel(col, row, 0);
            unsigned char g = img.getPixel(col, row, 1);
            unsigned char b = img.getPixel(col, row, 2);
            qImg.setPixel(col, row, qRgb(r, g, b));
        }
    }

    return QPixmap::fromImage(qImg);
}
