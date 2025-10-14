#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QImage>
#include <QWidget>
#include <QStack>

class CanvasWidget : public QWidget {
    Q_OBJECT

public:
    enum class ToolMode { Select, Move, Resize };

    explicit CanvasWidget(QWidget *parent = nullptr);

    void setImage(const QImage &img);
    QImage image() const { return m_image; }

    void setTool(ToolMode tool);

    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyVeriticalReflection();
    void applyHorizontalReflection();
    void applyYellowFilter(const int intensity);
    void applyPurpleFilter(const int intensity);
    void applyInfraRedFilter();
    void applyBlackAndWhiteFilter();
    void applyBlurFilter(int kernelSize);
    void applyResizeTool(int newWidth, int newHeight);
    void applyEdgeDetection();
    void applyLightOrDarkFilter(int percent);

    void commitChanges();
    void cancelChanges();
    void resetImage();

    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void saveState();
    void clearRedoStack();

    QImage m_image;
    QImage o_image;
    QImage r_image;
    QImage m_baseImage;
    QImage o_baseImage;

    QStack<QPair<QImage, QImage>> m_undoStack;
    QStack<QPair<QImage, QImage>> m_redoStack;
    static const int MAX_UNDO_STEPS = 20;

    QPoint m_offset;
    ToolMode m_tool = ToolMode::Move;

    bool m_dragging = false;
    QPoint m_start;
    QRect m_selection;
    QRect interRect;
    QRect checkRect;

    int start_x = 0;
    int end_x = 0;
    int start_y = 0;
    int end_y = 0;

    bool m_moving = false;
    QPoint m_lastPos;

    QRect m_selectionRect;
    bool m_draggingHandle_x = false;
    bool m_draggingHandle_y = false;
    QPoint m_dragStart;
};

#endif
