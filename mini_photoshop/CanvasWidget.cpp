#include "CanvasWidget.h"
#include <QtConcurrent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>
#include <QListWidget>
#include <bits/stdc++.h>


CanvasWidget::CanvasWidget(QWidget *parent) : QOpenGLWidget(parent),activeIndex(-1)  {
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

void CanvasWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    canvasRect = QRect(0,0,1920,1080);
    m_panOffset = QPoint(width()/2 - canvasRect.width()/2, height()/2 - canvasRect.height()/2);

}

void CanvasWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}


QPoint& CanvasWidget::offset() {
    if (activeIndex >= 0 && activeIndex < images.size())
        return images[activeIndex].offset;
    return m_offset;
}
QImage& CanvasWidget::m_image() {
if (activeIndex >= 0 && activeIndex < images.size())
return images[activeIndex].image;
return dump;
}

QImage& CanvasWidget::o_image() {
    if (activeIndex >= 0 && activeIndex < images.size())
        return images[activeIndex].o_image;
    return dump;
}


void CanvasWidget::addImage(const QImage &img) {
    if (img.isNull()) return;
    images.push_back({img,img,img,QPoint(0,0),true,});

    if (activeIndex == -1)
        setActiveLayer(0);
    setMinimumSize(200, 200);
    update();
}

void CanvasWidget::setActiveLayer(int index){
    activeIndex =index;
    Layer activeLayer = images[index];
    start_x = 0;
    end_x = m_image().width();
    start_y = 0;
    end_y = m_image().height();

    moveResizeGroup();
    moveCropGroup(0,0,0,0,'m');
    interRect = QRect();
    m_selection = QRect();
    update();
}


void CanvasWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    QPainter p(this);
    p.fillRect(rect(), QColor(53,53,53));


    // Apply pan and zoom
    p.translate(m_panOffset);
    p.scale(m_zoom, m_zoom);

    p.fillRect(canvasRect,canvasBackgroundColor);
        for (const Layer &layer : images) {
            if (!layer.visible) continue;
            QPixmap pix = QPixmap::fromImage(layer.image);
            p.drawPixmap(layer.offset, pix);
        }

        checkRect =
            QRect(offset().x(), offset().y(), m_image().width(), m_image().height());
        if (m_tool != ToolMode::Select || interRect.isNull()) {
            start_x = 0;
            end_x = m_image().width();
            start_y = 0;
            end_y = m_image().height();
            o_x_s = 0;
            o_x_e = o_image().width();
            o_y_s = 0;
            o_y_e = o_image().height();;
        }


    // Draw Selection Rectangle
    if (m_tool == ToolMode::Select) {
        if (!m_selection.isNull()) {
            QPen marqueePen(Qt::white, 1.0 / m_zoom, Qt::DashLine);
            marqueePen.setDashPattern({4.0, 4.0});
            p.setPen(marqueePen);
            p.setBrush(QColor(0, 0, 0, 20));
            p.drawRect(m_selection);
        }
        if (!interRect.isNull()) {
            QPen marqueePen(Qt::white, 1.0 / m_zoom, Qt::DashLine);
            marqueePen.setDashPattern({4.0, 4.0});
            p.setPen(marqueePen);
            p.setBrush(QColor(0, 0, 0, 20));
            p.drawRect(interRect);
        }
    }

    if (m_tool == ToolMode::Resize && !m_image().isNull()) {
        p.setRenderHint(QPainter::Antialiasing);
        QPen borderPen(Qt::blue, 2);
        p.setPen(borderPen);
        p.drawRect(m_resizeRect);


        p.fillRect(resizeHandleBottomRect, Qt::gray);
        // Draw right center handle


        p.fillRect(resizeHandleRightRect, Qt::gray);

    }

    if (m_tool == ToolMode::Crop && !m_image().isNull()) {

        p.save();

        QPen borderPen(Qt::blue, 2);
        p.setPen(borderPen);
        p.drawRect(m_cropRect);

        QPainterPath path;
        path.addRect(QRectF(offset(), QSizeF(m_image().width(), m_image().height())));
        p.setBrush(QColor(0, 0, 0, 150));
        p.setPen(Qt::NoPen);
        path.addRect(m_cropRect);
        p.drawPath(path);



        p.fillRect(handleBottomRect, Qt::gray);
        p.fillRect(handleUpperRect, Qt::gray);
        p.fillRect(handleRightRect, Qt::gray);
        p.fillRect(handleLeftRect, Qt::gray);

        p.restore();

    }
    emit requestUpdateLayers();
      p.end();
}

QPointF CanvasWidget::translateEventPosition(const QPointF &screenPoint) const {
    return (screenPoint - m_panOffset) / m_zoom;
}

