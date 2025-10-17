#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QStack>
#include <QWidget>
#include <QPair>

#include "Image_Class.h"

class CanvasWidget : public QWidget {
    Q_OBJECT

public:
    explicit CanvasWidget(QWidget *parent = nullptr);

    enum class ToolMode { None, Move, Select, Resize, Crop };
    enum class FilterMode { Preview, Increment };

    // Public API
    void setImage(const QImage &img);
    void setTool(ToolMode tool);
    const QImage &image() const { return m_image; }
    void setPreviewMode(bool enabled);
    bool isMImageNull();
    Image qImageToImage(const QImage &qimg);
    QImage imageToQImage(const Image &img);

    // Undo/Redo & State Management
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    void saveState();
    void commitChanges();
    void cancelChanges();
    void resetImage();

    // Filters and Tools (Restored from your original code)
    void applyCrop();
    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyResizeTool(int newWidth, int newHeight);
    void applyVeriticalReflection();
    void applyHorizontalReflection();
    void applyYellowFilter(const int intensity, FilterMode mode = FilterMode::Increment);
    void applyPurpleFilter(const int intensity, FilterMode mode = FilterMode::Increment);
    void applyInfraRedFilter();
    void applyBlackAndWhiteFilter(int threshold, FilterMode mode = FilterMode::Increment);
    void applyBlurFilter(int kernelSize, FilterMode mode = FilterMode::Increment);
    void applyEdgeDetection();
    void applyLightOrDarkFilter(int percent, FilterMode mode = FilterMode::Increment);
    void applyRotateFilter(int degrees);
    void applyFrameFilter(int thickness, int r, int g, int b);
    void applyTvNoiseFilter();
    void applyOilPaintFilter(int kernelSize, FilterMode mode = FilterMode::Increment);
    void applySkewFilter(double degree, FilterMode mode = FilterMode::Increment);

    // Public member for mainwindow to access and modify
    QRectF m_cropRect;

signals:
    // Signal for the main window to react to UI changes (Restored)
    void previewModeChanged(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Drawing Helpers
    void drawCropTool(QPainter &painter);
    QPointF screenToImage(const QPointF &p_screen) const;

    // Handle detection for tools
    enum class Handle { None, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight, Move };
    Handle getHandleAt(const QPointF &p_image) const;
    void updateCursor(const QPoint& p_screen);
    void resetView();
    void rotate(QImage &image, int degrees);

    // Member Variables
    QImage m_image;
    QImage o_image;
    QImage r_image;
    QImage m_baseImage;
    QImage o_baseImage;
    QStack<QPair<QImage, QImage>> m_undoStack;
    QStack<QPair<QImage, QImage>> m_redoStack;
    static const int MAX_UNDO_STEPS = 20;
    int m_lastRotation = 0;
    int start_x, end_x, start_y, end_y; // For filter region

    qreal m_scale = 1.0;
    QPointF m_panOffset;

    ToolMode m_tool = ToolMode::None;
    QPointF m_panLastPos;
    bool m_isPanning = false;
    bool m_isDraggingTool = false;
    QPointF m_dragStartPos_image;
    Handle m_activeHandle = Handle::None;
    bool m_previewMode = false;
    QBrush m_checkerboardBrush;
    QRectF m_selectionRect;
};

#endif
