#include "croppicture.h"
#include "croprenderer.h"
#include "project.h"
#include "picture.h"


#include <QGraphicsPixmapItem>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRect>
#include <QDebug>
#include <QPainter>
#include <QVector>
#include <QTimer>
#include "math.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static const qreal aspectRatioFixed = static_cast<qreal>(GRAPHICSVIEW_PIXMAP_WIDTH) /
                                 static_cast<qreal>(GRAPHICSVIEW_PIXMAP_HEIGHT);


CropPicture::CropPicture(Picture *pic, QGraphicsObject *parent) :
    QGraphicsObject(parent),
    m_renderer(nullptr),
    m_pic(pic),
    m_pixmapItem(nullptr),
    m_pivot(nullptr),
    m_lastMove(0,0),
    m_movedTo(0,0),
    m_brightnessTimer(nullptr),
    m_rotation(0),
    m_scale(1),
    m_brightness(0.5),
    m_dragInProgress(false)
{

    m_pivot = new QGraphicsPixmapItem(this);
    m_pivot->setPos(0, 0);

    const QPixmap *pix = m_pic->originalPixmap();
    m_pixmapItem = new QGraphicsPixmapItem(*pix, m_pivot);
    m_pixmapItem->setPos(0, 0);

    qreal height = qMin(pix->width(), pix->height());

    qreal width = height * aspectRatioFixed;
    qreal x = pix->width() - width;
    qreal y = pix->height() - height;
    x = x > 0 ? x / 2 : 0;
    y = y > 0 ? y / 2 : 0;
    m_boundingRect = QRect(static_cast<int>(x),
                           static_cast<int>(y),
                           static_cast<int>(width),
                           static_cast<int>(height));

    QPointF boundingCenter = mapToScene(m_boundingRect.center());
    QPointF pixCenter = mapToScene(m_pixmapItem->boundingRect().center());
    QPointF placePos = boundingCenter - pixCenter;

    if (pix->width() < pix->height()) {
        m_pivot->setX(placePos.x());
        m_pivot->setY(placePos.y());
    }

    m_pivot->setTransformOriginPoint(m_pixmapItem->boundingRect().center());

    m_renderer = new CropRenderer(this);

    m_pixmapItem->setTransformOriginPoint(m_pixmapItem->boundingRect().center());

    m_movedTo = m_pixmapItem->pos();

    //setZValue(1.0);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    update();
}

CropPicture::~CropPicture()
{
    delete m_pixmapItem;
    if (m_brightnessTimer)
        delete m_brightnessTimer;


}

void CropPicture::show()
{
    if (scene() && scene()->views().at(0))
        scene()->views().at(0)->centerOn(this);
    update();
    QGraphicsObject::show();
}

void CropPicture::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(painter);
   Q_UNUSED(option);
   Q_UNUSED(widget);
}

void CropPicture::centerView()
{
    if (scene() && scene()->views().at(0)) {
        scene()->views().at(0)->centerOn(m_pixmapItem);
    }
}

void CropPicture::setCropRect(QRect visibleArea)
{
    m_cropRect.setX(visibleArea.x());
    m_cropRect.setY(visibleArea.y());
    m_cropRect.setWidth(visibleArea.width());
    m_cropRect.setHeight(visibleArea.height());

    m_renderer->setCropRect(m_visibleRect);
}