void CanvasWidget::setTool(ToolMode tool) {
    m_tool = tool;
    update();

    if(m_tool == ToolMode::Resize){
        int handleSize = 10;

        // Draw the selection border
        m_resizeRect = QRect(offset().x(), offset().y(), m_image().width(), m_image().height());

        // Draw bottom center handle
         resizeBottomCenter = QPoint(m_resizeRect.center().x(), m_resizeRect.bottom());
         resizeHandleBottomRect = QRect(resizeBottomCenter.x() - handleSize / 2,
                               resizeBottomCenter.y() - handleSize / 2, handleSize,
                               handleSize);

        // Draw right center handle
        resizeRightCenter = QPoint(m_resizeRect.right(), m_resizeRect.center().y());
         resizeHandleRightRect= QRect(resizeRightCenter.x() - handleSize / 2,
                              resizeRightCenter.y() - handleSize / 2, handleSize,
                              handleSize);
    }
    if(m_tool == ToolMode::Crop){
        saveState();
        m_cropRect = QRect(offset().x(), offset().y(), m_image().width(), m_image().height());
        updateCropHandles();
       }
}

void CanvasWidget::updateCropHandles()
{
    const qreal handleSize = 10.0;

    handleBottomCenter = QPoint(m_cropRect.center().x(), m_cropRect.bottom());
    handleUpperCenter  = QPoint(m_cropRect.center().x(), m_cropRect.top());
    handleRightCenter  = QPoint(m_cropRect.right(), m_cropRect.center().y());
    handleLeftCenter   = QPoint(m_cropRect.left(),  m_cropRect.center().y());

    handleBottomRect = QRect(handleBottomCenter.x() - handleSize / 2,
                             handleBottomCenter.y() - handleSize / 2,
                             handleSize, handleSize);
    handleUpperRect  = QRect(handleUpperCenter.x() - handleSize / 2,
                            handleUpperCenter.y() - handleSize / 2,
                            handleSize, handleSize);
    handleRightRect  = QRect(handleRightCenter.x() - handleSize / 2,
                            handleRightCenter.y() - handleSize / 2,
                            handleSize, handleSize);
    handleLeftRect   = QRect(handleLeftCenter.x() - handleSize / 2,
                           handleLeftCenter.y() - handleSize / 2,
                           handleSize, handleSize);
}


void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    // if (m_image().isNull()) return;

    const double zoomFactor = 1.15;
    double oldZoom = m_zoom;

    if (event->angleDelta().y() > 0)
        m_zoom *= zoomFactor;
    else
        m_zoom /= zoomFactor;

    m_zoom = std::clamp(m_zoom, 0.01, 100.0); // Zoom limits

    updateCropHandles();

    QPointF mousePos = event->position();
    QPointF imagePosBefore = (mousePos - m_panOffset) / oldZoom;
    m_panOffset = mousePos - imagePosBefore * m_zoom;

    update();
}


