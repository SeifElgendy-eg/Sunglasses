#include "MainWindow.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createMenu();
    createToolBar();
    applyModernStyle();
    canvas = new CanvasWidget(this);
    QWidget *filterPanel = createFilterSidePanel();

    QSplitter *splitter = new QSplitter(this);

    splitter->addWidget(canvas);
    splitter->addWidget(filterPanel);

    splitter->setSizes({750, 250});

    setCentralWidget(splitter);

    setWindowTitle("Mini Photoshop");
    resize(1000, 700);
}

void MainWindow::createMenu() {
    QMenuBar *menuBar = new QMenuBar(this);
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
    connect(resetAction, &QAction::triggered, this, &MainWindow::saveImage);

    QMenu *filtersMenu = menuBar->addMenu("&Filters");

    QAction *grayscaleAction = filtersMenu->addAction("&Grayscale");
    grayscaleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(grayscaleAction, &QAction::triggered, this,
            [this]() { canvas->applyGrayScaleFilter(); });

    QAction *bnwAction = filtersMenu->addAction("&Black && White");
    bnwAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    connect(bnwAction, &QAction::triggered, this,
            [this]() { canvas->applyBlackAndWhiteFilter(); });

    QAction *invertAction = filtersMenu->addAction("&Invert Colors");
    invertAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    connect(invertAction, &QAction::triggered, this,
            [this]() { canvas->applyInversionFilter(); });

    QAction *reflectVAction = filtersMenu->addAction("&Veritical Flip");
    connect(reflectVAction, &QAction::triggered, this,
            [this]() { canvas->applyVeriticalReflection(); });

    QAction *reflectHAction = filtersMenu->addAction("&Horizontal Flip");
    connect(reflectHAction, &QAction::triggered, this,
            [this]() { canvas->applyHorizontalReflection(); });

    filtersMenu->addSeparator();

    QAction *edgesAction = filtersMenu->addAction("&Edge Detection");
    connect(edgesAction, &QAction::triggered, this,
            [this]() { canvas->applyBlackAndWhiteFilter(); });

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
    moveAct->setToolTip("Select the image in canvas");
    moveAct->setCheckable(false);

    connect(moveAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Move); });



    QAction *lightenAct = new QAction("Birghten Image", this);
    lightenAct->setToolTip("Enhance reds and greens");
    lightenAct->setCheckable(false);

    connect(lightenAct, &QAction::triggered, this,
            [this]() { canvas->CanvasWidget::applyYellowFilter(20); });

    QAction *purpleAct = new QAction("Purple Image", this);
    purpleAct->setToolTip("Enhance reds and blues");
    purpleAct->setCheckable(false);

    connect(purpleAct, &QAction::triggered, this,
            [this]() { canvas->CanvasWidget::applyPurpleFilter(40); });


    QAction *blackAndWhiteAct = new QAction("Black and white image", this);
    blackAndWhiteAct->setToolTip("Make a classic image");
    blackAndWhiteAct->setCheckable(false);

    connect(blackAndWhiteAct, &QAction::triggered, this,
            [this]() { canvas->CanvasWidget::applyBlackAndWhiteFilter(); });

    QAction *blurAct = new QAction("Blur", this);
    blurAct->setToolTip("Make the image blury");
    blurAct->setCheckable(false);

    connect(blurAct, &QAction::triggered, this, [this]() {
        QDialog dialog(this);
        dialog.setWindowTitle("Adjust Blur");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Blur strength:", &dialog);

        slider->setRange(1, 15);
        slider->setValue(3);

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

        connect(slider, &QSlider::valueChanged, this,
                [this](int value) { canvas->applyBlurFilter(value); });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });


    QAction *brightAct = new QAction("brightness", this);
    brightAct->setToolTip("Control the brightness of the image");
    brightAct->setCheckable(false);


    connect(brightAct, &QAction::triggered, this, [this]() {
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

        connect(slider, &QSlider::valueChanged, this,
                [this](int value) { canvas->applyLightOrDarkFilter(value); });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    // Add to toolbar
    QToolBar *tb = addToolBar("Tools");
    tb->addAction(moveAct);
    tb->addAction(selectAct);
    tb->addAction(resizeAct);
    tb->addAction(blackAndWhiteAct);
    tb->addAction(lightenAct);
    tb->addAction(purpleAct);
    tb->addAction(blurAct);
    tb->addAction(brightAct);
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
    connect(btnBnW, &QPushButton::clicked, this,
            [this]() { canvas->applyBlackAndWhiteFilter(); });
    basicLayout->addWidget(btnBnW, 0, 1);

    QPushButton *btnInvert = new QPushButton("Invert Color", basicGroup);
    connect(btnInvert, &QPushButton::clicked, this,
            [this]() { canvas->applyInversionFilter(); });
    basicLayout->addWidget(btnInvert, 1, 0);

    QPushButton *btnRedScale = new QPushButton("Red Scale", basicGroup);
    connect(btnRedScale, &QPushButton::clicked, this,
            [this]() { canvas->applyInfraRedFilter(); });
    basicLayout->addWidget(btnRedScale, 1, 1);

    QPushButton *btnEdges = new QPushButton("Edge Detection", basicGroup);
    connect(btnEdges, &QPushButton::clicked, this,
            [this]() { canvas->applyEdgeDetection(); });
    basicLayout->addWidget(btnEdges, 2, 0, 1, 2);

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
    rotateSpinInput->setValue(90);
    rotateSpinInput->setSuffix("°");
    rotateControlLayout->addWidget(rotateSpinInput, 1);
    rotateLayout->addLayout(rotateControlLayout);


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
        canvas->applyLightOrDarkFilter(percent);
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
        canvas->applyLightOrDarkFilter(-percent);
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

    connect(btnBlur, &QPushButton::clicked, this, [this,blurSpinBox](){
        canvas ->applyBlurFilter(blurSpinBox ->value());
        canvas->commitChanges();
    });

    layout->addWidget(blurGroup);


    // Frame
    QGroupBox *frameGroup = new QGroupBox("Add Frame", contentWidget);
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
   QSpinBox  *frameGSpinBox = new QSpinBox(frameGroup);
    frameGSpinBox->setRange(0, 255);
    frameLayout->addWidget(frameGSpinBox, 1, 3);

    frameLayout->addWidget(new QLabel("B:"), 2, 0);
    QSpinBox *frameBSpinBox = new QSpinBox(frameGroup);
    frameBSpinBox->setRange(0, 255);
    frameLayout->addWidget(frameBSpinBox, 2, 1);

    QPushButton *btnFrame = new QPushButton("Apply Frame", frameGroup);

    frameLayout->addWidget(btnFrame, 2, 2, 1, 2);

    layout->addWidget(frameGroup);

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

    connect(resizeWidthSpinBox, &QSpinBox::valueChanged, this, [this](int val) {
        canvas->applyResizeTool(val,0);
    });

    connect(resizeHeightSpinBox, &QSpinBox::valueChanged, this, [this](int val) {
        canvas->applyResizeTool(0,val);
    });

    layout->addWidget(resizeGroup);

    QGroupBox *panelContainer = new QGroupBox(this);
    QVBoxLayout *panelLayout = new QVBoxLayout(panelContainer);
    panelLayout->addWidget(scrollArea);

    layout->addStretch();

    scrollArea->setWidget(contentWidget);

    QTabWidget *tabWidget = new QTabWidget();

    tabWidget->addTab(panelContainer, "Filters");

    tabWidget->setMinimumWidth(350);
    tabWidget->setMaximumWidth(500);

    return tabWidget;
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

void MainWindow::exitApp() { close(); }