const QRect CropPicture::cropRect() const
{
//    QRectF rect(m_visibleRect);
//    rect = rect.normalized();
//    qreal factor = static_cast<qreal>(rect.width()) / static_cast<qreal>(m_pixmapItem->pixmap().width() * m_pixmapItem->scale());
//    QRect newRect((rect.x() * factor) + m_movedTo.x() , (rect.y() * factor) + m_movedTo.y(),
//                  rect.width() * factor, rect.height() * factor);

    QRect rect(m_cropRect.toRect());
    qreal scaleFactor = m_pixmapItem->scale();
    rect.setX(static_cast<int>(rect.x() / scaleFactor));
    rect.setY(static_cast<int>(rect.y() / scaleFactor));
    rect.setWidth(static_cast<int>(rect.width() / scaleFactor));
    rect.setHeight(static_cast<int>(rect.height() / scaleFactor));
    QPointF center = m_pixmapItem->transformOriginPoint();
    center = rect.center() - center;
    rect.translate(QPoint(0, 0) - center.toPoint());
    qDebug()<<"rect:"<<rect <<endl;
    return rect;

//    QPointF topLeft = m_pixmapItem->mapToItem(this, m_cropRect.topLeft());
//    QPointF topRight = m_pixmapItem->mapToItem(this, m_cropRect.topRight());
//    QPointF bottomLeft = m_pixmapItem->mapToItem(this, m_cropRect.bottomLeft());
//    QPointF bottomRight = m_pixmapItem->mapToItem(this, m_cropRect.bottomRight());
//
//    QPolygon poly;
//    poly.append(topLeft.toPoint());
//    poly.append(topRight.toPoint());
//    poly.append(bottomLeft.toPoint());
//    poly.append(bottomRight.toPoint());
//
//    return poly;
}

void CropPicture::update()
{
    findLargestVisible();
    QGraphicsObject::update();
}

