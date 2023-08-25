#include "classnameingraphicsview.h"
#include "project.h"
#include <QKeyEvent>
#include <QDebug>
#include <QTextDocument>
#include <QTextCursor>

#define FONT "arial [sans serif]"
#define FONTSIZE 48

ClassNameInGraphicsView::ClassNameInGraphicsView(Project *project,  QGraphicsObject *parent) :
    QGraphicsTextItem(parent),
    m_project(project),
    m_isDragged(false)
{
    setPlainText(m_project->className());
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    setTextInteractionFlags(Qt::TextEditorInteraction);

    QFont myFont;
    myFont.setFamily(FONT);
    myFont.setBold(true);
    myFont.setPointSize(FONTSIZE);
    setFont(myFont);
    setToolTip(tr("Du kan få ny rad genom att hålla nere shift och sedan trycka på enter."));

    setZValue(1.0);
}

ClassNameInGraphicsView::~ClassNameInGraphicsView()
{
}

void ClassNameInGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && event->modifiers() != Qt::ShiftModifier) {
        // stop editing
        event->setAccepted(false);
        clearFocus();
        setSelected(false);
        m_project->setClassName(toPlainText());
        return;
    }
    QGraphicsTextItem::keyPressEvent(event);
    return;
}

void ClassNameInGraphicsView::focusInEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::focusInEvent(event);
}

void ClassNameInGraphicsView::focusOutEvent(QFocusEvent *event)
{
    if (toPlainText() != m_project->className())
        m_project->setClassName(toPlainText());

    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        cursor.clearSelection();
        setTextCursor(cursor);
    }

    setSelected(false);

    QGraphicsTextItem::focusOutEvent(event);
}


void ClassNameInGraphicsView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragged = true;
    QGraphicsObject::mouseMoveEvent(event);
}

void ClassNameInGraphicsView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragged) {
        // save position
        m_project->updateClassNamePosition();
    }

    m_isDragged = false;
    QGraphicsObject::mouseReleaseEvent(event);
}
