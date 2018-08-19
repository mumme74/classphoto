#include "project.h"
#include "picture.h"
#include <QtGlobal>
#include <QImage>
#include <QRgb>
#include <QPainter>
#include <QColor>
#include <math.h>
#include <limits>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#define MAX_WORKING_WIDTH 1280.0

Picture ::Picture(QObject *parent)
    : QObject(parent),
    m_originalPixmap(nullptr),
    m_currentPixmap(nullptr),
    m_placed(false)
{
    initClass();
}

Picture ::~Picture()
{
    delete m_currentPixmap;
    delete m_originalPixmap;
}

void Picture ::initClass()
{
    m_viewPort = QRect();
    m_pos = QPointF();
    m_rotationPoint = QPointF();
    m_scaleFactor = 1.0;
    m_brightness = 0.5;
    m_rotation = 0;
    m_hasChanges = true;
}

/*static*/ void Picture::doBrightness(QImage *image, qreal brightness)
{

    for (int x = 0; x < image->width(); ++x) {
        for (int y = 0; y < image->height(); ++y) {
            QRgb rgb = image->pixel(x, y);
            int red   = qMin(255, static_cast<int>(qRed(rgb) * (brightness * 2)));
            int green = qMin(255, static_cast<int>(qGreen(rgb) * (brightness * 2)));
            int blue  = qMin(255, static_cast<int>(qBlue(rgb) * (brightness * 2)));
            QRgb newRgb = qRgb(red, green, blue);

            image->setPixel(QPoint(x, y), newRgb);
        }
    }
}


void Picture ::setPixmap(const QPixmap &originalPixmap)
{
    if (m_originalPixmap) {
        // reset class
        delete m_originalPixmap;
        m_originalPixmap = nullptr;
        delete m_currentPixmap;
        m_currentPixmap = nullptr;
        initClass();
    }

    m_originalPixmap = new QPixmap(originalPixmap.copy());

    m_hasChanges = true;
}

