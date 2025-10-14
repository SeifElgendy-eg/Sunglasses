#include "CanvasWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <bits/stdc++.h>

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

void CanvasWidget::setImage(const QImage &img) {
    m_image = img;
    o_image = img;

    start_x = 0;
    end_x = m_image.width();
    start_y = 0;
    end_y = m_image.height();

    setMinimumSize(img.size());
    update(); // calls paintEvent()
}

void CanvasWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    if (!m_image.isNull()) {
        p.drawImage(m_offset, m_image);
        checkRect =
            QRect(m_offset.x(), m_offset.y(), m_image.width(), m_image.height());
        if (m_tool != ToolMode::Select || interRect.isNull()) {
            start_x = 0;
            end_x = m_image.width();
            start_y = 0;
            end_y = m_image.height();
        }
    } else
        p.drawText(rect(), Qt::AlignCenter, "No image loaded");

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

        // Draw the selection border
        QRect borderRect(m_offset.x(), m_offset.y(), m_image.width(),
                         m_image.height());
        QPen borderPen(Qt::blue, 2);
        p.setPen(borderPen);
        p.drawRect(borderRect);

        // Draw bottom center handle
        int handleSize = 10;
        QPoint handleBottomCenter(borderRect.center().x(), borderRect.bottom());
        QRect handleBottomRect(handleBottomCenter.x() - handleSize / 2,
                               handleBottomCenter.y() - handleSize / 2, handleSize,
                               handleSize);

        p.fillRect(handleBottomRect, Qt::gray);
        // Draw right center handle

        QPoint handleRightCenter(borderRect.right(), borderRect.center().y());
        QRect handleRightRect(handleRightCenter.x() - handleSize / 2,
                              handleRightCenter.y() - handleSize / 2, handleSize,
                              handleSize);

        p.fillRect(handleRightRect, Qt::gray);

        m_selectionRect = borderRect;
    }
}