void CropPicture::findLargestVisible()
{


    qreal rotationDegree = m_pivot->rotation();

    // 4 | 1
    // 3 | 2
    int quadrant = 1;
    if (rotationDegree >= 90 && rotationDegree < 180)
        quadrant = 2;
    else if (rotationDegree >= 180 && rotationDegree < 270)
        quadrant = 3;
    else if (rotationDegree >= 270 && rotationDegree < 360)
        quadrant = 4;

    int rotation90 = static_cast<int>(rotationDegree);
    while(rotation90 > 90) rotation90 -= 90;
    while(rotation90 < 0) rotation90 += 90;

    qreal rotation = (270 - rotationDegree) * M_PI / 180;


    QRect pixRect = m_pixmapItem->pixmap().rect();
    qreal outerWidth = pixRect.width();
    qreal outerHeight = pixRect.height();
    qreal innerWidth = 0;
    qreal innerHeight = 0;
    qreal minRadie = qMin(outerWidth, outerHeight) / 2;

    // idea for this algorithm inspired from here
    // http://www.physicsforums.com/showthread.php?t=311203
    //  outerWidth = innerHeight*sin(rotation) + innerWidth*cos(rotation)
    //  outerHeight = innerWidth*sin(rotation) + innerHeight*cos(rotation)
    //
    // solver
    // http://en.allexperts.com/q/Advanced-Math-1363/unknown-variable-equations.htm
    // http://en.wikipedia.org/wiki/Cramer's_rule
    //  cramers rule, matrix
    //  [a b]
    //  [c d]
    //  a = sin(rotation)innerHeight
    //  b = cos(rotation)innerWidth
    //  c = cos(rotation)innerHeight
    //  d = sin(rotation)innerWidth
    //  [sin(rotation)innerHeight cos(rotation)innerWidth]
    //  [cos(rotation)innerHeight sin(rotation)innerWidth]
    //  [sin cos]
    //  [cos sin]

    qreal cos0 = cos(rotation);
    qreal sin0 = sin(rotation);
    cos0 = qMax(cos0, -cos0);
    sin0 = qMax(sin0, -sin0);
    if (cos0 < 0.0000001)
        cos0 = 0.0;
    if (sin0 < 0.0000001)
        sin0 = 0.0;
    qreal a = sin0;
    qreal b = cos0;
    qreal c = cos0;
    qreal d = sin0;
    qreal e = outerWidth;
    qreal f = outerHeight;
    qreal ad = a * d;
    qreal bc = b * c;
    qreal divider = ad - bc;

    innerWidth = (e*d - b*f);
    innerHeight = (a*f - e*c);
    // unsafe to compare a floatingpoint directly against 0
    if (divider > 0.0 || divider < 0.0) {
        innerHeight /= divider;
        innerWidth /= divider;
    }

    innerHeight = qMax(innerHeight, -innerHeight);
    innerWidth = qMax(innerWidth, -innerWidth);

    if (outerWidth > outerHeight) {
        // wider than tall pic
        //qDebug()<<"rotation90:"<<rotation90<<rotationDegree<<" quadrant:"<<quadrant<<" cos0:"<<cos0<<endl;
        if ((rotation90 < 45 && (quadrant == 1 || quadrant == 3)) ||
            (rotation90 > 45 && (quadrant == 2 || quadrant == 4))
        ) {
            if ((sin0 != 0.0 && cos0 != 0.0) && (innerWidth * cos0 >= minRadie)) {
                //qDebug()<<"rotation < 45 && innerWidth * cos0:"<<innerWidth*sin0<< " >= minRadie:"<<(minRadie)<<endl;
                innerWidth = minRadie / cos0;
                innerHeight = minRadie / sin0;
            }
        } else {
            if ((sin0 != 0.0 && cos0 != 0.0) && (innerHeight * sin0 >= minRadie)) {
                //qDebug()<<"rotation >= 45 && innerWidth * cos0:"<<innerWidth*sin0<< " <= minRadie:"<<(minRadie)<<endl;
                innerWidth = minRadie / cos0;
                innerHeight = minRadie / sin0;
            }

        }

        innerHeight = qMax(innerHeight, -innerHeight);
        innerWidth = qMax(innerWidth, -innerWidth);

        if (innerHeight > outerHeight) {
            if (sin0 != 0.0)
                innerWidth = innerWidth + (innerHeight - outerHeight) * sin0;

            innerHeight = outerHeight;
        }

    } else {
        // taller than wide pic

        //qDebug()<<"rotation90:"<<rotation90<<rotationDegree<<" quadrant:"<<quadrant<<" cos0:"<<cos0<<endl;
        if ((rotation90 < 45 && (quadrant == 1 || quadrant == 3)) ||
            (rotation90 > 45 && (quadrant == 2 || quadrant == 4))
        ) {
            if ((sin0 != 0.0 && cos0 != 0.0) && (innerHeight * cos0 >= minRadie)) {
                //qDebug()<<"rotation < 45 && innerWidth * cos0:"<<innerWidth*sin0<< " >= minRadie:"<<(minRadie)<<endl;
                innerWidth = minRadie / sin0;
                innerHeight = minRadie / cos0;
            }
        } else {
            if ((sin0 != 0.0 && cos0 != 0.0) && (innerWidth * sin0 >= minRadie)) {
                //qDebug()<<"rotation >= 45 && innerWidth * cos0:"<<innerWidth*sin0<< " <= minRadie:"<<(minRadie)<<endl;
                innerWidth = minRadie / sin0;
                innerHeight = minRadie / cos0;
            }

        }

        innerHeight = qMax(innerHeight, -innerHeight);
        innerWidth = qMax(innerWidth, -innerWidth);

        if (innerWidth > outerWidth) {
            if (sin0 != 0.0)
                innerHeight = innerHeight + (innerWidth - outerWidth) * sin0;

            innerWidth = outerWidth;
        }
    }

    //qDebug()<<innerWidth << " "<<innerHeight<<endl;

//    m_boundingRect = mapRectFromItem(m_pixmapItem, m_pixmapItem->boundingRect());
//    m_boundingRect.setWidth(qMax(m_boundingRect.width(), m_boundingRect.height()));
//    m_boundingRect.setHeight(m_boundingRect.width());

    qreal halfWidth = innerWidth / 2.0;
    qreal halfHeight = innerHeight / 2.0;
    if (!scene() || !scene()->views().at(0))
        return;
    QPointF center = scene()->views().at(0)->mapToScene(scene()->views().at(0)->viewport()->rect().center());//m_boundingRect.center();
    //center = this->mapFromScene(center);
    //center = center - (m_movedTo / 2) * m_pixmapItem->scale();
    //QPointF center = mapFromItem(m_pixmapItem, m_pixmapItem->pixmap().rect().center());

    qreal top = center.y() - halfHeight;
    qreal left = center.x() - halfWidth;
/*
    qreal halfOuterWidth = (outerWidth * m_pixmapItem->scale()) / 2.0;
    qreal halfOuterHeight = (outerHeight * m_pixmapItem->scale()) / 2.0;
    qreal top = halfOuterHeight - halfHeight;
    qreal left = halfOuterWidth - halfWidth;
*/
    m_visibleRect.setLeft(left);
    m_visibleRect.setTop(top);
    m_visibleRect.setWidth(innerWidth);
    m_visibleRect.setHeight(innerHeight);
    qreal newWidth, newHeight;

    if (aspectRatioFixed >= 1.0) {
       if (innerWidth > innerHeight * aspectRatioFixed) {
            newWidth = innerHeight * aspectRatioFixed;
            newHeight = innerHeight;
        } else {
            newWidth = innerWidth;
            newHeight = innerWidth / aspectRatioFixed;
        }

        if (newHeight > innerHeight) {
            newHeight = innerHeight;
            newWidth = newHeight * aspectRatioFixed;
        }
        if  (newWidth > innerWidth) {
            newWidth = innerWidth;
            newHeight = newWidth / aspectRatioFixed;
        }
    }

    QRectF cropRect(0, 0, newWidth, newHeight);
    QPointF move = m_visibleRect.center() - cropRect.center();
    cropRect.translate(move);
    m_cropRect = cropRect;

    m_renderer->setCropRect(cropRect);

    //setPos(m_pixmapItem->pos());

}


