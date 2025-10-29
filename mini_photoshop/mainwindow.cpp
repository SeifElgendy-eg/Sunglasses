#include "MainWindow.h"
#include "CanvasWidget.h"
#include <omp.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QToolBar>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFrame>
#include <QListWidget>
#include <QShortcut>


extern void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode);
extern void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor);
extern void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage,
                          const std::string &outputPath, int frameCount, double blendFactor);


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createMenu();
    createToolBar();
    applyModernStyle();
    canvas = new CanvasWidget(this);

    connect(canvas,&CanvasWidget::requestUpdateLayers,this,&MainWindow::updateLayers);
    QWidget *filterPanel = createFilterSidePanel();
    QWidget *morphPanel  =  createMorphPanel();
    QWidget *mergePanel = createMergePanel();
    QWidget *layersPanel = createLayersPanel();

    tabWidget->addTab(filterPanel, "Filters");
    tabWidget->addTab(morphPanel, "Morph");
    tabWidget->addTab(mergePanel,"Merge");
    tabWidget->addTab(layersPanel,"Layers");
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
        QRect r = canvas->m_cropRect;
        canvas->applyCrop(r.left(), r.right(), r.top(), r.bottom());

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

    QAction *openAction = fileMenu->addAction("&Add Image");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::addImage);

    QAction *saveAction = fileMenu->addAction("&Save Active Image");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);

    QAction *changeAction = fileMenu->addAction("&Change Image");
    connect(changeAction, &QAction::triggered, this, [this](){
        changeImage();
        canvas->update();
    });

    QAction *exportAction = fileMenu->addAction("&Export Canvas");
    connect(exportAction, &QAction::triggered, this, [this]() {
        QImage finalImage = canvas->exportCanvas();
        QString filePath = QFileDialog::getSaveFileName(this, "Export Image", "", "PNG (*.png);;JPEG (*.jpg)");
        if (!filePath.isEmpty())
            finalImage.save(filePath);
    });
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

    QAction *sendLayerBack = editMenu->addAction("&Bring back");
    sendLayerBack->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft)); // Ctrl+[
    connect(sendLayerBack, &QAction::triggered, this, [this]() {
        if(canvas-> activeIndex !=0){
            std::swap( canvas ->images[canvas->activeIndex],canvas ->images[canvas->activeIndex-1]);
            canvas ->activeIndex =canvas->activeIndex-1;
            updateLayers();
            canvas->update();
        }
    });

    QAction *sendLayerFront = editMenu->addAction("&Bring Front");
    sendLayerFront->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight)); // Ctrl+]
    connect(sendLayerFront, &QAction::triggered, this, [this]() {
        if(canvas-> activeIndex !=canvas->images.size()-1){
            std::swap( canvas ->images[canvas->activeIndex],canvas ->images[canvas->activeIndex+1]);
            canvas ->activeIndex =canvas->activeIndex+1;
            updateLayers();
            canvas->update();
        }
    });

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

    QAction *lightenAct = filtersMenu->addAction("&Brighten");
    lightenAct->setToolTip("Adjust reds and greens");
    lightenAct->setCheckable(false);

    connect(lightenAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Tint");

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

    QAction *purpleAct = filtersMenu->addAction("&Tint");
    purpleAct->setToolTip("Adjust reds and blues");
    purpleAct->setCheckable(false);

    connect(purpleAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Tint");

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
    resizeAct ->setIcon(QIcon(":/icons/resize.svg"));
    resizeAct->setToolTip("Resize the image");
    resizeAct->setCheckable(false);

    connect(resizeAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Resize); });

    QAction *selectAct = new QAction("Select", this);
    selectAct ->setIcon(QIcon(":/icons/select.svg"));
    selectAct->setToolTip("Select pixels to modify");
    selectAct->setCheckable(false);

    connect(selectAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Select); });

    QAction *moveAct = new QAction("Move", this);
    moveAct->setToolTip("move the image freely in the canvas");
    moveAct ->setIcon(QIcon(":/icons/move.svg"));
    moveAct->setCheckable(false);
    connect(moveAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Move);
    });

    QAction *panAct = new QAction("Pan", this);
    panAct->setToolTip("Move the Camera");
    panAct ->setIcon(QIcon(":/icons/pan.svg"));
    panAct->setCheckable(false);
    connect(panAct, &QAction::triggered, this,
            [this]() { canvas->setTool(CanvasWidget::ToolMode::Pan);
            });


    QAction *blackAndWhiteAct = new QAction("Black and white image", this);
    blackAndWhiteAct ->setIcon(QIcon(":/icons/bnw.svg"));
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



    QAction *pixelSortAct = new QAction("Pixel Sort", this);
    pixelSortAct ->setIcon(QIcon(":/icons/sort.svg"));
    pixelSortAct->setToolTip("Sort pixels for a nice visual effect");
    pixelSortAct->setCheckable(false);

    connect(pixelSortAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Pixel Sorting");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Threshold:", &dialog);

        slider->setRange(0, 255);
        slider->setValue(128);

        canvas->applyPixelSort(slider->value(),CanvasWidget::FilterMode::Preview);
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
            canvas->applyPixelSort(value, CanvasWidget::FilterMode::Preview);
        });

        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });
    QAction *blurAct = new QAction("Blur", this);
    blurAct ->setIcon(QIcon(":/icons/blur.svg"));
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
    brightAct ->setIcon(QIcon(":/icons/brightness.svg"));
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
    oilAct ->setIcon(QIcon(":/icons/paint.svg"));
    oilAct->setToolTip("Simulate oil painting of the image");
    oilAct->setCheckable(false);

    connect(oilAct, &QAction::triggered, this, [this]() {
        canvas->saveState();
        QDialog dialog(this);
        dialog.setWindowTitle("Oil Piant");

        QVBoxLayout *layout = new QVBoxLayout(
            &dialog); // H,V BoxLayouts are a basically Rows and Columns
        QSlider *slider = new QSlider(Qt::Horizontal);
        QLabel *label = new QLabel("Kernel Size:", &dialog);

        slider->setRange(1,20);
        slider->setValue(0);

        QSlider *sliderL = new QSlider(Qt::Horizontal);
        QLabel *labelL = new QLabel("levels:", &dialog);

        sliderL->setRange(1,255);
        sliderL->setValue(0);

        QHBoxLayout *buttonsRow = new QHBoxLayout(this);

        QPushButton *applyBtn = new QPushButton("Apply", &dialog);
        QPushButton *canelBtn = new QPushButton("Cancel", &dialog);

        buttonsRow->addWidget(applyBtn);
        buttonsRow->addWidget(canelBtn);
        layout->addWidget(label);
        layout->addWidget(slider);
        layout->addWidget(labelL);
        layout->addWidget(sliderL);
        layout->addLayout(buttonsRow);

        connect(applyBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->commitChanges();
            dialog.accept();
        });

        connect(canelBtn, &QPushButton::clicked, &dialog, [this, &dialog]() {
            canvas->cancelChanges();
            dialog.reject();
        });

        auto previewPaint = [this, slider,sliderL](){
             canvas->applyOilPaintFilter(slider->value(), sliderL->value(),CanvasWidget::FilterMode::Preview);
        };
        connect(slider, &QSlider::valueChanged, this,previewPaint);
        connect(sliderL, &QSlider::valueChanged, this,previewPaint);
        connect(&dialog, &QDialog::rejected, this,
                [this]() { canvas->cancelChanges(); });
        dialog.exec();
    });

    QAction *skewAct = new QAction("Skew", this);
    skewAct ->setIcon(QIcon(":/icons/skew.svg"));
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
    cropAct ->setIcon(QIcon(":/icons/crop.svg"));
    cropAct ->setToolTip("Crop The Image");
    cropAct ->setCheckable(false);
    connect(cropAct, &QAction::triggered, this, [this](){
        canvas->saveState();
        canvas->setTool(CanvasWidget::ToolMode::Crop);
        canvas->setPreviewMode(true);
    });
    // Add to toolbar
    tb = addToolBar("Tools");
    addToolBar(Qt::LeftToolBarArea, tb);
    tb->addAction(moveAct);
    tb->addAction(panAct);
    tb->addAction(selectAct);
    tb->addAction(resizeAct);
    tb->addAction(cropAct);
    tb->addAction(blackAndWhiteAct);
    tb->addAction(blurAct);
    tb->addAction(brightAct);
    tb->addAction(oilAct);
    tb->addAction(skewAct);
    tb->addAction(pixelSortAct);
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

    QGroupBox *outputGroup = new QGroupBox("Output",morphContent);
    QVBoxLayout *previewLayout = new QVBoxLayout(outputGroup);
    outputMorphLabel = new QLabel("No Morph applied", targetGroup);
    outputMorphLabel->setAlignment(Qt::AlignCenter);
    outputMorphLabel->setWordWrap(true);
    outputMorphLabel->setMinimumHeight(100);
    outputMorphLabel->setStyleSheet(
        "QLabel { "
        "border: 2px dashed #404040; "
        "background: #1a1a1a; "
        "color: #808080; "
        "border-radius: 6px; "
        "padding: 20px; }");
    previewLayout->addWidget(outputMorphLabel);

    layout->addWidget(outputGroup);

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
   connect(btnMorphAnimated, &QPushButton::clicked, this, [this, blendFactorSpinBox,animateFramesSpinBox](){
        applyMorphAnimated(blendFactorSpinBox->value(),animateFramesSpinBox->value());
    });
    animLayout->addWidget(btnMorphAnimated);

    layout->addWidget(animGroup);

    // Main Morph Button
    QPushButton *btnMorph = new QPushButton("Apply Morph", morphContent);
    btnMorph->setStyleSheet("QPushButton { background-color: #2563EB; font-weight: 600; padding: 16px; font-size: 11pt; } QPushButton:hover { background-color: #3B82F6; }");
    connect(btnMorph, &QPushButton::clicked, this, [this,blendFactorSpinBox](){
        applyMorphFilter(blendFactorSpinBox->value());
    });
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
    QGroupBox *btnGroup =new QGroupBox("Options",mergeContent);
    QHBoxLayout *btnsLayout =new QHBoxLayout(btnGroup);
    QPushButton *btnLoad = new QPushButton("Load to canvas", mergeContent);
    connect(btnLoad, &QPushButton::clicked, this, [this](){
        if(!outputMergeImage){
            QMessageBox::warning(this, "Warning", "Please perform merge.");
            return;
        }
        canvas->saveState();
        canvas->addImage(canvas->imageToQImage(*outputMergeImage));
    });
    QPushButton *btnExport = new QPushButton("Export", mergeContent);
    connect(btnExport, &QPushButton::clicked, this, [this](){
        if(!outputMergeImage){
            QMessageBox::warning(this, "Warning", "Please perform merge.");
            return;
        }
        QString fileName = QFileDialog::getSaveFileName(
            this, "Save Image", QString(), "PNG (*.png);;JPEG (*.jpg *.jpeg)");


        if (!outputMergeImage->saveImage(fileName.toStdString()))
            QMessageBox::warning(this, "Error", "Could not save image!");

    });

    btnsLayout ->addWidget(btnLoad);
    btnsLayout ->addWidget(btnExport);
    layout->addWidget(btnGroup);

    layout->addStretch();

    mergeScroll->setWidget(mergeContent);

    QVBoxLayout *mergePanelLayout = new QVBoxLayout(mergePanel);
    mergePanelLayout->setContentsMargins(0, 0, 0, 0);
    mergePanelLayout->addWidget(mergeScroll);

    return mergePanel;

}