void CanvasWidget::setTool(ToolMode tool) {
    m_tool = tool;
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {

    if (m_image.isNull())
        return;

    if (event->button() == Qt::LeftButton) {
        if ((event->modifiers() & Qt::ControlModifier) &&
            checkRect.contains(event->pos())) {
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
    QPoint handleBottomCenter(m_selectionRect.center().x(),
                              m_selectionRect.bottom());
    QRect handleBottomRect(handleBottomCenter.x() - handleSize / 2,
                           handleBottomCenter.y() - handleSize / 2, handleSize,
                           handleSize);

    QPoint handleRightCenter(m_selectionRect.right(),
                             m_selectionRect.center().y());
    QRect handleRightRect(handleRightCenter.x() - handleSize / 2,
                          handleRightCenter.y() - handleSize / 2, handleSize,
                          handleSize);

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

    if ((m_draggingHandle_x || m_draggingHandle_y) &&
        m_tool == ToolMode::Resize) {
        int newHeight = o_image.height();
        int newWidth =
            o_image.width(); // Max is used to prevent negative widths/heights
        if (m_draggingHandle_y) {
            int deltaY = event->pos().y() - m_dragStart.y();
            newHeight = std::max(10, m_image.height() + deltaY);
            newWidth = m_image.width();
        } else if (m_draggingHandle_x) {
            int deltaX = event->pos().x() - m_dragStart.x();
            newWidth = std::max(10, m_image.width() + deltaX);
            newHeight = m_image.height();
        }

        // Apply resizing (nearest neighbor)
        QImage resized(newWidth, newHeight, o_image.format());
        double scaleY = static_cast<double>(o_image.height()) / newHeight;
        double scaleX = static_cast<double>(o_image.width()) / newWidth;

        for (int x = 0; x < resized.width(); x++) {
            for (int y = 0; y < resized.height(); y++) {
                int oldX = std::floor(int(x * scaleX));
                int oldY = std::floor(int(y * scaleY));
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

            update(); // update is queued so that it is called after the current
            // function returns
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

void CanvasWidget::applyGrayScaleFilter() {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;
            color.setRgb(gray, gray, gray);
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;
            color.setRgb(gray, gray, gray);
            o_image.setPixelColor(x, y, color);
        }
    }

    m_image = result;
    update(); // redraw on screen
}

void CanvasWidget::applyInversionFilter() {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            color.setRgb(255 - color.red(), 255 - color.green(), 255 - color.blue());
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);
            color.setRgb(255 - color.red(), 255 - color.green(), 255 - color.blue());
            o_image.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyVeriticalReflection() {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y / 2; y++) {

            // QColor color_1 = result.pixelColor(x, y);
            // QColor color_2 = result.pixelColor(x, m_image.height() - 1 - y);

            // std::swap(color_1,color_2);

            QRgb temp = result.pixel(x, y);

            result.setPixel(x, y, result.pixel(x, end_y - 1 - y));

            result.setPixel(x, end_y - 1 - y, temp);
        }
    }

    int o_x_s = start_x * o_image.width() / m_image.width();
    int o_x_e = end_x * o_image.width() / m_image.width();
    int o_y_s = start_y * o_image.height() / m_image.height();
    int o_y_e = end_y * o_image.height() / m_image.height();

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e / 2; y++) {

            QRgb temp = o_image.pixel(x, y);

            o_image.setPixel(x, y, o_image.pixel(x, o_y_e - 1 - y));

            o_image.setPixel(x, o_y_e - 1 - y, temp);
        }
    }

    m_image = result;
    update();
};
void CanvasWidget::applyHorizontalReflection() {

    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x / 2; x++) {
        for (int y = start_y; y < end_y; y++) {

            // QColor color_1 = result.pixelColor(x, y);
            // QColor color_2 = result.pixelColor(x, m_image.height() - 1 - y);

            // std::swap(color_1,color_2);

            QRgb temp = result.pixel(x, y);

            result.setPixel(x, y, result.pixel(end_x - 1 - x, y));

            result.setPixel(end_x - 1 - x, y, temp);
        }
    }

    int o_x_s = start_x * o_image.width() / m_image.width();
    int o_x_e = end_x * o_image.width() / m_image.width();
    int o_y_s = start_y * o_image.height() / m_image.height();
    int o_y_e = end_y * o_image.height() / m_image.height();

    for (int x = o_x_s; x < o_x_e / 2; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {

            QRgb temp = o_image.pixel(x, y);

            o_image.setPixel(x, y, o_image.pixel(o_x_e - x - 1, y));

            o_image.setPixel(o_x_e - x - 1, y, temp);
        }
    }

    m_image = result;
    update();
};

void CanvasWidget::applyYellowFilter(const int intensity) {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() + intensity, 0, 255),
                         std::clamp(color.blue() - intensity, 0, 255));
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() + intensity, 0, 255),
                         std::clamp(color.blue() - intensity, 0, 255));
            o_image.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyPurpleFilter(const int intensity) {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() - intensity, 0, 255),
                         std::clamp(color.blue() + intensity, 0, 255));
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() - intensity, 0, 255),
                         std::clamp(color.blue() + intensity, 0, 255));
            o_image.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyInfraRedFilter() {
    applyInversionFilter();
    applyGrayScaleFilter();

    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            color.setRed(255);
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);
            color.setRed(255);
            o_image.setPixelColor(x, y, color);
        }
    }
    m_image = result;
    update();
}

