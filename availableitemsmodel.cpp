#include "availableitemsmodel.h"
#include "project.h"
#include <QMimeData>

AvailableItemsModel::AvailableItemsModel(Project *project, QObject *parent) :
    QAbstractListModel(parent),
    m_project(project)
{
}

int AvailableItemsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_keys.count();
}

QVariant AvailableItemsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_keys.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_keys.at(index.row());
    else if(role == Qt::EditRole)
        return m_project->knownName(m_keys.at(index.row()));
    else
        return QVariant();
}

QVariant AvailableItemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return tr("Kolumn %1").arg(section);
    else
        return tr("Row %1").arg(section);
}

Qt::ItemFlags AvailableItemsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}

bool AvailableItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_project->setKnownName(m_keys.at(index.row()), value.toString());
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QStringList AvailableItemsModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.student";
    return types;
}

QMimeData *AvailableItemsModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach(QModelIndex index, indexes) {
        if (index.isValid()) {

            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("application/vnd.student", encodedData);
    return mimeData;
}

bool AvailableItemsModel::insertRowsFromList(int position, const QStringList newKeys, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginInsertRows(QModelIndex(), position, position + newKeys.size() - 1);


    for (int i = newKeys.count() - 1; i >= 0; --i) {
        m_keys.insert(position, newKeys.at(i));
    }

    endInsertRows();
    return true;
}

bool AvailableItemsModel::removeRows(int position, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(QModelIndex(), position, position + count - 1);

    for (int row = 0; row < count; ++row) {
        m_keys.removeAt(position);
    }

    endRemoveRows();
    return true;
}


void AvailableItemsModel::updateRow(const QString key)
{
    int idx = m_keys.indexOf(QRegExp(key));
    if (idx != -1) {
        QModelIndex index = createIndex(idx, 0);
        emit dataChanged(index, index);
    }
}

void AvailableItemsModel::setVisible(QString key, bool visible)
{
    int idx = m_keys.indexOf(QRegExp(key));
    if (idx != -1) {
        if (visible && m_invisibleKeys.contains(key)) {
            m_invisibleKeys.remove(key);

            QModelIndex index = createIndex(idx, 0);
            emit dataChanged(index, index);
        } else if (!visible && !m_invisibleKeys.contains(key)) {
            m_invisibleKeys.insert(key, idx);

            QModelIndex index = createIndex(idx, 0);
            emit dataChanged(index, index);
        }
    }
}
