#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QImage>
#include <QWidget>
#include <QStack>
#include <QPair>

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    enum class ToolMode { Select, Move, Resize };  // Keep this one

    explicit CanvasWidget(QWidget *parent = nullptr);

    // Image management
    void setImage(const QImage &img);
    QImage image() const { return m_image; }

    // Tool management
    void setTool(ToolMode tool);

    // Filter operations - Basic
    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyVeriticalReflection();
    void applyHorizontalReflection();
    void applyBlackAndWhiteFilter();
    void applyEdgeDetection();

    // Filter operations - Color
    void applyYellowFilter(const int intensity);
    void applyPurpleFilter(const int intensity);
    void applyInfraRedFilter();

    // Filter operations - Effects
    void applyBlurFilter(int kernelSize);
    void applyLightOrDarkFilter(int percent);

    // Utility operations
    void applyResizeTool(int newWidth, int newHeight);

    // State management
    void commitChanges();
    void cancelChanges();
    void resetImage();

    // History management
    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

    // Selection bounds helper
    void getSelectionBounds(int &x1, int &x2, int &y1, int &y2) const {
        x1 = start_x; x2 = end_x; y1 = start_y; y2 = end_y;
    }

    // Public members accessible from MainWindow
    QImage m_image;
    QImage o_image;
    QRect interRect;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // State management
    void saveState();
    void clearRedoStack();

    // Image data
    QImage r_image;        // Reset/original image
    QImage m_baseImage;    // Base image for filters
    QImage o_baseImage;    // Original base image

    // Undo/Redo history
    QStack<QPair<QImage, QImage>> m_undoStack;
    QStack<QPair<QImage, QImage>> m_redoStack;
    static const int MAX_UNDO_STEPS = 20;

    // Canvas state
    QPoint m_offset;
    ToolMode m_tool = ToolMode::Move;

    // Selection state
    bool m_dragging = false;
    QPoint m_start;
    QRect m_selection;
    QRect checkRect;

    // Selection bounds
    int start_x = 0;
    int end_x = 0;
    int start_y = 0;
    int end_y = 0;

    // Movement state
    bool m_moving = false;
    QPoint m_lastPos;

    // Resize tool state
    QRect m_selectionRect;
    bool m_draggingHandle_x = false;
    bool m_draggingHandle_y = false;
    QPoint m_dragStart;
};

#endif // CANVASWIDGET_H
