#include "CanvasWidget.h"
#include "Image_Class.h"
#include <QMouseEvent>
#include <QPainter>
#include <QRgb>
#include <QColor>
#include <algorithm>
#include <cmath>

extern void reflectH(Image &image);
extern void reflectV(Image &image);
extern void purpleFilter(Image &image, const int intensity);
extern void yellowFilter(Image &image, const int intensity);
extern void redscale(Image &image);

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

void CanvasWidget::setImage(const QImage &img) {
    m_image = img;
    o_image = img;
    r_image = img;

    start_x = 0;
    end_x = m_image.width();
    start_y = 0;
    end_y = m_image.height();

    setMinimumSize(img.size());
    update();
}

void CanvasWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    if (!m_image.isNull()) {
        p.drawImage(m_offset, m_image);
        checkRect = QRect(m_offset.x(), m_offset.y(), m_image.width(), m_image.height());

        if (m_tool != ToolMode::Select || interRect.isNull()) {
            start_x = 0;
            end_x = m_image.width();
            start_y = 0;
            end_y = m_image.height();
        }
    } else {
        p.drawText(rect(), Qt::AlignCenter, "No image loaded");
    }

    // Draw Selection Rectangle
    if (m_tool == ToolMode::Select) {
        if (!m_selection.isNull()) {
            QPen pen(Qt::black, 1, Qt::DashLine);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(m_selection);
        }
        if (!interRect.isNull()) {
            QPen pen(Qt::black, 1, Qt::DashLine);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(interRect);
        }
    }

    if (m_tool == ToolMode::Resize && !m_image.isNull()) {
        p.setRenderHint(QPainter::Antialiasing);

        QRect borderRect(m_offset.x(), m_offset.y(), m_image.width(), m_image.height());
        QPen borderPen(Qt::blue, 2);
        p.setPen(borderPen);
        p.drawRect(borderRect);

        int handleSize = 10;
        QPoint handleBottomCenter(borderRect.center().x(), borderRect.bottom());
        QRect handleBottomRect(handleBottomCenter.x() - handleSize / 2,
                               handleBottomCenter.y() - handleSize / 2,
                               handleSize, handleSize);
        p.fillRect(handleBottomRect, Qt::gray);

        QPoint handleRightCenter(borderRect.right(), borderRect.center().y());
        QRect handleRightRect(handleRightCenter.x() - handleSize / 2,
                              handleRightCenter.y() - handleSize / 2,
                              handleSize, handleSize);
        p.fillRect(handleRightRect, Qt::gray);

        m_selectionRect = borderRect;
    }
}

void CanvasWidget::setTool(ToolMode tool) {
    m_tool = tool;
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if (m_image.isNull()) return;

    if (event->button() == Qt::LeftButton) {
        if ((event->modifiers() & Qt::ControlModifier) && checkRect.contains(event->pos())) {
            m_moving = true;
            m_lastPos = event->pos();
            m_selection = QRect();
            interRect = QRect();
            update();
        } else if (m_tool == ToolMode::Select) {
            m_dragging = true;
            m_start = event->pos();
            m_selection = QRect(m_start, QSize());
            interRect = QRect();
        }
    }

    if (m_tool == ToolMode::Resize) {
        int handleSize = 10;
        QPoint handleBottomCenter(m_selectionRect.center().x(), m_selectionRect.bottom());
        QRect handleBottomRect(handleBottomCenter.x() - handleSize / 2,
                               handleBottomCenter.y() - handleSize / 2,
                               handleSize, handleSize);

        QPoint handleRightCenter(m_selectionRect.right(), m_selectionRect.center().y());
        QRect handleRightRect(handleRightCenter.x() - handleSize / 2,
                              handleRightCenter.y() - handleSize / 2,
                              handleSize, handleSize);

        if (handleBottomRect.contains(event->pos())) {
            m_draggingHandle_y = true;
            m_dragStart = event->pos();
        } else if (handleRightRect.contains(event->pos())) {
            m_draggingHandle_x = true;
            m_dragStart = event->pos();
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_moving) {
        QPoint delta = event->pos() - m_lastPos;
        m_offset += delta;
        m_lastPos = event->pos();
        update();
        return;
    } else if (m_dragging) {
        m_selection = QRect(m_start, event->pos()).normalized();
        update();
    }

    if ((m_draggingHandle_x || m_draggingHandle_y) && m_tool == ToolMode::Resize) {
        int newHeight = o_image.height();
        int newWidth = o_image.width();

        if (m_draggingHandle_y) {
            int deltaY = event->pos().y() - m_dragStart.y();
            newHeight = std::max(10, m_image.height() + deltaY);
            newWidth = m_image.width();
        } else if (m_draggingHandle_x) {
            int deltaX = event->pos().x() - m_dragStart.x();
            newWidth = std::max(10, m_image.width() + deltaX);
            newHeight = m_image.height();
        }

        QImage resized(newWidth, newHeight, o_image.format());
        double scaleY = static_cast<double>(o_image.height()) / newHeight;
        double scaleX = static_cast<double>(o_image.width()) / newWidth;

        for (int x = 0; x < resized.width(); x++) {
            for (int y = 0; y < resized.height(); y++) {
                int oldX = static_cast<int>(x * scaleX);
                int oldY = static_cast<int>(y * scaleY);
                resized.setPixelColor(x, y, o_image.pixelColor(oldX, oldY));
            }
        }

        m_image = resized;
        m_dragStart = event->pos();
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_dragging) {
            m_dragging = false;
            m_selection = QRect(m_start, event->pos()).normalized();
            if (m_selection.intersects(checkRect)) {
                interRect = m_selection.intersected(checkRect);
                start_x = (interRect.topLeft() - m_offset).x();
                end_x = (interRect.topRight() - m_offset).x();
                start_y = (interRect.topLeft() - m_offset).y();
                end_y = (interRect.bottomRight() - m_offset).y();
            }
            update();
        }
        if (m_moving) {
            m_moving = false;
            update();
        }
    }

    if (m_draggingHandle_x || m_draggingHandle_y) {
        m_draggingHandle_x = false;
        m_draggingHandle_y = false;
        update();
    }
}