void CropPicture::findViewPortSize()
{
//    qreal rotationDegree = m_pivot->rotation();
//    qreal rotation = (/*270 - */rotationDegree) * M_PI / 180;
//
//    // 4 | 1
//    // 3 | 2
//    int quadrant = 1;
//    if (rotationDegree >= 90 && rotationDegree < 180)
//        quadrant = 2;
//    else if (rotationDegree >= 180 && rotationDegree < 270)
//        quadrant = 3;
//    else if (rotationDegree >= 270 && rotationDegree < 360)
//        quadrant = 4;
//
//    int rotation90 = rotationDegree;
//    while(rotation90 > 90) rotation90 -= 90;
//    while(rotation90 < 0) rotation90 += 90;
//
//
//    qreal outerWidth = m_pixmapItem->pixmap().width();
//    qreal outerHeight = m_pixmapItem->pixmap().height();
//    qreal scaledOuterWidth = outerWidth * m_pixmapItem->scale();
//    qreal scaledOuterHeight = outerHeight * m_pixmapItem->scale();
//    qreal scaledMinSide = qMin(scaledOuterWidth, scaledOuterHeight);
//    qreal scaledMaxSide = scaledMinSide * aspectRatio;
//    qreal minSide = qMin(outerWidth, outerHeight);
//    qreal maxSide = minSide * aspectRatio;
//
//    qreal height = 0;
//    qreal width = 0;
//
//    qreal cos0 = cos(rotation);
//    qreal sin0 = sin(rotation);
//    cos0 = qMax(cos0, -cos0);
//    sin0 = qMax(sin0, -sin0);
//    if (cos0 < 0.0000001)
//        cos0 = 0.0;
//    if (sin0 < 0.0000001)
//        sin0 = 0.0;
//    qreal a = sin0;
//    qreal b = cos0;
//    qreal c = cos0;
//    qreal d = sin0;
//    qreal e = outerWidth;
//    qreal f = outerHeight;
//    qreal ad = a * d;
//    qreal bc = b * c;
//
//    innerWidth = (e*d - b*f);
//    innerHeight = (a*f - e*c);
//    if ((ad - bc) != 0) {
//        innerHeight /= (ad - bc);
//        innerWidth /= (ad - bc);
//    }
//
//    innerHeight = qMax(innerHeight, -innerHeight);
//    innerWidth = qMax(innerWidth, -innerWidth);
//
//    if (quadrant == 4) {
//        height = qMax(scaledMinSide * cos(rotation), scaledMinSide * (1- cos(rotation)));
//        width = height * aspectRatio;
//
//        //height =qAbs(scaledMinSide * cos(rotation)) - qAbs(scaledMinSide * sin(rotation));
//        //width = qAbs(scaledMaxSide * cos(rotation)) - qAbs(scaledMaxSide * sin(rotation));
//        if (width > maxSide)
//            width = maxSide;
//        if (height > minSide)
//            height = minSide;
//
//        if (height < minSide) {
//           width = height * aspectRatio;
//        }
//
//    }
//
//
//
//    QRectF cropRect(0, 0, width, height);
//    QPointF center = scene()->views().at(0)->mapToScene(scene()->views().at(0)->viewport()->rect().center());
//    cropRect.translate(center - cropRect.center());
//
//    m_renderer->setCropRect(cropRect);
}

