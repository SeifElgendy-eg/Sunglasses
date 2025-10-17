#include <QWidget>
#include <QImage>
#include <QRect>
#include <QPoint>
#include <QStack>
#include "Image_Class.h"

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
enum class ToolMode { None,Move,Select, Resize,Crop};
    enum class FilterMode {Preview, Increment};
    void setImage(const QImage &img);
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
    void applyOilPaintFilter(int kernelSize, FilterMode mode);
    void applySkewFilter(double degree, FilterMode mode);
    void applyCrop(int xs,int xe,int ys, int ye);
    bool isMImageNull();
    Image  qImageToImage(const QImage &qimg);
    QImage imageToQImage(const Image &img);
    QRect  m_cropRect;
    void resetImage();
    void commitChanges();
    void saveState();
    void cancelChanges();
    const QImage &image() const { return m_image; }
    const QImage $image() const {return o_image;}

    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }


    void moveResizeGroup();

    void moveCropGroup(int x, int x_2,int y, int y_2,char m);

    void setPreviewMode(bool enabled) {
        m_previewMode = enabled;
        emit previewModeChanged(enabled);
    }



signals:
    void previewModeChanged(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void clearRedoStack();
    void  rotate(QImage &image, int degrees );
    QImage m_image;
    QImage o_image;
    QImage r_image;
    QImage m_baseImage;
    QImage o_baseImage;
    bool m_previewMode = false;

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


    QStack<QPair<QImage, QImage>> m_undoStack;
    QStack<QPair<QImage, QImage>> m_redoStack;

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

    QRect  m_resizeRect;

    QRect checkRect;
    QRect interRect;

    int start_x,end_x;
    int start_y,end_y;
    int m_lastRotation =0;
};
