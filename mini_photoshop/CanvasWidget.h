#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QImage>
#include <QRect>
#include <QPoint>
#include <QStack>
#include "Image_Class.h"

class MainWindow;
class CanvasWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
enum class ToolMode { None,Move,Select, Resize,Crop,Pan};
    enum class FilterMode {Preview, Increment};
    void addImage(const QImage &img);
    void setTool(ToolMode tool);
    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyResizeTool(int newWidth, int newHeight);
    void applyVeriticalReflection();
    void applyHorizontalReflection();
    void applyYellowFilter(const int intensity,FilterMode mode);
    void applyPurpleFilter(const int intensity,FilterMode mode);
    void applyInfraRedFilter();
    void applyBlackAndWhiteFilter(const int Threshold,FilterMode mode);
    void applyBlurFilter(int kernelSize, FilterMode mode);
    void applyLightOrDarkFilter(int percent,FilterMode mode );
    void applyEdgeDetection();
    void applyRotateFilter(int degrees);
    void applyFrameFilter(int thickness, int r, int g, int b);
    void applyTvNoiseFilter();
    void applyOilPaintFilter(int kernelSize, int levels,FilterMode mode);
    void applySkewFilter(double degree, FilterMode mode);
    void applyCrop(int xs,int xe,int ys, int ye);
    void updateCropHandles();
    void setActiveLayer(int index);
    bool isMImageNull();
    QImage& m_image();
    Image  qImageToImage(const QImage &qimg);
    QImage imageToQImage(const Image &img);
    QRect  m_cropRect;
    void resetImage();
    void commitChanges();
    void saveState();
    void cancelChanges();
    const QImage &image() const { return activeImage; }
    const QImage $image() const {return dump;}
    QRect canvasRect;
    QColor canvasBackgroundColor =Qt::white;
    QImage exportCanvas();
    bool showTransparency =false;
    struct Layer {
        QImage image;
        QImage o_image;
        QImage r_image;
        QPoint offset = QPoint(0, 0);
        bool visible = true;
        double opacity = 1.0;
        QStack<QPair<QImage, QImage>> undoStack;
        QStack<QPair<QImage, QImage>> redoStack;
    };

    QVector <Layer> images;
    int activeIndex;
    void undo();
    void redo();



    void moveResizeGroup();

    void moveCropGroup(int x, int x_2,int y, int y_2,char m);

    void setPreviewMode(bool enabled) {
        m_previewMode = enabled;
        emit previewModeChanged(enabled);
    }



signals:
    void previewModeChanged(bool enabled);
    void requestUpdateLayers();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event)override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void clearRedoStack();
    QPoint& offset();
    QImage& o_image();
    QPointF translateEventPosition(const QPointF &screenPoint)const;
    void  rotate(QImage &image, int degrees );
    QImage activeImage;

    void setActiveImage();
    QImage r_image;
    QImage dump;
    QImage m_baseImage;
    QImage o_baseImage;
    bool m_previewMode = false;

    double m_zoom = 1.0;
    QPointF m_panOffset = {0, 0};
    bool m_panning = false;
    QPoint m_lastPanPos;


    QPoint handleBottomCenter;
    QRect handleBottomRect;
    QPoint cropOffSet;

    QPoint handleUpperCenter;
    QRect handleUpperRect;

    QPoint handleRightCenter;
    QRect handleRightRect;

    QPoint handleLeftCenter;
    QRect handleLeftRect;

    QPoint resizeRightCenter;
    QRect  resizeHandleRightRect;

    QPoint resizeBottomCenter;
    QRect resizeHandleBottomRect;


    static const int MAX_UNDO_STEPS = 20;

    ToolMode m_tool = ToolMode::None;

    bool m_dragging = false;
    bool m_moving = false;


    QPoint m_start;
    QRect m_selection;

    QPoint m_offset = QPoint(0, 0);
    QPoint m_lastPos;

    double m_zoomFactor = 1.0;

    bool m_draggingHandle_x = false;
    bool m_draggingHandle_y = false;

    QRect m_selectionRect;
    QPoint m_dragStart;

    bool m_croppingHandle_x = false;
    bool m_croppingHandle_y = false;
    bool m_croppingHandle_x_2 = false;
    bool m_croppingHandle_y_2 = false;

    QRect  m_bottomHandle;
    QPoint m_cropStart;

    QPoint m_lastCrop;

    QRect  m_resizeRect;

    QRect checkRect;
    QRect interRect;

    int start_x,end_x;
    int start_y,end_y;
    int o_x_s;
    int o_x_e;
    int o_y_s;
    int o_y_e;

    int m_lastRotation =0;
};