void CropPicture::setScale(qreal scale)
{
    scale -= m_scale;
    m_scale += scale;

    // unsafe to compare floating point directly against 0
    if (scale > 0.0 || scale < 0.0) {
        qreal scaleFactor = 1.0 + (static_cast<qreal>(scale) / (101.0 - m_scale));
//        m_pixmapItem->setTransformOriginPoint(m_pixmapItem->pixmap().rect().center() + m_movedTo);

        m_pixmapItem->setScale(m_pixmapItem->scale() * scaleFactor);

        //ui->graphicsView->centerOn(m_pixmapItem);
        m_renderer->update();
    }
    //m_pixmapItem->setPos(0, 0);
    update();
}

qreal CropPicture::scale() const
{
    return m_pixmapItem->scale();
}

const QPointF CropPicture::rotationPoint() const
{
    if (m_pivot) {
        QPointF pos = m_pixmapItem->pixmap().rect().center() - m_pixmapItem->transformOriginPoint();
        //pos = m_pixmapItem->scale();
        //pos = mapFromItem(m_pivot, pos);
//        qreal angle = atan2(pos.y(), pos.x());
//        qreal radians = angle + m_pivot->rotation() * M_PI / 180;
//        qreal hyp = sqrt(pos.x()*pos.x() + pos.y()*pos.y());
//        qreal x = hyp * cos(radians);
//        qreal y = hyp * sin(radians);
//        pos = QPointF(x, y);
        return  pos;//m_pivot->mapToParent(m_pixmapItem->pos());
    }

    return QPointF(0, 0);
}

void CropPicture::setRotationPoint(const QPointF point)
{

    QPointF pos = point;
    //pos = mapToItem(m_pivot, pos);
    //pos = m_pixmapItem->scale();
//    qreal angle = atan2(pos.y(), pos.x());
//    qreal radians = angle - m_pivot->rotation() * M_PI / 180;
//    qreal hyp = sqrt(pos.x()*pos.x() + pos.y()*pos.y());
//    qreal x = hyp * cos(radians);
//    qreal y = hyp * sin(radians);
//    pos = QPointF(x, y);

    m_pixmapItem->setPos(pos);
    pos = m_pixmapItem->pixmap().rect().center() - pos;
    m_pixmapItem->setTransformOriginPoint(pos);
    m_movedTo = pos;

    m_renderer->setCenter(mapFromItem(m_pixmapItem, m_pixmapItem->transformOriginPoint()));

    //m_pixmapItem->setPos(mapToItem(m_pivot, point));
    update();

}