void CanvasWidget::mousePressEvent(QMouseEvent *event) {

    if((m_tool ==ToolMode::Pan&&event ->button() == Qt::LeftButton) ||event->button() == Qt::MiddleButton){
        m_panning = true;
        m_lastPanPos = event ->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (m_image().isNull())
        return;

    if (event->button() == Qt::LeftButton) {
        if (((event->modifiers() & Qt::ControlModifier) ||
             (m_tool == ToolMode::Move)) &&
            checkRect.contains(translateEventPosition(event->pos()).toPoint())) {
            m_moving = true;
            m_lastPos = translateEventPosition(event->pos()).toPoint();
            m_selection = QRect();
            interRect = QRect();
            update();
        } else if (m_tool == ToolMode::Select) {
            m_dragging = true;
            m_start = translateEventPosition(event->pos()).toPoint();
            m_selection = QRect(m_start, QSize());
            interRect = QRect();
        }
    }






    if (m_tool == ToolMode::Resize) {

        if (resizeHandleBottomRect.contains(translateEventPosition(event->pos()).toPoint())) {
            saveState();
            m_draggingHandle_y = true;
            m_dragStart = translateEventPosition(event->pos()).toPoint();
        } else if (resizeHandleRightRect.contains(translateEventPosition(event->pos()).toPoint())) {
            saveState();
            m_draggingHandle_x = true;
            m_dragStart = translateEventPosition(event->pos()).toPoint();
        }
    }
    if (m_tool == ToolMode::Crop) {
        if (handleBottomRect.contains(translateEventPosition(event->pos()).toPoint())) {
            m_croppingHandle_y= true;
            m_cropStart = translateEventPosition(event->pos()).toPoint();
        } else if (handleRightRect.contains(translateEventPosition(event->pos()).toPoint())) {
            m_croppingHandle_x = true;
            m_cropStart = translateEventPosition(event->pos()).toPoint();
        } else if (handleLeftRect.contains(translateEventPosition(event->pos()).toPoint())) {
            m_croppingHandle_x_2 = true;
            m_cropStart = translateEventPosition(event->pos()).toPoint();
        } else if (handleUpperRect.contains(translateEventPosition(event->pos()).toPoint())) {
            m_croppingHandle_y_2= true;
            m_cropStart = translateEventPosition(event->pos()).toPoint();
        }
    }


     update();

}
void CanvasWidget::moveResizeGroup()
{
    m_resizeRect = QRect(offset().x(), offset().y(), m_image().width(), m_image().height());
    resizeBottomCenter = QPoint(m_resizeRect.center().x(), m_resizeRect.bottom());
    resizeHandleBottomRect.moveCenter(resizeBottomCenter);

    resizeRightCenter = QPoint(m_resizeRect.right(), m_resizeRect.center().y());
    resizeHandleRightRect.moveCenter(resizeRightCenter);
}

void CanvasWidget::moveCropGroup(int x, int x_2,int y, int y_2,char m)
{
    if( m =='m'){
        QPoint delta = offset() - m_lastCrop;
        m_cropRect.translate(delta);
        m_lastCrop = offset();
    }
    else{
    int newBottom =m_cropRect.bottom()+y;
    int newTop = m_cropRect.top()+y_2;
    int newRight = m_cropRect.right()+x;
    int newLeft = m_cropRect.left()+ x_2;
    if(checkRect.contains(QPoint(m_cropRect.center().x(), newBottom))&& newBottom > m_cropRect.top())
        m_cropRect.setBottom(newBottom);

    if (checkRect.contains(QPoint(m_cropRect.center().x(),newTop))&& newTop<m_cropRect.bottom())
            m_cropRect.setTop(newTop);

    if(checkRect.contains(QPoint(newRight,m_cropRect.center().y()))&&newRight>m_cropRect.left())
    m_cropRect.setRight(newRight);

    if(checkRect.contains(QPoint(newLeft,m_cropRect.center().y()))&& newLeft<m_cropRect.right())
    m_cropRect.setLeft(newLeft);
    }

    handleBottomCenter = QPoint(m_cropRect.center().x(), m_cropRect.bottom());
    handleBottomRect.moveCenter(handleBottomCenter);

    handleRightCenter = QPoint(m_cropRect.right(), m_cropRect.center().y());
    handleRightRect.moveCenter(handleRightCenter);

    handleUpperCenter = QPoint(m_cropRect.center().x(), m_cropRect.top());
    handleUpperRect.moveCenter(handleUpperCenter);

    handleLeftCenter = QPoint(m_cropRect.left(), m_cropRect.center().y());
    handleLeftRect.moveCenter(handleLeftCenter);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_moving) {
        QPoint delta = translateEventPosition(event->pos()).toPoint() - m_lastPos;
        offset() += delta;
        m_lastPos = translateEventPosition(event->pos()).toPoint();
        moveResizeGroup();
        moveCropGroup(0,0,0,0,'m');
        update();
        return;
    } else if (m_dragging) {
        m_selection = QRect(m_start, translateEventPosition(event->pos()).toPoint()).normalized();
        update();
    }

    if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPos;
        m_panOffset += delta;
        m_lastPanPos = event->pos();
        update();
        return;
    }

    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        return;
    }

    if ((m_draggingHandle_x || m_draggingHandle_y) &&
        m_tool == ToolMode::Resize) {
        int newHeight = o_image().height();
        int newWidth =o_image().width(); // Max is used to prevent negative widths/heights
        if (m_draggingHandle_y) {
            int deltaY = translateEventPosition(event->pos()).toPoint().y() - m_dragStart.y();
            newHeight = std::max(10, m_image().height() + deltaY);
            newWidth = m_image().width();
        } else if (m_draggingHandle_x) {
            int deltaX = translateEventPosition(event->pos()).toPoint().x() - m_dragStart.x();
            newWidth = std::max(10, m_image().width() + deltaX);
            newHeight = m_image().height();
        }

        // Apply resizing (nearest neighbor)
        QImage resized(newWidth, newHeight, o_image().format());
        double scaleY = static_cast<double>(o_image().height()) / newHeight;
        double scaleX = static_cast<double>(o_image().width()) / newWidth;

        for (int x = 0; x < resized.width(); x++) {
            for (int y = 0; y < resized.height(); y++) {
                int oldX = std::floor(int(x * scaleX));
                int oldY = std::floor(int(y * scaleY));
                resized.setPixelColor(x, y, o_image().pixelColor(oldX, oldY));
            }
        }
        moveResizeGroup();
        m_image() = resized;
        m_dragStart = translateEventPosition(event->pos()).toPoint();
        update();
    }

    if ((m_croppingHandle_x || m_croppingHandle_x_2 || m_croppingHandle_y ||
         m_croppingHandle_y_2) &&
        m_tool == ToolMode::Crop) {

        int deltaX=0, deltaX_2=0, deltaY=0, deltaY_2=0;

        if (m_croppingHandle_y) {
            deltaY = translateEventPosition(event->pos()).toPoint().y() - m_cropStart.y();
        }
        if (m_croppingHandle_y_2) {
            deltaY_2 = translateEventPosition(event->pos()).toPoint().y() - m_cropStart.y();
        }
        if (m_croppingHandle_x) {
            deltaX = translateEventPosition(event->pos()).toPoint().x() - m_cropStart.x();
        }
        if (m_croppingHandle_x_2) {
            deltaX_2 = translateEventPosition(event->pos()).toPoint().x() - m_cropStart.x();
        }

        moveCropGroup(deltaX, deltaX_2, deltaY, deltaY_2,'Y');
        m_cropStart = translateEventPosition(event->pos()).toPoint();
        update();
    }
}

