#ifndef CROPRENDERER_H
#define CROPRENDERER_H

#include <QGraphicsObject>

class CropRenderer : public QGraphicsObject
{
Q_OBJECT
public:
    explicit CropRenderer(QGraphicsObject *parent = nullptr);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setInnerRect(QRectF innerRect);
    void setCropRect(QRectF cropRect);
    void setCenter(const QPointF center);
    void setPolygon(const QPolygonF poly);

signals:

public slots:

private:

    QRectF m_innerRect;
    QRectF m_cropRect;
    QRectF m_boundingRect;
    QPointF m_center;
    QPolygonF m_poly;
};

#endif // CROPRENDERER_H