qreal CropPicture::rotation() const
{
    return m_pivot->rotation();
}

void CropPicture::setRotation(int rotation)
{
    rotation += 180;
    while(rotation > 360) rotation -= 360;
    while(rotation < 0) rotation += 360;
    if (static_cast<int>(m_pixmapItem->rotation()) != rotation) {

        m_renderer->setCenter(mapFromItem(m_pixmapItem, m_pixmapItem->transformOriginPoint()));
        //m_pixmapItem->setRotation(rotation);
        m_pivot->setRotation(rotation);

        update();
        m_renderer->setInnerRect(m_visibleRect);
    }
}

void CropPicture::doBrightness()
{
    if (m_brightnessTimer) {
        m_brightnessTimer->stop();
        delete m_brightnessTimer;
        m_brightnessTimer = nullptr;
    }

    QImage img = m_pic->originalPixmap()->toImage();

    Picture::doBrightness(&img, m_brightness);
    m_pixmapItem->setPixmap(QPixmap::fromImage(img));
    update();
}

void CropPicture::setBrightness(qreal brightness)
{
    if (m_brightnessTimer) {
        if (m_brightnessTimer->isActive()) {
            m_brightnessTimer->stop();
        }
        delete m_brightnessTimer;
        m_brightnessTimer = nullptr;
    }

    m_brightness = brightness;
    m_brightnessTimer = new QTimer(this);
    connect(m_brightnessTimer, SIGNAL(timeout()), this, SLOT(doBrightness()));
    m_brightnessTimer->start(200);
}


void CropPicture::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {

        m_dragStart = m_pivot->mapToParent(m_pixmapItem->pos()); // event->pos();
        m_lastMove = event->pos();
        m_dragInProgress = true;
    }
}

void CropPicture::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragInProgress) {
        qreal x = (m_lastMove.x() - event->pos().x());
        qreal y = (m_lastMove.y() - event->pos().y());

        QPointF pixPos = m_pivot->mapToParent(m_pixmapItem->pos());
        QPointF newPos = pixPos - QPointF(x, y);

        QPolygonF poly = m_pixmapItem->mapToParent(m_pixmapItem->pixmap().rect());
        poly.translate(newPos);
//        QRectF crop = mapToItem(m_pivot, m_cropRect).boundingRect();


      //  QPolygonF intersected = poly.intersected(crop);
        //intersected.translate(m_pivot->pos());

        //qDebug()<<intersected;

//        m_renderer->setPolygon(intersected);
//        m_renderer->update();
//        if (newPos.x() > pixPos.x()) {
//
//        } else {
//        }



//        if (rect.x() > crop.x())
//            rect.setX(crop.x());
//        if (rect.y() > crop.y())
//            rect.setY(crop.y());
//        if (rect.x() + rect.width() < crop.x() + crop.width())
//            rect.setX(crop.x() + crop.width() - rect.width());
//        if (rect.y() + rect.height() < crop.y() +crop.height())
//            rect.setY(crop.y() + crop.height() - rect.height());

        m_pixmapItem->setPos(mapToItem(m_pivot, newPos));

        m_lastMove = event->pos();
    }
}

void CropPicture::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_dragInProgress) {
            m_dragInProgress = false;

            m_movedTo = QPointF(0, 0) + m_pivot->mapToParent(m_pixmapItem->pos());

            QPointF offsetPos =  this->scenePos() + mapToScene(m_pixmapItem->pos());

            QPointF pixCenter = m_pixmapItem->pixmap().rect().center();

            m_pixmapItem->setTransformOriginPoint(pixCenter - offsetPos);

            m_renderer->setCenter(mapFromItem(m_pixmapItem, m_pixmapItem->transformOriginPoint()));
            rotationPoint();
        }
    }
}

void CropPicture::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    event->accept();
}

void CropPicture::keyPressEvent(QKeyEvent *event)
{
    event->accept();
}

void CropPicture::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();
}
