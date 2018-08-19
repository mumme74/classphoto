#ifndef CROPPICTURE_H
#define CROPPICTURE_H

#include <QGraphicsObject>

class QGraphicsPixmapItem;
class Picture;
class CropRenderer;

class CropPicture : public QGraphicsObject
{
Q_OBJECT
public:
    explicit CropPicture(Picture *pic, QGraphicsObject *parent = nullptr);
    ~CropPicture();

    const QRect cropRect() const;
    void setCropRect(const QRect visibleArea);

    QRectF boundingRect() const { return m_boundingRect; }

    void update();
    void findLargestVisible();
    void findViewPortSize();
    qreal rotation() const;
    void show();
    void setScale(qreal scale);
    qreal scale() const;
    void centerView();

    const QPointF rotationPoint() const;
    void setRotationPoint(const QPointF pointRelativeToCenter);


    void setBrightness(qreal brightness);
    void setRotation(int rotation);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);


signals:

public slots:
    void doBrightness();

private:
    CropRenderer *m_renderer;
    Picture *m_pic;
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsPixmapItem *m_pivot;
    QRectF m_visibleRect;
    QRectF m_boundingRect;
    QRectF m_cropRect;
    QPointF m_dragStart;
    QPointF m_lastMove;
    QPointF m_movedTo;
    QTimer *m_brightnessTimer;
    int m_rotation;
    int m_scale;
    qreal m_brightness;
    bool m_dragInProgress;

};

#endif // CROPPICTURE_H
