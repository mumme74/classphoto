#include "croprenderer.h"
#include <QVector>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

CropRenderer::CropRenderer(QGraphicsObject *parent) :
    QGraphicsObject(parent),
    m_innerRect(0, 0, 0, 0),
    m_cropRect(0, 0, 0, 0),
    m_boundingRect(0, 0, 0, 0)
{
}

QRectF CropRenderer::boundingRect() const
{
    QGraphicsView *view;
    if (scene()->views().count() > 0) {
        view = scene()->views().at(0);
        return QRectF(view->viewport()->rect());
    }

    return QRectF(-1000, -1000, 2000, 2000);
}

void CropRenderer::setInnerRect(QRectF innerRect)
{
    m_innerRect = innerRect;
    update();
}

void CropRenderer::setCropRect(QRectF cropRect)
{
    m_cropRect = cropRect;
    update();
}

void CropRenderer::setPolygon(QPolygonF poly)
{
    m_poly = poly;
}

void CropRenderer::setCenter(QPointF center)
{
    m_center = center;
    update();
}

void CropRenderer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->save();
    painter->setPen(Qt::darkBlue);
    painter->setBrush(Qt::blue);


    QGraphicsView *view = scene()->views().at(0);

    QRectF viewRect = mapFromScene(view->mapToScene(view->viewport()->rect())).boundingRect();
    painter->setOpacity(0.4);
    painter->setCompositionMode(QPainter::CompositionMode_Xor);

    // draw frames for crop
    painter->drawRect(parentObject()->boundingRect());
    painter->drawRect(viewRect);
    painter->restore();

    painter->drawRect(m_innerRect);
    painter->setPen(Qt::red);
    painter->drawRect(m_cropRect);

    painter->setPen(Qt::black);
    painter->setBrush(Qt::red);
    painter->drawEllipse(mapFromParent(m_center) - QPointF(5, 5), 10, 10);

    painter->setPen(Qt::black);
    painter->setBrush(Qt::blue);
    QPointF viewCenter = view->mapToScene(view->viewport()->rect().center());

    painter->drawLine(static_cast<int>(viewCenter.x() - 20.0),
                      static_cast<int>(viewCenter.y()),
                      static_cast<int>(viewCenter.x() + 20.0),
                      static_cast<int>(viewCenter.y()));
    painter->drawLine(static_cast<int>(viewCenter.x()),
                      static_cast<int>(viewCenter.y() - 20.0),
                      static_cast<int>(viewCenter.x()),
                      static_cast<int>(viewCenter.y() + 20.0));

    painter->setOpacity(0.7);
    painter->drawPolygon(m_poly, Qt::WindingFill);

}
