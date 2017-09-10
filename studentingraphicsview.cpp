#include "studentingraphicsview.h"

#include "project.h"
#include "studenteditdialog.h"

#include <QPainter>
#include <QFontMetrics>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

#define FONT "arial [sans serif]"
#define FONTSIZE 10
#define FONTBOLD false


StudentInGraphicsView::StudentInGraphicsView(Project *project, const QString key, QGraphicsObject *parent) :
    QGraphicsObject(parent),
    m_project(project),
    m_key(key),
    m_isOver(false),
    m_isDragged(false),
    m_pixmapSize(0, 0),
    m_name(""),
    m_pixmap(0)
{

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    studentPixmapChanged(project->pixmapForKey(key));
    QString name = project->knownName(key);
    if (name.isEmpty())
        studentNameChanged(m_key);
    else
        studentNameChanged(name);
}


void StudentInGraphicsView::studentNameChanged(const QString newName)
{
    QFont font;
    font.setFamily(FONT);
    font.setBold(FONTBOLD);
    font.setPointSize(FONTSIZE);

    QFontMetrics fm(font);

    int maxWidth = m_pixmapSize.width() - 20;
    int rows = 1;
    int width = fm.width(newName);


    if (width > maxWidth) {
        QStringList nameParts = newName.split(" ");
        QString sep(' ');
        if (nameParts.count() == 1) {
            // no whitespace in name
            sep.clear();
            nameParts.clear();
            for (int i = 0; i < newName.count(); ++i) {
                nameParts.append(newName.at(i));
            }
        }

        QString tmp = "";
        QString row = "";
        width = 0;

        while (!nameParts.isEmpty()) {
            if (fm.width(row + sep + nameParts.first()) > maxWidth) {
                rows++;
                tmp += row + '\n';

                row.clear();
                continue;
            }
            if (!row.isEmpty())
                row += sep;
            row += nameParts.first();
            nameParts.removeFirst();
        }


        m_name = tmp + row;

    } else {
        m_name = newName;
    }

    update();
}

QRect StudentInGraphicsView::nameRect(QString text, QPainter *painter)
{


    QFontMetrics fm(painter->font(), 0); // use screen pixel size(printer gets messed up otherwise)

    QStringList textParts = text.split('\n');
    int width = 0;
    foreach (QString part, textParts) {
        if (fm.width(part) > width)
            width = fm.width(part);
    }

    // text should be centered on widget
    int x = (m_pixmapSize.width() / 2) - (width / 2) + 1;
    int y = m_pixmapSize.height() - (fm.height() * textParts.count()) + 1;
    QRect nameRect(x, y, width, (fm.height() * textParts.count()));

    return nameRect;
}

void StudentInGraphicsView::studentPixmapChanged(const QPixmap *newPixmap)
{

    m_pixmap = newPixmap;

    m_pixmapSize.setWidth(GRAPHICSVIEW_PIXMAP_WIDTH);
    m_pixmapSize.setHeight(GRAPHICSVIEW_PIXMAP_HEIGHT);

    //qDebug() << newPixmap->width() << " " << m_pixmap->height() << m_pixmapSize.width() << " " << m_pixmapSize.height() <<  " " <<factor <<endl;
    update();
}

QRectF StudentInGraphicsView::boundingRect() const
{

    if (m_pixmap) {
        return QRectF(0, 0, m_pixmapSize.width() + 7, m_pixmapSize.height() + 7);
    }

    return QRectF(0, 0, 10.0, 10.0);
}


void StudentInGraphicsView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    // draw shadow
    QPen shadowPen(QColor(0x28, 0x28, 0x28));
    shadowPen.setWidth(5);
    shadowPen.setStyle(Qt::SolidLine);
    shadowPen.setCapStyle(Qt::FlatCap);
    shadowPen.setJoinStyle(Qt::BevelJoin);
    painter->setPen(shadowPen);
    painter->setBrush(Qt::black);

    painter->drawRect(4, 4, m_pixmapSize.width() - 2, m_pixmapSize.height() - 2);

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

    // draw pixmap
    painter->drawPixmap(2, 2, m_pixmapSize.width(),
                        m_pixmapSize.height(),
                        *m_pixmap);


    // draw name rectangle
    QFont font;
    font.setFamily(FONT);
    font.setBold(FONTBOLD);
    font.setPointSize(FONTSIZE);
    painter->setFont(font);

    QRect rect = nameRect(m_name, painter);
    QRect boxRect(rect);
    boxRect.setX(boxRect.x() - 8);
    boxRect.setY(boxRect.y() - 2);
    boxRect.setHeight(boxRect.height() + 2);
    boxRect.setWidth(boxRect.width() + 16);
    painter->setPen(Qt::gray);
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(boxRect, 5.0, 5.0);

    // draw name, printing messes this up
    painter->save();
    QFontMetrics fm(font);
    qreal scale = static_cast<qreal>(painter->fontMetrics().width("testing for a lengthy string to compare with")) /
                  static_cast<qreal>(fm.width("testing for a lengthy string to compare with"));
    QRect scaledRect(rect.x() * scale, rect.y() * scale, rect.width() * scale, rect.height() * scale);
    painter->scale(1 / scale, 1 / scale);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
    painter->drawText(scaledRect, Qt::AlignCenter, m_name);
    painter->restore();

    // selected rectangle
    QRectF selectedRectangle(1, 1, m_pixmapSize.width() + 1, m_pixmapSize.height() + 1);
    painter->setBrush(Qt::NoBrush);
    if (isSelected()) {
        // first draw a white rect
        QPen selectedPen(Qt::white);
        selectedPen.setCosmetic(true);
        selectedPen.setStyle(Qt::SolidLine);
        painter->setPen(selectedPen);
        painter->drawRect(selectedRectangle);

        // then a black dashed on atop
        selectedPen.setColor(option->palette.Highlight);
        selectedPen.setStyle(Qt::DashLine);
        painter->setPen(selectedPen);
        painter->drawRect(selectedRectangle);
    }
}

void StudentInGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (isSelected() && event->key() == Qt::Key_Delete) {
        m_project->removeStudentFromGraphicsView(m_key);
    }
}

void StudentInGraphicsView::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    setSelected(false);
}

void StudentInGraphicsView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragged = true;
    QGraphicsObject::mouseMoveEvent(event);
}

void StudentInGraphicsView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragged) {
        // save position
        m_project->updateStudentPosition(m_key);
    }
    
    m_isDragged = false;
    QGraphicsObject::mouseReleaseEvent(event);
}

void StudentInGraphicsView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    setSelected(false);
    StudentEditDialog edit(m_project, m_key, m_project->mainWindow());
    //edit.setWindowModality(Qt::WindowModal);
    edit.exec();
}