void CanvasWidget::saveState() {
    m_undoStack.push(qMakePair(m_image, o_image));
    if (m_undoStack.size() > MAX_UNDO_STEPS) {
        m_undoStack.removeFirst();
    }
    clearRedoStack();
}

void CanvasWidget::undo() {
    if (m_undoStack.isEmpty()) return;
    m_redoStack.push(qMakePair(m_image, o_image));
    QPair<QImage, QImage> state = m_undoStack.pop();
    m_image = state.first;
    o_image = state.second;
    m_baseImage = QImage();
    o_baseImage = QImage();
    update();
}

void CanvasWidget::redo() {
    if (m_redoStack.isEmpty()) return;
    m_undoStack.push(qMakePair(m_image, o_image));
    QPair<QImage, QImage> state = m_redoStack.pop();
    m_image = state.first;
    o_image = state.second;
    m_baseImage = QImage();
    o_baseImage = QImage();
    update();
}

void CanvasWidget::clearRedoStack() {
    m_redoStack.clear();
}

void CanvasWidget::resetImage() {
    saveState();
    m_image = r_image;
    o_image = r_image;
    update();
}

Image qImageToImage(const QImage &qimg) {
    Image img(qimg.width(), qimg.height());
    for (int y = 0; y < qimg.height(); y++) {
        for (int x = 0; x < qimg.width(); x++) {
            unsigned int pixel = qimg.pixel(x, y);
            img.setPixel(x, y, 0, static_cast<unsigned char>(qRed(pixel)));
            img.setPixel(x, y, 1, static_cast<unsigned char>(qGreen(pixel)));
            img.setPixel(x, y, 2, static_cast<unsigned char>(qBlue(pixel)));
        }
    }
    return img;
}

QImage imageToQImage(const Image &img) {
    QImage qimg(img.width, img.height, QImage::Format_RGB888);
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            unsigned char r = img.getPixel(x, y, 0);
            unsigned char g = img.getPixel(x, y, 1);
            unsigned char b = img.getPixel(x, y, 2);
            qimg.setPixel(x, y, qRgb(r, g, b));
        }
    }
    return qimg;
}

void CanvasWidget::applyVeriticalReflection() {
    if (m_image.isNull()) return;
    saveState();

    Image img = qImageToImage(m_image);
    reflectV(img);
    m_image = imageToQImage(img);

    Image oimg = qImageToImage(o_image);
    reflectV(oimg);
    o_image = imageToQImage(oimg);

    update();
}

void CanvasWidget::applyHorizontalReflection() {
    if (m_image.isNull()) return;
    saveState();

    Image img = qImageToImage(m_image);
    reflectH(img);
    m_image = imageToQImage(img);

    Image oimg = qImageToImage(o_image);
    reflectH(oimg);
    o_image = imageToQImage(oimg);

    update();
}

void CanvasWidget::applyYellowFilter(const int intensity) {
    if (m_image.isNull()) return;
    saveState();

    Image img = qImageToImage(m_image);
    yellowFilter(img, intensity);
    m_image = imageToQImage(img);

    Image oimg = qImageToImage(o_image);
    yellowFilter(oimg, intensity);
    o_image = imageToQImage(oimg);

    update();
}

void CanvasWidget::applyPurpleFilter(const int intensity) {
    if (m_image.isNull()) return;
    saveState();

    Image img = qImageToImage(m_image);
    purpleFilter(img, intensity);
    m_image = imageToQImage(img);

    Image oimg = qImageToImage(o_image);
    purpleFilter(oimg, intensity);
    o_image = imageToQImage(oimg);

    update();
}

void CanvasWidget::applyInfraRedFilter() {
    if (m_image.isNull()) return;
    saveState();

    Image img = qImageToImage(m_image);
    redscale(img);
    m_image = imageToQImage(img);

    Image oimg = qImageToImage(o_image);
    redscale(oimg);
    o_image = imageToQImage(oimg);

    update();
}

void CanvasWidget::applyResizeTool(int newWidth, int newHeight) {
    if (m_image.isNull()) return;
    if (newWidth <= 0) newWidth = m_image.width();
    if (newHeight <= 0) newHeight = m_image.height();

    saveState();
    m_image = m_image.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    o_image = o_image.scaled(newWidth, newHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    update();
}

void CanvasWidget::commitChanges() {
    m_baseImage = QImage();
    o_baseImage = QImage();
}

void CanvasWidget::cancelChanges() {
    if (!m_baseImage.isNull()) {
        m_image = m_baseImage;
        m_baseImage = QImage();
        update();
    }
    if (!o_baseImage.isNull()) {
        o_image = o_baseImage;
        o_baseImage = QImage();
        update();
    }
}