QWidget *MainWindow::createLayersPanel(){

    QWidget *layersPanel = new QWidget();
    layersLayout = new QVBoxLayout(layersPanel);
    layersWidget = new QListWidget();
    layersWidget->setFrameShape(QFrame::NoFrame);
    layersWidget->setSpacing(2);
    layersWidget->setUniformItemSizes(true);
    layersPanel->setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #f0f0f0;
            font-family: 'Segoe UI';
        }
        QPushButton {
            background-color: #3c3c3c;
            border: none;
            padding: 6px 10px;
            color: #e0e0e0;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton:pressed {
            background-color: #4a90e2;
            color: white;
        }
        QCheckBox {
            spacing: 5px;
        }
        QListWidget {
        background-color: #2b2b2b;
        border: none;
    }
QListWidget::item {
    background: transparent;
    border: none;
    margin: 0;
    padding: 0;
}
    QListWidget::item:selected {
        background-color: #4a90e2;
    }
    )");

    layersLayout->setSpacing(4);
    QPushButton *btnAddLayer =new QPushButton("+ Add Layer");
    btnAddLayer ->setIcon(QIcon::fromTheme("List Add"));
    btnAddLayer->setStyleSheet("font-weight: bold; background-color: #4a90e2; color: white;");
    layersLayout ->addWidget(btnAddLayer,0,Qt::AlignTop);
    layersLayout ->addWidget(layersWidget);
    connect(btnAddLayer , &QPushButton::clicked,this,[this](){
        addImage();
        updateLayers();
    });

    return layersPanel;
}

