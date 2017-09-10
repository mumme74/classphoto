#ifndef STUDENTINGRAPHICSVIEW_H
#define STUDENTINGRAPHICSVIEW_H

#include <QGraphicsObject>

class Project;

class StudentInGraphicsView : public QGraphicsObject
{
Q_OBJECT
public:
    explicit StudentInGraphicsView(Project *project, const QString key, QGraphicsObject *parent = 0);

    QRectF boundingRect() const;

    void studentNameChanged(const QString newName);
    void studentPixmapChanged(const QPixmap *newPixmap);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
protected:
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

signals:

public slots:

private:
    QRect nameRect(const QString text, QPainter *painter);
    Project *m_project;
    QString m_key;
    bool    m_isOver;
    bool    m_isDragged;
    QSize    m_pixmapSize;
    QString  m_name;
    const QPixmap *m_pixmap;
};

#endif // STUDENTINGRAPHICSVIEW_H
