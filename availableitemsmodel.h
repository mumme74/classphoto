#ifndef AVAILABLEITEMSMODEL_H
#define AVAILABLEITEMSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QMap>

class Project;

class AvailableItemsModel : public QAbstractListModel
{
Q_OBJECT
public:
    explicit AvailableItemsModel(Project *project, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void updateRow(const QString key);
    void setVisible(const QString key, bool visible);
    bool isVisible(const QString key) const { return !m_invisibleKeys.contains(key); }

    bool insertRowsFromList(int position, const QStringList newKeys, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int count, const QModelIndex &parent = QModelIndex());

signals:

public slots:

private:
    QStringList m_keys;
    Project *m_project;
    QMap<QString, int> m_invisibleKeys;
};

#endif // AVAILABLEITEMSMODEL_H