void CanvasWidget::applyCrop(int xs,int xe,int ys, int ye){

    if(m_image().isNull())
        return;

    int cropX = xs - offset().x();
    int cropY = ys - offset().y();
    int cropW = xe - xs;
    int cropH = ye - ys;

    QImage cropped(cropW,cropH,m_image().format());

    for (int x = 0; x < cropW; x++)
        for (int y = 0; y < cropH; y++)
            cropped.setPixelColor(x, y, m_image().pixelColor(cropX + x, cropY + y));

    int o_cropW = cropW * o_image().width() / m_image().width();
    int o_cropH = cropH * o_image().height() / m_image().height();

    int o_cropX = cropX * o_image().width()  / m_image().width();
    int o_cropY = cropY * o_image().height() / m_image().height();

     QImage o_cropped(o_cropW,o_cropH,o_image().format());

    for (int x = 0; x < o_cropW; x++)
        for (int y = 0; y < o_cropH; y++)
            o_cropped.setPixelColor(x, y, o_image().pixelColor(o_cropX + x, o_cropY + y));

    m_image() = cropped;
    o_image() = o_cropped;
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_dragging) {
            m_dragging = false;
            m_selection = QRect(m_start, translateEventPosition(event->pos()).toPoint()).normalized();
            if (m_selection.intersects(checkRect)) {
                interRect = m_selection.intersected(checkRect);
                start_x = (interRect.topLeft() - offset()).x();
                end_x = std::min(m_image().width(),(interRect.topRight() - offset()).x()+1);
                start_y = (interRect.topLeft() - offset()).y();
                end_y = std::min(m_image().height(),(interRect.bottomRight() - offset()).y()+1);

                 o_x_s = start_x * o_image().width() / m_image().width();
                 o_x_e = end_x * o_image().width() / m_image().width();
                 o_y_s = start_y * o_image().height() / m_image().height();
                 o_y_e = end_y * o_image().height() / m_image().height();
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

    if (m_croppingHandle_x||m_croppingHandle_x_2 || m_croppingHandle_y||m_croppingHandle_y_2) {
        m_croppingHandle_x = false;
        m_croppingHandle_y = false;
        m_croppingHandle_x_2 = false;
        m_croppingHandle_y_2 = false;
        update();
    }

    if((m_panning &&event->button() == Qt::LeftButton) ||event->button() == Qt::MiddleButton){
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
}
void CanvasWidget::saveState() {
    images[activeIndex].undoStack.push(qMakePair(m_image(), o_image()));
    if (images[activeIndex].undoStack.size() > MAX_UNDO_STEPS) {
        images[activeIndex].undoStack.removeFirst();
    }
    clearRedoStack();
}

void CanvasWidget::undo() {
    if (images[activeIndex].undoStack.isEmpty())
        return;
    images[activeIndex].redoStack.push(qMakePair(m_image(), o_image()));
    QPair<QImage, QImage> state = images[activeIndex].undoStack.pop();
    m_image() = state.first;
    o_image() = state.second;
    if(m_tool == ToolMode::Resize){
        moveResizeGroup();
    }
    update();
}

void CanvasWidget::redo() {
    if (images[activeIndex].redoStack.isEmpty())
        return;
    images[activeIndex].undoStack.push(qMakePair(m_image(), o_image()));
    QPair<QImage, QImage> state = images[activeIndex].redoStack.pop();
    m_image() = state.first;
    o_image() = state.second;
    if(m_tool == ToolMode::Resize){
        moveResizeGroup();
    }
    update();
}

void CanvasWidget::clearRedoStack() { images[activeIndex].redoStack.clear(); }

void CanvasWidget::resetImage() {
    saveState();
    m_image() = r_image;
    o_image() = r_image;
    update();
}

void CanvasWidget::applyGrayScaleFilter() {
    if (m_image().isNull())
        return;

    saveState();

    QImage img = m_image().convertToFormat(QImage::Format_ARGB32);
    int bytesPerLine = img.bytesPerLine();
    uchar *bits = img.bits();

    QVector <int> rows;
            for (int y = start_y; y < end_y; y++)
        rows.append(y);

    int threadCount = std::max(1, QThreadPool::globalInstance()->maxThreadCount() - 2);
    QThreadPool::globalInstance()->setMaxThreadCount(threadCount);
    QtConcurrent::blockingMap(rows,[&](int y){
    uchar *line = bits + y * bytesPerLine;
    for (int x = start_x; x < end_x; x++) {
        uchar *pixel = line + x * 4;
        uchar blue = pixel[0];
        uchar green = pixel[1];
        uchar red = pixel[2];
        uchar gray = (red + green + blue) / 3;
        pixel[0] = pixel[1] = pixel[2] = gray;
        }
});

    QVector<int> o_rows;

     for (int y = o_y_s; y < o_y_e; y++)
        o_rows.append(y);

     QThreadPool::globalInstance()->setMaxThreadCount(threadCount);

     QImage o_img = m_image().convertToFormat(QImage::Format_ARGB32);
     int o_bytesPerLine = o_img.bytesPerLine();
     uchar *o_bits = o_img.bits();

     QtConcurrent::blockingMap(o_rows,[&](int y){
     uchar *line = o_bits + y * o_bytesPerLine;
    for (int x = o_x_s; x < o_x_e; x++) {
             uchar *pixel = line + x*4;
        uchar blue = pixel[0];
        uchar green = pixel[1];
        uchar red = pixel[2];
        uchar gray = (red + green + blue) / 3;
        pixel[0] = pixel[1] = pixel[2] = gray;
        }
    }
);
    m_image() = img;
    o_image() = o_img;
    update(); // redraw on screen
}

void CanvasWidget::applyInversionFilter() {
    if (m_image().isNull())
        return;

    saveState();

    QImage result = m_image();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            color.setRgb(255 - color.red(), 255 - color.green(), 255 - color.blue());
            result.setPixelColor(x, y, color);
        }
    }

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            QColor color = o_image().pixelColor(x, y);
            color.setRgb(255 - color.red(), 255 - color.green(), 255 - color.blue());
            o_image().setPixelColor(x, y, color);
        }
    }
    m_image() = result;
    update();
}