void Picture ::rebuildCurrentPixmap()
{
    Q_ASSERT(m_originalPixmap);

    if (m_currentPixmap) {
        delete m_currentPixmap;
        m_currentPixmap = nullptr;
    }

    qreal factor = 1.0;

    QImage tmp;
    QRect viewPort = m_viewPort;
    QPointF pos = m_rotationPoint.toPoint();
//    qreal angle = atan2(pos.y(), pos.x());
//    qreal radians = angle - m_rotation * M_PI / 180;
//    qreal hyp = sqrt(pos.x()*pos.x() + pos.y()*pos.y());
//    qreal x = hyp * cos(radians);
//    qreal y = hyp * sin(radians);
//    pos = QPointF(x, y);
    QPoint rotationPoint = m_originalPixmap->rect().center() + pos.toPoint();
    if (m_originalPixmap->width() > MAX_WORKING_WIDTH) {
        tmp = m_originalPixmap->scaledToWidth(MAX_WORKING_WIDTH).toImage();
        factor = MAX_WORKING_WIDTH / static_cast<qreal>(m_originalPixmap->width());

        rotationPoint *= factor;

        if (!viewPort.isEmpty()) {
            viewPort.setX(static_cast<int>(viewPort.x() * factor));
            viewPort.setY(static_cast<int>(viewPort.y() * factor));
            viewPort.setWidth(static_cast<int>(viewPort.width() * factor));
            viewPort.setHeight(static_cast<int>(viewPort.height() * factor));
        }

    } else {
        tmp = m_originalPixmap->toImage();
    }


    // do the brightness
    if (m_brightness != 0.5) {
        Picture::doBrightness(&tmp, m_brightness);
    }

    // do the rotation
    if (m_rotation != 0.0) {
        qreal radians = m_rotation * M_PI / 180;

        int newWidth = static_cast<int>(tmp.width() * qAbs(cos(radians)) + tmp.height() * qAbs(sin(radians)));
        int newHeight = static_cast<int>(tmp.height() * qAbs(cos(radians)) + tmp.width() * qAbs(sin(radians)));

        qreal aspectRatio = static_cast<qreal>(tmp.width()) /
                            static_cast<qreal>(tmp.height());
        static const qreal staticAspectRatio = static_cast<qreal>(GRAPHICSVIEW_PIXMAP_WIDTH) /
                             static_cast<qreal>(GRAPHICSVIEW_PIXMAP_HEIGHT);

        qreal factor = 1.0;

        if (aspectRatio > 1.0) {
            if (newWidth < newHeight * aspectRatio) {
                newWidth = static_cast<int>(newHeight * aspectRatio);
                factor *= aspectRatio;
            } else if (newHeight < newWidth / aspectRatio) {
                newHeight = static_cast<int>(newWidth / aspectRatio);
                factor /= aspectRatio;
            }

            int minWidth = static_cast<int>(tmp.height() * staticAspectRatio);
            if (newWidth < minWidth) {
                factor *= static_cast<qreal>(minWidth) / static_cast<qreal>(newWidth);
                newWidth = minWidth;
                newHeight = static_cast<int>(newWidth * aspectRatio);
            }
        } else {
            // aspectratio is below 1 here
            if (newHeight < newWidth / aspectRatio) {
                newHeight = static_cast<int>(newWidth / aspectRatio);
                factor /= aspectRatio;
            } else if (newWidth < newHeight * aspectRatio) {
                newWidth = static_cast<int>(newHeight * aspectRatio);
                factor *= aspectRatio;
            }

            int minHeight = static_cast<int>(tmp.width() * staticAspectRatio);
            if (newHeight < minHeight) {
                factor *= static_cast<qreal>(minHeight) / static_cast<qreal>(newHeight);
                newHeight = minHeight;
                newWidth = static_cast<int>(newHeight * aspectRatio);
            }
        }

        QRect copyRect(0, 0, static_cast<int>(tmp.rect().width() * factor),
                             static_cast<int>(tmp.rect().height() * factor));
        QRect boundingBox(0, 0, newWidth, newHeight);
        QImage rotated(boundingBox.width(), boundingBox.height(), QImage::Format_RGB32);
        QPainter painter(&rotated);
        painter.translate(boundingBox.center());
        painter.rotate(m_rotation);
        QPoint startPaintPos = QPoint(0, 0) - boundingBox.center();
        painter.translate(startPaintPos);

        painter.drawImage(boundingBox, tmp);

        copyRect.translate(boundingBox.center() - copyRect.center());

        QPainter backPainter(&tmp);
        QRect tmpRect = tmp.rect();
        backPainter.drawImage(tmpRect, rotated, copyRect);
    } // end rotation

    qreal aspectRatioCurrent = static_cast<qreal>(tmp.width()) /
                               static_cast<qreal>(tmp.height());
    static const qreal aspectRatioFixed = static_cast<qreal>(GRAPHICSVIEW_PIXMAP_WIDTH) /
                                          static_cast<qreal>(GRAPHICSVIEW_PIXMAP_HEIGHT);
    QRect clipRect;
    if (aspectRatioCurrent > aspectRatioFixed){
        int width = static_cast<int>(tmp.height() * aspectRatioFixed);
        int x = (tmp.width() - width) / 2;
        int y = 0;
        int height = tmp.height();
        if (!viewPort.isEmpty()) {
            height = viewPort.height();
            width = static_cast<int>(viewPort.width() / aspectRatioFixed);
            x = viewPort.x();
            y = viewPort.y();
        }
        clipRect = QRect(x, y, width, height);
        rotationPoint.setX(rotationPoint.x() - (x));

    } else {

        int width = tmp.width();
        int height = static_cast<int>(tmp.width() / aspectRatioFixed);
        int x = 0;
        int y = (tmp.height() - height) / 2;
        if (!viewPort.isEmpty()) {
            height = static_cast<int>(viewPort.height() / aspectRatioFixed);
            width = viewPort.width();
            x = viewPort.x();
            y = viewPort.y();
        }
        clipRect = QRect(x, y, width, height);
        rotationPoint.setY(rotationPoint.y() - (y));
    }
    //clipRect.translate(QPoint(0, 0) - m_centerPos.toPoint());




    QRect currentRect(0, 0, clipRect.width(), clipRect.height());
    QPixmap *current = new QPixmap(currentRect.width(), currentRect.height());
    QPainter painter(current);
    // white background
    QBrush brush;
    brush.setColor(Qt::white);
    brush.setStyle(Qt::SolidPattern);
    painter.fillRect(currentRect, brush);

    // paint image
    painter.drawImage(currentRect, tmp, clipRect);

    m_currentPixmap = current;

    m_hasChanges = false;
}

const QPixmap * Picture::pixmap()
{
    if (m_hasChanges)
        rebuildCurrentPixmap();
    if (m_originalPixmap)
        return m_currentPixmap;

    // paint a no picture
    QPixmap *tmp = new QPixmap(150, 112);
    QPainter painter(tmp);
    QPen pen(Qt::blue);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawEllipse(75, 61, 50, 50);
    painter.drawLine(40, 96, 110, 26);
    pen.setColor(QColor(Qt::red));
    painter.drawText(10, 90, 130, 20, Qt::AlignCenter, trUtf8("Ingen bild"));
    m_currentPixmap = tmp;

    return m_currentPixmap;
}

void Picture ::setBrightness(qreal brightness)
{
    brightness = qMax(0.1, brightness);
    brightness = qMin(1.0, brightness);
    m_brightness = brightness;
    m_hasChanges = true;
}

void Picture ::setScale(qreal scaleFactor)
{
    scaleFactor = qMax(0.0, scaleFactor);
    scaleFactor = qMin(3.0, scaleFactor);
    m_scaleFactor = scaleFactor;
    m_hasChanges = true;
}

void Picture ::setViewPort(QRect viewPort)
{
    m_viewPort = viewPort;
    m_hasChanges = true;
}

void Picture ::setRotation(int rotation)
{
    // make it within a 360 degree
    while(rotation < 0) rotation += 360;
    while(rotation > 360) rotation -= 360;

    m_rotation = static_cast<quint16>(rotation);
    m_hasChanges = true;
}

void Picture::setPos(const QPointF point)
{
    m_pos = point;
}

void Picture::setRotationPoint(const QPointF point)
{
    m_rotationPoint = point;
    m_hasChanges = true;
}

void Picture::setDefaultProperties()
{
    initClass();
}
