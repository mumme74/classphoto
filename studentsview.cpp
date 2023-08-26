#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QMenu>
#include "studentsview.h"
#include "studentingraphicsview.h"
#include "studentnamedialog.h"
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

void StudentsView::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    auto student = dynamic_cast<StudentInGraphicsView*>(itemAt(e->pos()));
    static QAction removeStudent(QIcon(":/window-close.png"), tr("Ta bort elev"));
    static QAction insertNoPic(QIcon(":/image-x-generic.png"), tr("Sätt in elev utan bild"));

    if (student)
        menu.addAction(&removeStudent);
    else
        menu.addAction(&insertNoPic);
    // run menu
    auto res = menu.exec(e->globalPos());
    if (res == &removeStudent && student)
        m_project->removeStudentFromGraphicsView(student);
    if (res == &insertNoPic) {
        StudentNameDialog nameDlg(this);
        if (nameDlg.exec() == QDialog::Accepted)
            m_project->insertPlaceHolder(nameDlg.name(),
                                         QPointF(mapToScene(e->pos())));
    }
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

    qDebug()<< event->mimeData()->formats().at(0) << Qt::endl;
    if (event->source() == this)
        return;

    if (event->mimeData()->formats().at(0) == "application/vnd.student") {
        QPointF point(this->mapToScene(event->position().toPoint()));

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
        qDebug()<< event->mimeData()->formats().at(0) << Qt::endl;
    }
}

void StudentsView::dragMoveEvent(QDragMoveEvent *event)
{
    Q_UNUSED(event)
}