void CanvasWidget::applyVeriticalReflection() {
    if (m_image().isNull())
        return;

    saveState();

    QImage result = m_image();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y / 2; y++) {

            // QColor color_1 = result.pixelColor(x, y);
            // QColor color_2 = result.pixelColor(x, m_image().height() - 1 - y);

            // std::swap(color_1,color_2);

            QRgb temp = result.pixel(x, y);

            result.setPixel(x, y, result.pixel(x, end_y - 1 - y));

            result.setPixel(x, end_y - 1 - y, temp);
        }
    }


    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e / 2; y++) {

            QRgb temp = o_image().pixel(x, y);

            o_image().setPixel(x, y, o_image().pixel(x, o_y_e - 1 - y));

            o_image().setPixel(x, o_y_e - 1 - y, temp);
        }
    }

    m_image() = result;
    update();
};
void CanvasWidget::applyHorizontalReflection() {

    if (m_image().isNull())
        return;

    saveState();

    QImage result = m_image();

    for (int x = start_x; x < end_x / 2; x++) {
        for (int y = start_y; y < end_y; y++) {

            // QColor color_1 = result.pixelColor(x, y);
            // QColor color_2 = result.pixelColor(x, m_image().height() - 1 - y);

            // std::swap(color_1,color_2);

            QRgb temp = result.pixel(x, y);

            result.setPixel(x, y, result.pixel(end_x - 1 - x, y));

            result.setPixel(end_x - 1 - x, y, temp);
        }
    }

    for (int x = o_x_s; x < o_x_e / 2; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {

            QRgb temp = o_image().pixel(x, y);

            o_image().setPixel(x, y, o_image().pixel(o_x_e - x - 1, y));

            o_image().setPixel(o_x_e - x - 1, y, temp);
        }
    }

    m_image() = result;
    update();
};

void CanvasWidget::applyYellowFilter(const int intensity, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();

    QImage result = m_image();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = m_baseImage.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() + intensity, 0, 255),
                         std::clamp(color.blue() - intensity, 0, 255));
            result.setPixelColor(x, y, color);
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    if (o_image().isNull())
        return;

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            QColor color = o_baseImage.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() + intensity, 0, 255),
                         std::clamp(color.blue() - intensity, 0, 255));
            o_image().setPixelColor(x, y, color);
        }
    }
    m_image() = result;
    update();
}

void CanvasWidget::applyPurpleFilter(const int intensity, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();
    QImage result = m_image();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = m_baseImage.pixelColor(x, y);

            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() - intensity, 0, 255),
                         std::clamp(color.blue() + intensity, 0, 255));
            result.setPixelColor(x, y, color);
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    if (o_image().isNull())
        return;

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            QColor color = o_baseImage.pixelColor(x, y);
            color.setRgb(std::clamp(color.red() + intensity, 0, 255),
                         std::clamp(color.green() - intensity, 0, 255),
                         std::clamp(color.blue() + intensity, 0, 255));
            o_image().setPixelColor(x, y, color);
        }
    }
    m_image() = result;
    update();
}

void CanvasWidget::applyInfraRedFilter() {
    applyInversionFilter();
    applyGrayScaleFilter();

    if (m_image().isNull())
        return;

    saveState();

    QImage result = m_image();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = result.pixelColor(x, y);
            color.setRed(255);
            result.setPixelColor(x, y, color);
        }
    }
    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            QColor color = o_image().pixelColor(x, y);
            color.setRed(255);
            o_image().setPixelColor(x, y, color);
        }
    }
    m_image() = result;
    update();
}

