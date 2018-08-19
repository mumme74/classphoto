#ifndef CLASSNAMEINGRAPHICSVIEW_H
#define CLASSNAMEINGRAPHICSVIEW_H

#include <QGraphicsTextItem>

class Project;

class ClassNameInGraphicsView : public QGraphicsTextItem
{
Q_OBJECT
public:
    explicit ClassNameInGraphicsView(Project *project, QGraphicsObject *parent = nullptr);
    ~ClassNameInGraphicsView();

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:

public slots:

private:
    Project *m_project;
    bool m_isDragged;

};

#endif // CLASSNAMEINGRAPHICSVIEW_H