void CanvasWidget::applyBlackAndWhiteFilter() {
    if (m_image.isNull())
        return;

    QImage result = m_image;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;

            if (gray >= 128) {
                color.setRgb(255, 255, 255);
                result.setPixelColor(x, y, color);
            } else {
                color.setRgb(0, 0, 0);
                result.setPixelColor(x, y, color);
            }
        }
    }

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_image.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;

            if (gray >= 128) {
                color.setRgb(255, 255, 255);
                o_image.setPixelColor(x, y, color);
            } else {
                color.setRgb(0, 0, 0);
                o_image.setPixelColor(x, y, color);
            }
        }
    }

    m_image = result;
    update(); // redraw on screen
}
void CanvasWidget::applyBlurFilter(int kernelSize) {
    if (m_baseImage.isNull())
        m_baseImage = m_image;
    if (m_image.isNull())
        return;

    // Ensure minimum and odd kernel size
    if (kernelSize < 1)
        kernelSize = 1;
    if (kernelSize % 2 == 0)
        kernelSize++;

    int radius = kernelSize / 2;

    QImage result = m_baseImage;
    QImage blurred = result;

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            int red = 0, green = 0, blue = 0, count = 0;

            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < result.width() && ny >= 0 &&
                        ny < result.height()) {
                        QColor color = result.pixelColor(nx, ny);
                        red += color.red();
                        green += color.green();
                        blue += color.blue();
                        count++;
                    }
                }
            }

            QColor newColor(std::round(float(red) / count),
                            std::round(float(green) / count),
                            std::round(float(blue) / count));

            blurred.setPixelColor(x, y, newColor);
    }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image;
    else if (o_image.isNull())
        return;

    QImage o_result = o_baseImage;
    QImage o_blurred = o_result;

    int o_x_s = start_x * o_image.width() / m_image.width();
    int o_x_e = end_x * o_image.width() / m_image.width();
    int o_y_s = start_y * o_image.height() / m_image.height();
    int o_y_e = end_y * o_image.height() / m_image.height();

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            int red = 0, green = 0, blue = 0, count = 0;

            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < o_result.width() && ny >= 0 &&
                        ny < o_result.height()) {
                        QColor color = o_result.pixelColor(nx, ny);
                        red += color.red();
                        green += color.green();
                        blue += color.blue();
                        count++;
                    }
                }
            }

            QColor newColor(std::round(float(red) / count),
                            std::round(float(green) / count),
                            std::round(float(blue) / count));

            o_blurred.setPixelColor(x, y, newColor);
        }
    }
    m_image = blurred;
    o_image = o_blurred;
    update(); // redraw on screen
}