void MainWindow::updateLayers(){
    layersWidget ->clear();
    for(int i=0; i<canvas->images.size();i++){
        QListWidgetItem *item = new QListWidgetItem(QString("Layers %1").arg(i+1));
        item->setSizeHint(QSize(0, 60));

        layersWidget->insertItem(0,item);

        layersWidget->setItemWidget(item, addLayer(i));
    }

}

QWidget * MainWindow::addLayer (int i){
    const CanvasWidget::Layer &layerData = canvas->images[i];
    QWidget *layer= new QWidget();
    layer->setFixedHeight(60);
    layer->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 5px;
        }
    )").arg(i == canvas->activeIndex ? "#4a90e2" : "#2e2e2e"));


    QHBoxLayout *row = new QHBoxLayout(layer);
    row->setContentsMargins(8, 4, 8, 4);
    row->setSpacing(8);

    QLabel *thumbnail = new QLabel();
    QPixmap thumb = QPixmap::fromImage(layerData.image.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    thumbnail->setPixmap(thumb);
    thumbnail->setFixedSize(40, 40);
    thumbnail->setStyleSheet("border: 1px solid #555; background-color: #1e1e1e;");

    QLabel *name = new QLabel(QString("Layer %1").arg(i + 1));
    name->setStyleSheet("color: white; font-size: 12px;");

    QCheckBox *visible = new QCheckBox();
    visible ->setChecked(layerData.visible);
    connect(visible, &QCheckBox::toggled,this,[this,i](bool value){
        canvas->images[i].visible = value;
        canvas->update();
    });
    QPushButton *btnSelect = new QPushButton ("Select");
    btnSelect->setStyleSheet("background-color: #3a3a3a; color: white; padding: 4px 8px; border-radius: 3px;");
    connect(btnSelect,&QPushButton::clicked, this,[this,i](){
        canvas->setActiveLayer(i);
        updateLayers();
    });

    row ->addWidget(visible);
    row ->addWidget(thumbnail);
    row->addWidget(name);
    row->addStretch();
    row ->addWidget(btnSelect);

    return layer;
}
void MainWindow::addImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Add Image", QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (fileName.isEmpty())
        return;

    QImage img;
    if (!img.load(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to load image!");
        return;
    }

    canvas->addImage(img);
}

