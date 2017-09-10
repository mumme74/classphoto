#ifndef PICTURE_H
#define PICTURE_H

#include <QString>
#include <QPixmap>


class QPointF;

class Picture : public QObject
{
public:
    Picture (QObject *parent);
    ~Picture ();

    static void doBrightness(QImage *image, qreal brightness);
    void setPixmap(const QPixmap &originalPixmap);

    const QPixmap *originalPixmap() const { return m_originalPixmap; }
    const QPixmap *pixmap();

    void setScale(qreal scaleFactor);
    void setRotation(int rotation);
    void setViewPort(QRect viewPort);
    void setBrightness(qreal brightness);
    void setPlaced(bool placed) { m_placed = placed; };
    bool isPlaced() const { return m_placed; }
    void setPos(const QPointF point);
    const QPointF pos() const { return m_pos; }
    void setRotationPoint(const QPointF point);
    const QPointF rotationPoint() const { return m_rotationPoint; }

    void setDefaultProperties();




    const QRect viewPort() const { return m_viewPort; }
    qreal scaleFactor() const { return m_scaleFactor; }
    qreal brightness() const { return m_brightness; }
    int   rotation() const { return m_rotation; }

private:
    void initClass();
    void rebuildCurrentPixmap();

    QPixmap *m_originalPixmap;
    QPixmap *m_currentPixmap;
    QRect    m_viewPort;
    qreal    m_scaleFactor;
    qreal    m_brightness;
    quint16  m_rotation;
    QPointF  m_pos;
    QPointF  m_rotationPoint;
    bool     m_placed;

    bool     m_hasChanges;
};


#endif // PICTURE_H
