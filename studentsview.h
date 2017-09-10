#ifndef STUDENTSVIEW_H
#define STUDENTSVIEW_H

#include <QGraphicsView>

class Project;

class StudentsView : public QGraphicsView
{
Q_OBJECT
public:
    explicit StudentsView(QWidget *parent = 0);

    void init(Project *project);
    void setScene(QGraphicsScene *scene);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void showEvent(QShowEvent *event);


signals:

public slots:
    void scaleViewToScene();

private:
    Project *m_project;
};

#endif // STUDENTSVIEW_H
