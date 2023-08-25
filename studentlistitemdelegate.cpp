#include "studentlistitemdelegate.h"
#include "project.h"
#include "picture.h"
#include "availableitemsmodel.h"
#include <QPainter>


StudentListItemDelegate::StudentListItemDelegate(Project *project, QObject *parent) :
    QStyledItemDelegate(parent),
    m_project(project)
{

}

void StudentListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem tempOption;
    initStyleOption(&tempOption, index);

    QString key = index.data(Qt::DisplayRole).toString();
    if (qobject_cast<const AvailableItemsModel *>(index.model())->isVisible(key)) {

        painter->save();

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());


        Picture *pic = m_project->picturesInDir()->value(key);
        if (!pic)
            return;

        const QPixmap *pix = pic->pixmap();
        QPixmap pix2 = pix->scaledToHeight(option.rect.height() - 10);


        painter->translate(option.rect.x(), option.rect.y());
        painter->drawPixmap(5, 5, pix2);


        painter->setRenderHint(QPainter::Antialiasing, true);
        if (option.state & QStyle::State_Selected)
            painter->setBrush(option.palette.highlightedText());
        else
            painter->setBrush(option.palette.text());

        QFont myFont;
        QFontMetrics fm(myFont);

        painter->drawText(pix2.width() + 10, 5, fm.horizontalAdvance(key), fm.height(), Qt::AlignBottom, key);

        QString name = m_project->knownNames()->value(key);
        if (!name.isEmpty()) {
            painter->drawText(pix2.width() + 10, fm.height() + 5, fm.horizontalAdvance(name), fm.height(), Qt::AlignBottom, name);
        }

        painter->restore();
    }
}

QSize StudentListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    QString key = index.data(Qt::DisplayRole).toString();
    if (qobject_cast<const AvailableItemsModel *>(index.model())->isVisible(key)) {
        Picture *pic = m_project->picturesInDir()->value(key);
        if (!pic)
            return QSize(0,0);
        int maxHeight = qMin(100, pic->pixmap()->height() + 10);
        int maxWidth = qMin(75,  pic->pixmap()->width() + 10);

        QString name = m_project->knownNames()->value(key);
        if (!name.isEmpty()) {
            QFont myFont;
            QFontMetrics fm(myFont);

            maxHeight = qMax(maxHeight, fm.height());
            maxWidth = qMax(maxWidth + 5, fm.horizontalAdvance(name) + 5);
        }
        return QSize(maxWidth, maxHeight);
    }
    // invisible
    return QSize(0, 0);
}

void StudentListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    editor->move(editor->x() + 130, editor->y());
}