void CanvasWidget::applyBlackAndWhiteFilter(int threshold, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();

    QImage result = m_image().copy();


for (int y= start_y; y < end_y; y++) {
    for (int x = start_x; x < end_x; x++) {
            QColor color = m_baseImage.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;

            if (gray >= threshold) {
                color.setRgb(255, 255, 255);
                result.setPixelColor(x, y, color);
            } else {
                color.setRgb(0, 0, 0);
                result.setPixelColor(x, y, color);
            }
        }
        }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    if (o_image().isNull()) return;



                for (int y = o_y_s; y < o_y_e; y++){
    for (int x = o_x_s; x < o_x_e; x++) {
            QColor color = o_baseImage.pixelColor(x, y);
            int gray = (color.red() + color.green() + color.blue()) / 3;

            if (gray >= threshold) {
                color.setRgb(255, 255, 255);
                o_image().setPixelColor(x, y, color);
            } else {
                color.setRgb(0, 0, 0);
                o_image().setPixelColor(x, y, color);
            }
        }
        };

    m_image() = result;
    update(); // redraw on screen
}
    void CanvasWidget::applyBlurFilter(int kernelSize, FilterMode mode) {

        if (m_baseImage.isNull())
            m_baseImage = m_image();
        if (m_image().isNull())
            return;

        if (mode == FilterMode::Increment)
            saveState();

        // Ensure minimum and odd kernel size
        if (kernelSize < 1)
            kernelSize = 1;
        if (kernelSize % 2 == 0)
            kernelSize++;

        int radius = kernelSize / 2;


        QImage result = m_baseImage;
        QImage blurred = result.copy();

            for (int y = start_y; y < end_y; y++) {
                for (int x = start_x; x < end_x; x++) {
                int red = 0, green = 0, blue = 0, count = 0;

                for (int dy = -radius; dy <= radius; dy++) {

                    for (int dx = -radius; dx<= radius; dx++) {
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
        };

        if (o_baseImage.isNull())
            o_baseImage = o_image();
        else if (o_image().isNull())
            return;

        QImage o_result = o_baseImage;
        QImage o_blurred = o_result;




            for (int y = o_y_s; y < o_y_e; y++) {
            for (int x = o_x_s; x < o_x_e; x++) {
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
        m_image() = blurred;
        o_image() = o_blurred;
        update();
    }

void CanvasWidget::applyResizeTool(int newWidth, int newHeight) {
    if (m_image().isNull())
        return;

    if (newWidth <= 0)
        newWidth = m_image().width();
    if (newHeight <= 0)
        newHeight = m_image().height();

    saveState();

    QImage resized(newWidth, newHeight, o_image().format());
    double scaleY = static_cast<double>(o_image().height()) / newHeight;
    double scaleX = static_cast<double>(o_image().width()) / newWidth;

    for (int x = 0; x < resized.width(); x++) {
        for (int y = 0; y < resized.height(); y++) {
            int oldX = std::floor(int(x * scaleX));
            int oldY = std::floor(int(y * scaleY));
            resized.setPixelColor(x, y, o_image().pixelColor(oldX, oldY));
        }
    }

    m_image() = resized;
    moveResizeGroup();
    update();
}
void CanvasWidget::applyEdgeDetection() {

    if (m_image().isNull())
        return;

    const int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    saveState();

    QImage copy = m_image();

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
            QColor color = m_image().pixelColor(y, x);
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
            m_image().setPixelColor(y, x, color);
        }
    }

    // if (o_baseImage.isNull())
    //     o_baseImage = o_image();
    // else if (o_image().isNull())
    //     return;

    QImage o_copy = o_image();

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
            QColor color = o_image().pixelColor(y, x);
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
            o_image().setPixelColor(y, x, color);
        }
    }

    update(); // redraw on screen
};

void CanvasWidget::applyLightOrDarkFilter(int percent, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();

    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            QColor color = m_baseImage.pixelColor(x, y);
            int newR =
                std::clamp(0, 255, color.red() + (color.red() * percent) / 100);
            int newG =
                std::clamp(0, 255, color.green() + (color.green() * percent) / 100);
            int newB =
                std::clamp(0, 255, color.blue() + (color.blue() * percent) / 100);

            color.setRgb(newR, newG, newB);

            m_image().setPixelColor(x, y, color);
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    else if (o_image().isNull())
        return;

    for (int x = o_x_s; x < o_x_e; x++) {
        for (int y = o_y_s; y < o_y_e; y++) {
            QColor color = o_baseImage.pixelColor(x, y);

            int newR =
                std::clamp(0, 255, color.red() + (color.red() * percent) / 100);
            int newG =
                std::clamp(0, 255, color.green() + (color.green() * percent) / 100);
            int newB =
                std::clamp(0, 255, color.blue() + (color.blue() * percent) / 100);

            color.setRgb(newR, newG, newB);
            o_image().setPixelColor(x, y, color);
        }
        update();
    }
}

void CanvasWidget::rotate(QImage &image, int degrees) {
    degrees = ((degrees % 360) + 360) % 360;
    int numRotations = (degrees / 90) % 4;

    for (int r = 0; r < numRotations; r++) {
        QImage rotated(image.height(), image.width(), image.format());

        for (int y = 0; y < image.height(); y++) {
            for (int x = 0; x < image.width(); x++) {
                rotated.setPixelColor(image.height() - 1 - y, x,
                                      image.pixelColor(x, y));
            }
        }
        image = rotated;
    }
}

void CanvasWidget::applyRotateFilter(int degrees) {
    saveState();
    if (m_image().isNull())
        return;
    if (m_lastRotation != 0) {
        rotate(m_image(), m_lastRotation);
        rotate(o_image(), m_lastRotation);
    }
    rotate(m_image(), -degrees);
    rotate(o_image(), -degrees);

    m_lastRotation = degrees;
    moveResizeGroup();
    update();
}

void CanvasWidget::applyFrameFilter(int thickness, int r, int g, int b) {
    if (m_image().isNull())
        return;

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (o_baseImage.isNull())
        o_baseImage = o_image();

    // work from unmodified copies every time until canceled or commited
    QImage result = m_baseImage;
    QImage o_result = o_baseImage;

    int maxThickness = std::min(result.width(), result.height()) / 2;
    if (thickness > maxThickness)
        thickness = maxThickness;

    // Apply frame to displayed image
    for (int y = 0; y < result.height(); y++) {
        for (int x = 0; x < result.width(); x++) {
            bool inFrame = (y < thickness || y >= result.height() - thickness ||
                            x < thickness || x >= result.width() - thickness);
            if (inFrame)
                result.setPixelColor(x, y, QColor(r, g, b));
        }
    }

    // Apply frame to original-scale image
    int o_thickness = thickness * o_result.width() / result.width();
    for (int y = 0; y < o_result.height(); y++) {
        for (int x = 0; x < o_result.width(); x++) {
            bool inFrame = (y < o_thickness || y >= o_result.height() - o_thickness ||
                            x < o_thickness || x >= o_result.width() - o_thickness);
            if (inFrame)
                o_result.setPixelColor(x, y, QColor(r, g, b));
        }
    }

    // Apply the result for display
    m_image() = result;
    o_image() = o_result;
    update();
}

void CanvasWidget::applyTvNoiseFilter() {
    if (m_image().isNull())
        return;

    saveState();

    QImage result = m_image();

    // Create a seed for randomization.
    std::random_device rnd;
    std::mt19937 gen(rnd());
    std::uniform_int_distribution<> distr(-40, 40); // random noise range

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {

            QColor color = result.pixelColor(x, y);

            int r = std::clamp(color.red() + distr(gen), 0, 255);
            int g = std::clamp(color.green() + distr(gen), 0, 255);
            int b = std::clamp(color.blue() + distr(gen), 0, 255);

            // Half brightness for every 4th group of rows
            if (y % 8 == 4 || y % 8 == 5 || y % 8 == 6 || y % 8 == 7) {
                r /= 2;
                g /= 2;
                b /= 2;
            }

            color.setRgb(r, g, b);
            result.setPixelColor(x, y, color);
        }
    }

    for (int y = o_y_s; y < o_y_e; y++) {
        for (int x = o_x_s; x < o_x_e; x++) {
            QColor color = o_image().pixelColor(x, y);

            int r = std::clamp(color.red() + distr(gen), 0, 255);
            int g = std::clamp(color.green() + distr(gen), 0, 255);
            int b = std::clamp(color.blue() + distr(gen), 0, 255);

            if (y % 8 == 4 || y % 8 == 5 || y % 8 == 6 || y % 8 == 7) {
                r /= 2;
                g /= 2;
                b /= 2;
            }

            color.setRgb(r, g, b);
            o_image().setPixelColor(x, y, color);
        }
    }

    m_image() = result;
    update(); // redraw on screen
}

