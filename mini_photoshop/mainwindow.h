#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include "Image_Class.h"

class CanvasWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void updateLayers();

private slots:
    void addImage();
    void saveImage();
    void changeImage();
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
    QWidget* createLayersPanel();

    QVBoxLayout *layersLayout;
    QListWidget *layersWidget;

    QWidget *addLayer(int i);

    void loadTargetImage();
    void loadWeightsImage();
    void loadTargetMergeImage();

    QLabel *targetImageLabel;
    QLabel *weightsImageLabel;
    QLabel *targetMergeLabel;
    QLabel *outputMergeLabel;
    QLabel *outputMorphLabel;

    std::unique_ptr<Image> targetImage;
    std::unique_ptr<Image> weightsImage;
    std::unique_ptr<Image> outputMorphImage;
    std::unique_ptr<Image> targetMergeImage;
    std::unique_ptr<Image> outputMergeImage;


    QToolBar *tb;
    QMenuBar *menuBar;
    QPushButton *applyButton;
    QPushButton *cancelButton;
    QPixmap imageToPixmap(const Image &img);

    void applyMergeFilter (double alpha = 0.4, char mode ='f');
    void applyMorphFilter(double blendFactor);
    void applyMorphAnimated(double blendFactor,int frames);

    CanvasWidget *canvas = nullptr;
};