void MainWindow::changeImage(){
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Image", QString(), "Images (*.png *.jpg *.jpeg *.bmp)");

    if (fileName.isEmpty())
        return;
    QImage img;
    if (!img.load(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to load image!");
        return;
    }

    canvas-> images[canvas->activeIndex].image = img;
    canvas-> images[canvas->activeIndex].o_image = img;
    canvas-> images[canvas->activeIndex].r_image = img;
    updateLayers();
}
void MainWindow::saveImage() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Image", QString(), "PNG (*.png);;JPEG (*.jpg *.jpeg)");

    if (fileName.isEmpty())
        return;

    if (!canvas->m_image().save(fileName))
        QMessageBox::warning(this, "Error", "Could not save image!");
}

void MainWindow::applyModernStyle() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2b2b;
            color: #f0f0f0;
            font-family: 'Segoe UI';
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
            background-color: #3c3c3c;
            color: #e0e0e0;
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
        Image image1 = canvas->qImageToImage(canvas ->m_image());
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

void MainWindow::applyMorphFilter(double blendFactor)
{
    if (canvas->isMImageNull())
    {
        QMessageBox::warning(this, "Warning", "Please load a main image first.");
        return;
    }

    if (!targetImage)
    {
        QMessageBox::warning(this, "Warning", "Please load a target merge image first.");
        return;
    }
        Image image1 = canvas->qImageToImage(canvas ->m_image());
    if (!weightsImage) {
        weightsImage = std::make_unique<Image>(image1.width, image1.height);
        for (int row = 0; row < weightsImage->height; row++) {
            for (int col = 0; col < weightsImage->width; col++) {
                weightsImage->setPixel(col, row, 0, 255);
                weightsImage->setPixel(col, row, 1, 255);
                weightsImage->setPixel(col, row, 2, 255);
            }
        }
    }
    try {
        morph(image1, *targetImage, *weightsImage, blendFactor);
            outputMorphImage = std::make_unique<Image>(image1);
        QPixmap pixmap = imageToPixmap(*outputMorphImage);
        outputMorphLabel->setPixmap(pixmap.scaled(
            outputMorphLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        outputMorphLabel->setStyleSheet(
            "QLabel { border: 2px solid #7c3aed; background: #252525; border-radius: 6px; }");


    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Morph failed: %1").arg(e.what()));

    }


}

void MainWindow::applyMorphAnimated(double blendFactor,int frames){

        if (canvas->isMImageNull()) {
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
        Image image1 = canvas->qImageToImage(canvas ->m_image());
        if (!weightsImage) {
            weightsImage = std::make_unique<Image>(image1.width, image1.height);
            for (int row = 0; row < weightsImage->height; row++) {
                for (int col = 0; col < weightsImage->width; col++) {
                    weightsImage->setPixel(col, row, 0, 255);
                    weightsImage->setPixel(col, row, 1, 255);
                    weightsImage->setPixel(col, row, 2, 255);
                }
            }
        }

        try {

            Image sourceCopy = image1;
            morphAnimated(sourceCopy, *targetImage, *weightsImage,
                          fileName.toStdString(), frames, blendFactor);
            QMessageBox::information(this, "Success",
                                     "Animated GIF created successfully!\n" + fileName);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error",
                                  QString("Animation failed: %1").arg(e.what()));
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
