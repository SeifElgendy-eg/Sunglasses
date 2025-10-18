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

    // Enums for clarity
    enum class ToolMode { None, Move, Select, Resize, Crop };
    enum class FilterMode { Preview, Increment };

    // --- Public API ---
    void setImage(const QImage &img);
    void updateImage(const QImage &img);
    void setTool(ToolMode tool);
    const QImage &image() const { return m_image; }
    const QImage &originalImage() const { return o_image; }
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
    void resetView();

    // Filters and Tools
    void applyCrop();
    void applyCrop(int xs, int xe, int ys, int ye);  // Legacy API
    void applyGrayScaleFilter();
    void applyInversionFilter();
    void applyResizeTool(int newWidth, int newHeight);
    void applyVerticalReflection();
    void applyHorizontalReflection();
    void applyYellowFilter(int intensity, FilterMode mode);
    void applyPurpleFilter(int intensity, FilterMode mode);
    void applyInfraRedFilter();
    void applyBlackAndWhiteFilter(int threshold, FilterMode mode);
    void applyBlurFilter(int kernelSize, FilterMode mode);
    void applyLightOrDarkFilter(int percent, FilterMode mode);
    void applyEdgeDetection();
    void applyRotateFilter(int degrees);
    void applyFrameFilter(int thickness, int r, int g, int b);
    void applyTvNoiseFilter();
    void applyOilPaintFilter(int kernelSize, FilterMode mode);
    void applySkewFilter(double degree, FilterMode mode);
    void applyPixelSortFilter(int threshold, FilterMode mode);
    void applySharpenFilter();

    // Public members for legacy compatibility
    QRect m_cropRect;

signals:
    void previewModeChanged(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Drawing Helpers
    void drawCropTool(QPainter &painter);
    void drawResizeHandle(QPainter &painter);
    QPointF screenToImage(const QPointF &p_screen) const;

    // Handle detection for tools
    enum class Handle { None, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight, Move };
    Handle getHandleAt(const QPointF &p_image) const;
    void updateCursor(const QPoint& p_screen);
    void rotate(QImage &image, int degrees);

    // Member Variables
    QImage m_image;           // Current displayed image
    QImage o_image;           // Original resolution image
    QImage r_image;           // Reset reference image
    QImage m_baseImage;       // Base image for preview filters
    QImage o_baseImage;       // Base original for preview filters

    QStack<QPair<QImage, QImage>> m_undoStack;
    QStack<QPair<QImage, QImage>> m_redoStack;
    static const int MAX_UNDO_STEPS = 20;

    int m_lastRotation = 0;
    int start_x, end_x, start_y, end_y; // For filter region

    // View transformation (pan and zoom)
    qreal m_scale = 1.0;
    QPointF m_panOffset = {0, 0};
    QPointF m_panLastPos;
    bool m_isPanning = false;

    // Tool state
    ToolMode m_tool = ToolMode::None;
    bool m_previewMode = false;

    // Tool-specific data
    QRectF m_cropRectF;        // Floating point crop rect for new system
    QRectF m_selectionRect;

    bool m_isDraggingTool = false;
    Handle m_activeHandle = Handle::None;
    QPointF m_dragStartPos_image;

    // Background
    QBrush m_checkerboardBrush;
};

#endif