void CanvasWidget::applyOilPaintFilter(int kernelSize,int levels, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();

    // Ensure minimum kernel size
    // Default kernelSize set to -1
    if (kernelSize < 1)
        // Just default preference
        kernelSize = 5 * m_image().width() * m_image().height() / (1024 * 768);

    // Ensure odd kernel size for symmetry around center pixel
    if (kernelSize % 2 == 0)
        kernelSize++;
    int radius = kernelSize / 2; // Distance from center to edge of kernel

    // Create a copy of the original image to read from during processing
    QImage copy = m_baseImage;
    QImage result = copy;

    for (int row = start_y; row < end_y; row++) {
        for (int col = start_x; col < end_x; col++) {

            int mx = 0;
            int intensityLvl[256] = {0}, aR[256] = {0}, aG[256] = {0}, aB[256] = {0};

            for (int dr = -radius; dr <= radius; dr++) {
                for (int dc = -radius; dc <= radius; dc++) {

                    int nr = row + dr; // Neighbor row
                    int nc = col + dc; // Neighbor column

                    // Check if neighbor coordinates are within image bounds
                    if (nr >= 0 && nr < m_image().height() && nc >= 0 &&
                        nc < m_image().width()) {
                        QColor c = copy.pixelColor(nc, nr);
                        int cur =
                            (int)((double)((c.red() + c.green() + c.blue()) / 3) * levels) /
                                  255.0f;
                        intensityLvl[cur]++;
                        aR[cur] += c.red();
                        aG[cur] += c.green();
                        aB[cur] += c.blue();
                        if (intensityLvl[cur] > intensityLvl[mx])
                            mx = cur;
                    }
                }
            }
            result.setPixelColor(
                col, row,
                QColor(std::round((float)aR[mx] / intensityLvl[mx]),
                       std::round((float)aG[mx] / intensityLvl[mx]),
                                 std::round((float)aB[mx] / intensityLvl[mx])));
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    else if (o_image().isNull())
        return;

    QImage o_copy = o_baseImage;
    QImage o_result = o_copy;

    for (int row = o_y_s; row < o_y_e; row++) {
        for (int col = o_x_s; col < o_x_e; col++) {

            int mx = 0;
            int intensityLvl[256] = {0}, aR[256] = {0}, aG[256] = {0}, aB[256] = {0};

            for (int dr = -radius; dr <= radius; dr++) {
                for (int dc = -radius; dc <= radius; dc++) {

                    int nr = row + dr; // Neighbor row
                    int nc = col + dc; // Neighbor column

                    // Check if neighbor coordinates are within image bounds
                    if (nr >= 0 && nr < o_image().height() && nc >= 0 &&
                        nc < o_image().width()) {
                        QColor c = o_copy.pixelColor(nc, nr);
                        int cur =
                            (int)((double)((c.red() + c.green() + c.blue()) / 3) * levels) /
                                  255.0f;
                        intensityLvl[cur]++;
                        aR[cur] += c.red();
                        aG[cur] += c.green();
                        aB[cur] += c.blue();
                        if (intensityLvl[cur] > intensityLvl[mx])
                            mx = cur;
                    }
                }
            }
            o_result.setPixelColor(
                col, row,
                QColor(std::round((float)aR[mx] / intensityLvl[mx]),
                       std::round((float)aG[mx] / intensityLvl[mx]),
                                 std::round((float)aB[mx] / intensityLvl[mx])));
        }
    }

    m_image() = result;
    o_image() = o_result;
    update();
}

void CanvasWidget::applySkewFilter(double degree, FilterMode mode) {

    if (m_baseImage.isNull())
        m_baseImage = m_image();
    if (m_image().isNull())
        return;

    if (mode == FilterMode::Increment)
        saveState();

    const double pi = acos(-1);
    int wdth = static_cast<int>(tan(pi * degree / 180.0) * m_baseImage.height() +
                                m_baseImage.width());

    QImage result(wdth, m_baseImage.height(), m_baseImage.format());
    result.fill(Qt::white);

    for (int row = 0; row < m_baseImage.height(); row++) {
        for (int col = 0; col < m_baseImage.width(); col++) {
            int newX = static_cast<int>(col + tan(pi * degree / 180.0) *
                                                  (m_baseImage.height() - row));
            if (newX >= 0 && newX < result.width()) {
                QColor c = m_baseImage.pixelColor(col, row);
                result.setPixelColor(newX, row, c);
            }
        }
    }

    if (o_baseImage.isNull())
        o_baseImage = o_image();
    else if (o_image().isNull())
        return;

    int o_wdth = static_cast<int>(
        tan(pi * degree / 180.0) * o_baseImage.height() + o_baseImage.width());
    QImage o_result(o_wdth, o_baseImage.height(), o_baseImage.format());
    o_result.fill(Qt::white);

    for (int row = 0; row < o_baseImage.height(); row++) {
        for (int col = 0; col < o_baseImage.width(); col++) {
            int newX = static_cast<int>(col + tan(pi * degree / 180.0) *
                                                  (o_baseImage.height() - row));
            if (newX >= 0 && newX < o_result.width()) {
                QColor c = o_baseImage.pixelColor(col, row);
                o_result.setPixelColor(newX, row, c);
            }
        }
    }

    m_image() = result;
    o_image() = o_result;
    moveResizeGroup();
    update();
}

bool CanvasWidget::isMImageNull() {
    if (m_image().isNull())
        return true;
    return false;
}

void CanvasWidget::commitChanges() {
    m_baseImage = QImage();
    o_baseImage = QImage();
}

void CanvasWidget::cancelChanges() {
    if (!m_baseImage.isNull()) {
        m_image() = m_baseImage;
        m_baseImage = QImage();
        update();
    }
    if (!o_baseImage.isNull()) {
        o_image() = o_baseImage;
        o_baseImage = QImage();
        update();
    }
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

QImage CanvasWidget::exportCanvas() {
    QImage exported(canvasRect.size(), QImage::Format_ARGB32);
    exported.fill(Qt::transparent);
    QPainter p(&exported);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    for (const Layer &layer : images) {
        if (!layer.visible)
            continue;
        p.drawImage(layer.offset, layer.image);
    }
    p.end();
    return exported;
}
