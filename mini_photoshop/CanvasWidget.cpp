#include "CanvasWidget.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QtMath>
#include <random>
#include <vector>

// Constants for modern dark theme
const QColor WIDGET_BACKGROUND_COLOR = QColor("#1e1e1e");
const QColor CHECKERBOARD_COLOR_1 = QColor("#2b2b2b");
const QColor CHECKERBOARD_COLOR_2 = QColor("#3a3a3a");
const int CHECKERBOARD_TILE_SIZE = 20;

// ============================================================================
// Constructor & Setup
// ============================================================================

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);

    // Create checkerboard pattern for transparent background
    QPixmap tile(CHECKERBOARD_TILE_SIZE * 2, CHECKERBOARD_TILE_SIZE * 2);
    tile.fill(Qt::transparent);
    QPainter p(&tile);
    p.fillRect(0, 0, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_COLOR_1);
    p.fillRect(CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_COLOR_1);
    p.fillRect(CHECKERBOARD_TILE_SIZE, 0, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_COLOR_2);
    p.fillRect(0, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_TILE_SIZE, CHECKERBOARD_COLOR_2);
    m_checkerboardBrush = QBrush(tile);
}

void CanvasWidget::setImage(const QImage &img) {
    m_undoStack.clear();
    m_redoStack.clear();
    m_image = img;
    o_image = img;
    r_image = img;

    start_x = 0;
    start_y = 0;
    end_x = m_image.width();
    end_y = m_image.height();

    resetView();
}

void CanvasWidget::updateImage(const QImage &img) {
    m_image = img;
    o_image = img;

    start_x = 0;
    start_y = 0;
    end_x = m_image.width();
    end_y = m_image.height();

    update();
}

void CanvasWidget::setTool(ToolMode tool) {
    if (m_tool == tool) return;
    m_tool = tool;
    m_isDraggingTool = false;
    m_activeHandle = Handle::None;

    if (tool == ToolMode::Crop) {
        m_cropRectF = m_image.rect();
        m_cropRect = m_image.rect();
        setPreviewMode(true);
    } else {
        setPreviewMode(false);
    }
    updateCursor(this->mapFromGlobal(QCursor::pos()));
    update();
}

void CanvasWidget::resetView() {
    if (m_image.isNull()) return;
    m_panOffset = m_image.rect().center();
    qreal scaleX = (qreal)width() / m_image.width();
    qreal scaleY = (qreal)height() / m_image.height();
    m_scale = qMin(scaleX, scaleY) * 0.95;
    m_cropRectF = m_image.rect();
    m_cropRect = m_image.rect();
    m_selectionRect = QRectF();

    start_x = 0;
    start_y = 0;
    end_x = m_image.width();
    end_y = m_image.height();
    update();
}

void CanvasWidget::setPreviewMode(bool enabled) {
    if (m_previewMode == enabled) return;
    m_previewMode = enabled;
    emit previewModeChanged(enabled);
    update();
}

// ============================================================================
// Event Handlers
// ============================================================================

void CanvasWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw dark background and checkerboard
    painter.fillRect(rect(), WIDGET_BACKGROUND_COLOR);
    m_checkerboardBrush.setTransform(painter.transform().inverted());
    painter.fillRect(rect(), m_checkerboardBrush);

    if (m_image.isNull()) return;

    // Apply view transformation
    painter.translate(rect().center());
    painter.scale(m_scale, m_scale);
    painter.translate(-m_panOffset);
    painter.drawImage(0, 0, m_image);

    // Draw tool-specific overlays
    if (m_tool == ToolMode::Crop && m_previewMode) {
        drawCropTool(painter);
    }
    if (m_tool == ToolMode::Resize) {
        drawResizeHandle(painter);
    }
    if (m_tool == ToolMode::Select && !m_selectionRect.isNull()) {
        QPen marqueePen(Qt::white, 1.0 / m_scale, Qt::DashLine);
        marqueePen.setDashPattern({4.0, 4.0});
        painter.setPen(marqueePen);
        painter.setBrush(QColor(0, 100, 255, 20));
        painter.drawRect(m_selectionRect);
    }
}

void CanvasWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    if (m_image.isNull()) return;
    if (event->angleDelta().y() > 0)
        m_scale *= 1.25;
    else
        m_scale /= 1.25;
    m_scale = qBound(0.01, m_scale, 100.0);

    // Zoom towards mouse cursor
    QPointF mousePos = event->position();
    m_panOffset = screenToImage(mousePos) - (mousePos - rect().center()) / m_scale;
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if (m_image.isNull()) return;

    // Middle mouse or Move tool = panning
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && m_tool == ToolMode::Move)) {
        m_isPanning = true;
        m_panLastPos = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPointF imagePos = screenToImage(event->position());
        m_activeHandle = getHandleAt(imagePos);

        if (m_activeHandle != Handle::None) {
            if (m_tool == ToolMode::Resize) {
                saveState(); // Save before starting a resize
            }
            m_isDraggingTool = true;
            m_dragStartPos_image = imagePos;
        } else if (m_tool == ToolMode::Crop) {
            m_isDraggingTool = true;
            m_activeHandle = Handle::BottomRight;
            m_cropRectF.setTopLeft(imagePos);
            m_cropRectF.setSize(QSizeF(0, 0));
        } else if (m_tool == ToolMode::Select) {
            m_isDraggingTool = true;
            m_dragStartPos_image = imagePos;
            m_selectionRect = QRectF(imagePos, QSizeF(0, 0));
        }
        event->accept();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPointF delta = event->position() - m_panLastPos;
        m_panOffset -= delta / m_scale;
        m_panLastPos = event->position();
        update();
        return;
    }

    if (m_isDraggingTool) {
        QPointF currentImagePos = screenToImage(event->position());

        if (m_tool == ToolMode::Select) {
            m_selectionRect = QRectF(m_dragStartPos_image, currentImagePos).normalized();
            update();
            return;
        }

        QPointF delta = currentImagePos - m_dragStartPos_image;

        if (m_tool == ToolMode::Resize && m_activeHandle == Handle::BottomRight) {
            // Calculate new dimensions relative to the original image size
            int newWidth = qMax(10, (int)(o_image.width() + delta.x()));
            int newHeight = qMax(10, (int)(o_image.height() + delta.y()));
            applyResizeTool(newWidth, newHeight);
        }

        else if (m_tool == ToolMode::Crop) {
            QRectF newRect = m_cropRectF;
            switch (m_activeHandle) {
            case Handle::Move:
                newRect.translate(delta);
                m_dragStartPos_image = currentImagePos;
                break;
            case Handle::Top: newRect.setTop(currentImagePos.y()); break;
            case Handle::Bottom: newRect.setBottom(currentImagePos.y()); break;
            case Handle::Left: newRect.setLeft(currentImagePos.x()); break;
            case Handle::Right: newRect.setRight(currentImagePos.x()); break;
            case Handle::TopLeft: newRect.setTopLeft(currentImagePos); break;
            case Handle::TopRight: newRect.setTopRight(currentImagePos); break;
            case Handle::BottomLeft: newRect.setBottomLeft(currentImagePos); break;
            case Handle::BottomRight: newRect.setBottomRight(currentImagePos); break;
            default: break;
            }
            m_cropRectF = newRect.normalized();
            m_cropRect = m_cropRectF.toRect();
        }
        update();
    } else {
        updateCursor(event->position().toPoint());
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        updateCursor(event->position().toPoint());
    }
    if (event->button() == Qt::LeftButton) {
        if (m_tool == ToolMode::Move && m_isPanning) {
            m_isPanning = false;
        }
        if (m_isDraggingTool) {
            if (m_tool == ToolMode::Select) {
                QRectF boundedSelection = m_selectionRect.intersected(m_image.rect());
                start_x = qMax(0, (int)boundedSelection.left());
                end_x = qMin(m_image.width(), (int)boundedSelection.right());
                start_y = qMax(0, (int)boundedSelection.top());
                end_y = qMin(m_image.height(), (int)boundedSelection.bottom());
            } else if (m_tool == ToolMode::Resize) {
                // Commit the resize by updating the original image
                o_image = m_image;
            }
            m_isDraggingTool = false;
            m_activeHandle = Handle::None;
        }
        updateCursor(event->position().toPoint());
    }
}

void CanvasWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_D && event->modifiers() == Qt::ControlModifier) {
        if (!m_selectionRect.isNull()) {
            m_selectionRect = QRectF(); // Clear selection
            // Reset filter region to full image
            start_x = 0;
            start_y = 0;
            end_x = m_image.width();
            end_y = m_image.height();
            update();
        }
    } else {
        QWidget::keyPressEvent(event); // Pass other key events up
    }
}

// ============================================================================
// Drawing, Coordinate, and Cursor Helpers
// ============================================================================

void CanvasWidget::drawCropTool(QPainter &painter) {
    painter.save();
    QPainterPath path;
    path.addRect(m_image.rect());
    path.addRect(m_cropRectF);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawPath(path);

    const qreal penWidth = 2.0 / m_scale;
    painter.setPen(QPen(Qt::white, penWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_cropRectF);

    const qreal handleSize = 10.0 / m_scale;
    painter.setBrush(Qt::white);
    painter.drawRect(QRectF(m_cropRectF.topLeft(), QSizeF(handleSize, handleSize)));
    painter.drawRect(QRectF(m_cropRectF.topRight() - QPointF(handleSize, 0), QSizeF(handleSize, handleSize)));
    painter.drawRect(QRectF(m_cropRectF.bottomLeft() - QPointF(0, handleSize), QSizeF(handleSize, handleSize)));
    painter.drawRect(QRectF(m_cropRectF.bottomRight() - QPointF(handleSize, handleSize), QSizeF(handleSize, handleSize)));
    painter.restore();
}

void CanvasWidget::drawResizeHandle(QPainter &painter) {
    painter.save();
    const qreal handleSize = 15.0 / m_scale;
    QRectF handleRect(m_image.rect().bottomRight() - QPointF(handleSize/2, handleSize/2),
                      QSizeF(handleSize, handleSize));
    painter.setPen(QPen(Qt::white, 2.0 / m_scale));
    painter.setBrush(Qt::white);
    painter.drawEllipse(handleRect);
    painter.restore();
}

QPointF CanvasWidget::screenToImage(const QPointF &p_screen) const {
    if(m_image.isNull()) return QPointF();
    return (p_screen - rect().center()) / m_scale + m_panOffset;
}

CanvasWidget::Handle CanvasWidget::getHandleAt(const QPointF &p_image) const {
    const qreal handleSize = 15.0 / m_scale;

    if (m_tool == ToolMode::Crop && m_previewMode) {
        QRectF top(m_cropRectF.left(), m_cropRectF.top() - handleSize/2, m_cropRectF.width(), handleSize);
        QRectF bottom(m_cropRectF.left(), m_cropRectF.bottom() - handleSize/2, m_cropRectF.width(), handleSize);
        QRectF left(m_cropRectF.left() - handleSize/2, m_cropRectF.top(), handleSize, m_cropRectF.height());
        QRectF right(m_cropRectF.right() - handleSize/2, m_cropRectF.top(), handleSize, m_cropRectF.height());

        if (QRectF(m_cropRectF.topLeft(), QSizeF(handleSize,handleSize)).contains(p_image))
            return Handle::TopLeft;
        if (QRectF(m_cropRectF.topRight() - QPointF(handleSize,0), QSizeF(handleSize,handleSize)).contains(p_image))
            return Handle::TopRight;
        if (QRectF(m_cropRectF.bottomLeft() - QPointF(0,handleSize), QSizeF(handleSize,handleSize)).contains(p_image))
            return Handle::BottomLeft;
        if (QRectF(m_cropRectF.bottomRight() - QPointF(handleSize,handleSize), QSizeF(handleSize,handleSize)).contains(p_image))
            return Handle::BottomRight;
        if (top.contains(p_image)) return Handle::Top;
        if (bottom.contains(p_image)) return Handle::Bottom;
        if (left.contains(p_image)) return Handle::Left;
        if (right.contains(p_image)) return Handle::Right;
        if (m_cropRectF.contains(p_image)) return Handle::Move;
    }
    else if (m_tool == ToolMode::Resize) {
        QRectF bottomRightHandle(m_image.rect().bottomRight() - QPointF(handleSize, handleSize),
                                 QSizeF(handleSize*2, handleSize*2));
        if(bottomRightHandle.contains(p_image)) return Handle::BottomRight;
    }
    else if (m_tool == ToolMode::Move) {
        if (m_image.rect().contains(p_image.toPoint())) {
            return Handle::Move;
        }
    }
    return Handle::None;
}

void CanvasWidget::updateCursor(const QPoint& p_screen) {
    if (m_isPanning) {
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    QPointF p_image = screenToImage(p_screen);
    Handle handle = getHandleAt(p_image);
    Qt::CursorShape cursor = Qt::ArrowCursor;

    switch (m_tool) {
    case ToolMode::None: cursor = Qt::ArrowCursor; break;
    case ToolMode::Select: cursor = Qt::CrossCursor; break;
    case ToolMode::Move: cursor = Qt::OpenHandCursor; break;
    case ToolMode::Crop:
        switch(handle) {
        case Handle::TopLeft: case Handle::BottomRight: cursor = Qt::SizeFDiagCursor; break;
        case Handle::TopRight: case Handle::BottomLeft: cursor = Qt::SizeBDiagCursor; break;
        case Handle::Left: case Handle::Right: cursor = Qt::SizeHorCursor; break;
        case Handle::Top: case Handle::Bottom: cursor = Qt::SizeVerCursor; break;
        case Handle::Move: cursor = Qt::SizeAllCursor; break;
        default: cursor = Qt::CrossCursor;
        }
        break;
    case ToolMode::Resize:
        if(handle == Handle::BottomRight) cursor = Qt::SizeFDiagCursor;
        else cursor = Qt::ArrowCursor;
        break;
    }
    setCursor(cursor);
}

// ============================================================================
// State Management (Undo/Redo)
// ============================================================================

void CanvasWidget::saveState() {
    m_undoStack.push({m_image, o_image});
    if (m_undoStack.size() > MAX_UNDO_STEPS) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();
}

void CanvasWidget::undo() {
    if (m_undoStack.isEmpty()) return;
    m_redoStack.push({m_image, o_image});
    QPair<QImage, QImage> state = m_undoStack.pop();
    m_image = state.first;
    o_image = state.second;
    update();
}

void CanvasWidget::redo() {
    if (m_redoStack.isEmpty()) return;
    m_undoStack.push({m_image, o_image});
    QPair<QImage, QImage> state = m_redoStack.pop();
    m_image = state.first;
    o_image = state.second;
    update();
}

bool CanvasWidget::canUndo() const { return !m_undoStack.isEmpty(); }
bool CanvasWidget::canRedo() const { return !m_redoStack.isEmpty(); }

void CanvasWidget::resetImage() {
    saveState();
    m_image = r_image;
    o_image = r_image;
    resetView();
}

void CanvasWidget::commitChanges() {
    m_baseImage = QImage();
    o_baseImage = QImage();
}

void CanvasWidget::cancelChanges() {
    if (!m_baseImage.isNull()) {
        m_image = m_baseImage;
        m_baseImage = QImage();
    }
    if (!o_baseImage.isNull()) {
        o_image = o_baseImage;
        o_baseImage = QImage();
    }
    update();
}

// ============================================================================
// Filters and Tools
// ============================================================================

void CanvasWidget::applyCrop() {
    if(m_image.isNull() || !m_cropRectF.isValid() || m_cropRectF.size().isEmpty()) return;
    saveState();
    m_image = m_image.copy(m_cropRectF.toRect().intersected(m_image.rect()));
    o_image = m_image;
    setTool(ToolMode::None);
    resetView();
}

void CanvasWidget::applyCrop(int xs, int xe, int ys, int ye) {
    if(m_image.isNull()) return;
    saveState();

    QRect cropRect(xs, ys, xe - xs, ye - ys);
    cropRect = cropRect.intersected(m_image.rect());

    m_image = m_image.copy(cropRect);
    o_image = m_image;
    resetView();
}

void CanvasWidget::applyResizeTool(int newWidth, int newHeight) {
    if (o_image.isNull()) return;

    m_image = o_image.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    update();
}

void CanvasWidget::applyGrayScaleFilter() {
    if (m_image.isNull()) return;
    saveState();
    QImage result = m_image;
    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            int gray = qGray(result.pixel(x, y));
            result.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyInversionFilter() {
    if (m_image.isNull()) return;
    saveState();
    m_image.invertPixels(QImage::InvertRgb);
    update();
}

void CanvasWidget::applyVerticalReflection() {
    if (m_image.isNull()) return;
    saveState();

    QImage result = m_image;
    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y / 2; y++) {
            QRgb temp = result.pixel(x, y);
            result.setPixel(x, y, result.pixel(x, end_y - 1 - y));
            result.setPixel(x, end_y - 1 - y, temp);
        }
    }

    m_image = result;
    update();
}

void CanvasWidget::applyHorizontalReflection() {
    if (m_image.isNull()) return;
    saveState();

    QImage result = m_image;
    for (int x = start_x; x < end_x / 2; x++) {
        for (int y = start_y; y < end_y; y++) {
            QRgb temp = result.pixel(x, y);
            result.setPixel(x, y, result.pixel(end_x - 1 - x, y));
            result.setPixel(end_x - 1 - x, y, temp);
        }
    }

    m_image = result;
    update();
}

void CanvasWidget::applyYellowFilter(const int intensity, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    QImage result = m_baseImage;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            QColor color = m_baseImage.pixelColor(x, y);
            color.setRgb(qBound(0, color.red() + intensity, 255),
                         qBound(0, color.green() + intensity, 255),
                         qBound(0, color.blue() - intensity, 255));
            result.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyPurpleFilter(const int intensity, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    QImage result = m_baseImage;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            QColor color = m_baseImage.pixelColor(x, y);
            color.setRgb(qBound(0, color.red() + intensity, 255),
                         qBound(0, color.green() - intensity, 255),
                         qBound(0, color.blue() + intensity, 255));
            result.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyInfraRedFilter() {
    if (m_image.isNull()) return;
    saveState();
    QImage result = m_image;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int gray = qGray(result.pixel(x,y));
            result.setPixel(x, y, qRgb(255, gray, gray));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyBlackAndWhiteFilter(int threshold, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    QImage result = m_baseImage;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int gray = qGray(m_baseImage.pixel(x, y));
            result.setPixel(x, y, gray >= threshold ? qRgb(255,255,255) : qRgb(0,0,0));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyBlurFilter(int kernelSize, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();
    if (kernelSize < 1) kernelSize = 1;
    if (kernelSize % 2 == 0) kernelSize++;

    int radius = kernelSize / 2;
    QImage result = m_baseImage;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    int nx = x + j;
                    int ny = y + i;
                    if (QRect(start_x, start_y, end_x-start_x, end_y-start_y).contains(nx, ny)) {
                        QColor c = m_baseImage.pixelColor(nx, ny);
                        r += c.red(); g += c.green(); b += c.blue();
                        count++;
                    }
                }
            }
            if(count > 0) result.setPixelColor(x, y, QColor(r/count, g/count, b/count));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applySharpenFilter() {
    if (m_image.isNull()) return;
    saveState();

    // Convert QImage to Image for processing
    Image img = qImageToImage(m_image);
    Image copy = img;

    const int op[3][3] = {{0, -1, 0},
                          {-1, 5, -1},
                          {0, -1, 0}};

    for(int row = 0; row < img.height; row++){
        for(int col = 0; col < img.width; col++){
            int r = 0, g = 0, b = 0;
            for(int i = -1; i <= 1; i++){
                for(int j = -1; j <= 1; j++){
                    if(col+j < 0 || row+i < 0 || col+j >= img.width || row+i >= img.height){
                        continue;
                    }
                    r += copy(col+j, row+i, 0) * op[i+1][j+1];
                    g += copy(col+j, row+i, 1) * op[i+1][j+1];
                    b += copy(col+j, row+i, 2) * op[i+1][j+1];
                }
            }
            img(col, row, 0) = std::max(std::min(r, 255), 0);
            img(col, row, 1) = std::max(std::min(g, 255), 0);
            img(col, row, 2) = std::max(std::min(b, 255), 0);
        }
    }

    // Convert back to QImage
    m_image = imageToQImage(img);
    update();
}

void CanvasWidget::applyPixelSortFilter(int threshold, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    // Convert QImage to Image for processing
    Image image = qImageToImage(m_baseImage);

    for(int col = 0; col < image.width; col++){
        int curSt = 0;
        std::vector<std::vector<int>> pxls;

        for(int row = 0; row < image.height; row++){
            if((image(col, row, 0) + image(col, row, 1) + image(col, row, 2)) / 3 < threshold){
                std::vector<int> pxl{image(col, row, 0), image(col, row, 1), image(col, row, 2)};
                pxls.push_back(pxl);
            }
            else{
                if(!pxls.size()){
                    curSt = row + 1;
                    continue;
                }
                std::sort(pxls.begin(), pxls.end(),
                          [](std::vector<int> &a, std::vector<int> &b){
                              return (a[0] + a[1] + a[2]) < (b[0] + b[1] + b[2]);
                          });

                for(int i = curSt; i < row; i++){
                    image(col, i, 0) = pxls[i - curSt][0];
                    image(col, i, 1) = pxls[i - curSt][1];
                    image(col, i, 2) = pxls[i - curSt][2];
                }
                curSt = row + 1;
                pxls.clear();
            }
        }
    }

    // Convert back to QImage
    m_image = imageToQImage(image);
    update();
}

void CanvasWidget::applyEdgeDetection() {
    if (m_image.isNull()) return;
    saveState();

    const int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    QImage result = m_image;
    for (int y = start_y + 1; y < end_y - 1; y++) {
        for (int x = start_x + 1; x < end_x - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int gray = qGray(m_image.pixel(x + j, y + i));
                    sumX += gray * Gx[i + 1][j + 1];
                    sumY += gray * Gy[i + 1][j + 1];
                }
            }
            int mag = qBound(0, (int)qSqrt(sumX * sumX + sumY * sumY), 255);
            result.setPixel(x, y, qRgb(mag, mag, mag));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyLightOrDarkFilter(int percent, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    QImage result = m_baseImage;
    int delta = (255 * percent) / 100;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            QColor c = m_baseImage.pixelColor(x, y);
            result.setPixelColor(x,y, QColor(qBound(0, c.red()+delta, 255),
                                              qBound(0, c.green()+delta, 255),
                                              qBound(0, c.blue()+delta, 255)));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::rotate(QImage &image, int degrees) {
    if (image.isNull() || degrees == 0) return;
    QTransform transform;
    transform.rotate(degrees);
    image = image.transformed(transform, Qt::SmoothTransformation);
}

void CanvasWidget::applyRotateFilter(int degrees) {
    if (m_image.isNull()) return;
    saveState();
    rotate(m_image, degrees);
    rotate(o_image, degrees);
    m_lastRotation = (m_lastRotation + degrees) % 360;
    resetView();
}

void CanvasWidget::applyFrameFilter(int thickness, int r, int g, int b) {
    if (m_image.isNull()) return;
    if (m_baseImage.isNull()) m_baseImage = m_image;
    saveState();

    QImage result = m_image;
    QPainter p(&result);
    p.setPen(QPen(QColor(r,g,b), thickness * 2));
    p.drawRect(result.rect().adjusted(thickness, thickness, -thickness, -thickness));
    m_image = result;
    update();
}

void CanvasWidget::applyTvNoiseFilter() {
    if (m_image.isNull()) return;
    saveState();

    QImage result = m_image;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(-30, 30);

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            QColor c = result.pixelColor(x, y);
            result.setPixelColor(x, y, QColor(qBound(0, c.red()+distr(gen), 255),
                                              qBound(0, c.green()+distr(gen), 255),
                                              qBound(0, c.blue()+distr(gen), 255)));
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyOilPaintFilter(int kernelSize, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();
    if (kernelSize < 1) kernelSize = 5;

    int radius = kernelSize / 2;
    QImage result = m_baseImage;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            std::vector<int> counts(256,0), sumR(256,0), sumG(256,0), sumB(256,0);
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    int ny = qBound(start_y, y + i, end_y - 1);
                    int nx = qBound(start_x, x + j, end_x - 1);
                    int intensity = qGray(m_baseImage.pixel(nx, ny));
                    QColor c = m_baseImage.pixelColor(nx, ny);
                    counts[intensity]++;
                    sumR[intensity]+=c.red();
                    sumG[intensity]+=c.green();
                    sumB[intensity]+=c.blue();
                }
            }
            int maxCount = 0, maxIntensity = 0;
            for(int i=0; i<256; ++i) {
                if(counts[i]>maxCount){
                    maxCount=counts[i];
                    maxIntensity=i;
                }
            }
            if(maxCount>0) {
                result.setPixelColor(x,y, QColor(sumR[maxIntensity]/maxCount,
                                                  sumG[maxIntensity]/maxCount,
                                                  sumB[maxIntensity]/maxCount));
            }
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applySkewFilter(double degree, FilterMode mode) {
    if (m_baseImage.isNull()) m_baseImage = m_image;
    if (m_image.isNull()) return;
    if (mode == FilterMode::Increment) saveState();

    QTransform transform;
    transform.shear(qTan(qDegreesToRadians(degree)), 0);
    m_image = m_baseImage.transformed(transform, Qt::SmoothTransformation);
    resetView();
    update();
}

bool CanvasWidget::isMImageNull() {
    return m_image.isNull();
}

Image CanvasWidget::qImageToImage(const QImage &qimg) {
    Image img(qimg.width(), qimg.height());
    for (int y = 0; y < qimg.height(); y++) {
        for (int x = 0; x < qimg.width(); x++) {
            QColor color = qimg.pixelColor(x, y);
            img.setPixel(x, y, 0, static_cast<unsigned char>(color.red()));
            img.setPixel(x, y, 1, static_cast<unsigned char>(color.green()));
            img.setPixel(x, y, 2, static_cast<unsigned char>(color.blue()));
        }
    }
    return img;
}

QImage CanvasWidget::imageToQImage(const Image &img) {
    QImage qImg(img.width, img.height, QImage::Format_RGB888);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            qImg.setPixel(x, y, qRgb(img(x, y, 0), img(x, y, 1), img(x, y, 2)));
        }
    }
    return qImg;
}
