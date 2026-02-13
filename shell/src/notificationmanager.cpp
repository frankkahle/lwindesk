/*
 * lwindesk - shell/src/notificationmanager.cpp
 */

#include "notificationmanager.h"

NotificationManager::NotificationManager(QObject *parent)
    : QAbstractListModel(parent) {
    /* TODO: Register as org.freedesktop.Notifications on D-Bus */
}

int NotificationManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_notifications.count();
}

QVariant NotificationManager::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= m_notifications.count())
        return QVariant();

    const Notification &n = m_notifications[index.row()];
    switch (role) {
    case IdRole:        return n.id;
    case AppNameRole:   return n.appName;
    case SummaryRole:   return n.summary;
    case BodyRole:      return n.body;
    case IconNameRole:  return n.iconName;
    case TimestampRole: return n.timestamp;
    }
    return QVariant();
}

QHash<int, QByteArray> NotificationManager::roleNames() const {
    return {
        {IdRole, "notifId"},
        {AppNameRole, "appName"},
        {SummaryRole, "summary"},
        {BodyRole, "body"},
        {IconNameRole, "iconName"},
        {TimestampRole, "timestamp"},
    };
}

void NotificationManager::dismiss(uint32_t id) {
    for (int i = 0; i < m_notifications.count(); ++i) {
        if (m_notifications[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_notifications.removeAt(i);
            endRemoveRows();
            emit countChanged();
            return;
        }
    }
}

void NotificationManager::clearAll() {
    beginResetModel();
    m_notifications.clear();
    endResetModel();
    emit countChanged();
}