void CanvasWidget::applyResizeTool(int newWidth, int newHeight) {
    if (m_image.isNull())
        return;

    if (newWidth <= 0)
        newWidth = m_image.width();
    if (newHeight <= 0)
        newHeight = m_image.height();

    // Apply resizing (nearest neighbor)
    QImage resized(newWidth, newHeight, o_image.format());
    double scaleY = static_cast<double>(o_image.height()) / newHeight;
    double scaleX = static_cast<double>(o_image.width()) / newWidth;

    for (int x = 0; x < resized.width(); x++) {
        for (int y = 0; y < resized.height(); y++) {
            int oldX = std::floor(int(x * scaleX));
            int oldY = std::floor(int(y * scaleY));
            resized.setPixelColor(x, y, o_image.pixelColor(oldX, oldY));
        }
    }

    m_image = resized;
    update();
}
void CanvasWidget::applyEdgeDetection() {

    if (m_image.isNull())
        return;

    const int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    QImage copy = m_image;

    for (int x = start_y; x < end_y; x++) {
        for (int y = start_x; y < end_x; y++) {
            int Ix[3] = {0, 0, 0};
            int Iy[3] = {0, 0, 0};
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = x + di;
                    int nj = y + dj;

                    if (ni >= 0 && ni < end_y && nj >= 0 && nj < end_x) {
                        int gx_val = Gx[di + 1][dj + 1];
                        int gy_val = Gy[di + 1][dj + 1];

                        QColor color = copy.pixelColor(nj, ni);
                        Ix[0] += color.red() * gx_val;
                        Ix[1] += color.green() * gx_val;
                        Ix[2] += color.blue() * gx_val;

                        Iy[0] += color.red() * gy_val;
                        Iy[1] += color.green() * gy_val;
                        Iy[2] += color.blue() * gy_val;
                    }
                }
            }
            QColor color = m_image.pixelColor(y, x);
            for (int c = 0; c < 3; c++) {
                int mag = static_cast<int>(std::sqrt(Ix[c] * Ix[c] + Iy[c] * Iy[c]));

                int val = std::min(mag, 255);
                if (c == 0)
                    color.setRed(val);
                else if (c == 1)
                    color.setGreen(val);
                else if (c == 2)
                    color.setBlue(val);
            }
            m_image.setPixelColor(y, x, color);
        }
    }

    // if (o_baseImage.isNull())
    //     o_baseImage = o_image;
    // else if (o_image.isNull())
    //     return;

    QImage o_copy = o_image;

    int o_x_s = start_x * o_image.width() / m_image.width();
    int o_x_e = end_x * o_image.width() / m_image.width();
    int o_y_s = start_y * o_image.height() / m_image.height();
    int o_y_e = end_y * o_image.height() / m_image.height();

    for (int x = o_y_s; x < o_y_e; x++) {
        for (int y = o_x_s; y < o_x_e; y++) {
            int Ix[3] = {0, 0, 0};
            int Iy[3] = {0, 0, 0};
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = x + di;
                    int nj = y + dj;

                    if (ni >= 0 && ni < end_y && nj >= 0 && nj < end_x) {
                        int gx_val = Gx[di + 1][dj + 1];
                        int gy_val = Gy[di + 1][dj + 1];

                        QColor color = o_copy.pixelColor(nj, ni);
                        Ix[0] += color.red() * gx_val;
                        Ix[1] += color.green() * gx_val;
                        Ix[2] += color.blue() * gx_val;

                        Iy[0] += color.red() * gy_val;
                        Iy[1] += color.green() * gy_val;
                        Iy[2] += color.blue() * gy_val;
                    }
                }
            }
            QColor color = o_image.pixelColor(y, x);
            for (int c = 0; c < 3; c++) {
                int mag = static_cast<int>(std::sqrt(Ix[c] * Ix[c] + Iy[c] * Iy[c]));

                int val = std::min(mag, 255);
                if (c == 0)
                    color.setRed(val);
                else if (c == 1)
                    color.setGreen(val);
                else if (c == 2)
                    color.setBlue(val);
            }
            o_image.setPixelColor(y, x, color);
        }
    }

    update(); // redraw on screen
};

void CanvasWidget::applyLightOrDarkFilter(int percent) {

    if (m_baseImage.isNull())
        m_baseImage = m_image;
    if (m_image.isNull())
        return;


    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = m_baseImage.pixelColor(x, y);
            int newR = std::clamp(0,255,color.red()+(color.red()*percent)/100);
            int newG = std::clamp(0,255,color.green()+(color.green()*percent)/100);
            int newB = std::clamp(0,255,color.blue()+(color.blue()*percent)/100);

            color.setRgb(newR,newG,newB);

            m_image.setPixelColor(x,y,color);
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image;
    else if (o_image.isNull())
        return;

    for (int x = start_x * o_image.width() / m_image.width();
         x < end_x * o_image.width() / m_image.width(); x++) {
        for (int y = start_y * o_image.height() / m_image.height();
             y < end_y * o_image.height() / m_image.height(); y++) {
            QColor color = o_baseImage.pixelColor(x, y);

            int newR = std::clamp(0,255,color.red()+(color.red()*percent)/100);
            int newG = std::clamp(0,255,color.green()+(color.green()*percent)/100);
            int newB = std::clamp(0,255,color.blue()+(color.blue()*percent)/100);

            color.setRgb(newR,newG,newB);
            o_image.setPixelColor(x, y, color);
        }
    update();
    }
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
