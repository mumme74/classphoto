#ifndef STUDENTLISTITEMDELEGATE_H
#define STUDENTLISTITEMDELEGATE_H

#include <QStyledItemDelegate>
class Project;

class StudentListItemDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    explicit StudentListItemDelegate(Project *project, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:

public slots:

private:
    Project *m_project;

};

#endif // STUDENTLISTITEMDELEGATE_H
