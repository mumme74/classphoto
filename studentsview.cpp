#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDebug>
#include "studentsview.h"
#include "project.h"


StudentsView::StudentsView(QWidget *parent) :
    QGraphicsView(parent)
{
    this->setAcceptDrops(true);
}


void StudentsView::setScene(QGraphicsScene *scene)
{
    QGraphicsView::setScene(scene);
}


void StudentsView::showEvent(QShowEvent *event)
{
    event->accept();
    scaleViewToScene();
}

void StudentsView::init(Project *project)
{
    m_project = project;
}

void StudentsView::scaleViewToScene()
{
    scene()->setSceneRect(scene()->itemsBoundingRect()); // reset sceneRect
    QRectF rect(scene()->sceneRect());

    //rect.setX(rect.x() - GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN);
    rect.setWidth(rect.width() + GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN * 2);
    //rect.setY(rect.y() - GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN);
    rect.setHeight(rect.height() + GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN * 2);

    if (rect.width() < 300) {
        rect.setWidth(300);
    }
    if (rect.height() < 200) {
        rect.setHeight(200);
    }
    fitInView(rect, Qt::KeepAspectRatio);
}

void StudentsView::dropEvent(QDropEvent *event)
{

    qDebug()<< event->mimeData()->formats().at(0) <<endl;
    if (event->source() == this)
        return;

    if (event->mimeData()->formats().at(0) == "application/vnd.student") {
        QPointF point(this->mapToScene(event->pos()));

        QByteArray encodedData = event->mimeData()->data("application/vnd.student");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QStringList newItems;
        int rows = 0;

        while (!stream.atEnd()) {
          QString text;
          stream >> text;
          newItems << text;
          ++rows;
        }

        foreach(QString key, newItems) {
            m_project->addStudentToGraphicsView(key, point);
        }
    }

}

void StudentsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.student")) {
        event->acceptProposedAction();
        qDebug()<< event->mimeData()->formats().at(0) <<endl;
    }
}

void StudentsView::dragMoveEvent(QDragMoveEvent *event)
{
    Q_UNUSED(event);
}
