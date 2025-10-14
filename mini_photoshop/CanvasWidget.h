#include <QWidget>
#include <QImage>
#include <QRect>
#include <QPoint>



class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
enum class ToolMode { None,Move,Select, Resize};
    void setImage(const QImage &img);
    void setTool(ToolMode tool);
    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyResizeTool(int newWidth, int newHeight);
    void applyVeriticalReflection();
    void applyHorizontalReflection();
    void applyYellowFilter(const int intensity);
    void applyPurpleFilter(const int intensity);
    void applyInfraRedFilter();
    void applyBlackAndWhiteFilter();
    void applyBlurFilter(int kernelSize);
    void applyLightOrDarkFilter(int percent);
    void applyEdgeDetection();
    void commitChanges();
    void cancelChanges();
    const QImage &image() const { return m_image; }
    const QImage $image() const {return o_image;}


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QImage m_image;
    QImage o_image;
    QImage m_baseImage;
    QImage o_baseImage;

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


    QRect checkRect;
    QRect interRect;

    int start_x,end_x;
    int start_y,end_y;
};
