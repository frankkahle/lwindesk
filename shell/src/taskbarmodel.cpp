/*
 * lwindesk - shell/src/taskbarmodel.cpp - Taskbar model implementation
 */

#include "taskbarmodel.h"

TaskbarModel::TaskbarModel(QObject *parent)
    : QAbstractListModel(parent) {
}

int TaskbarModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_entries.count();
}

QVariant TaskbarModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= m_entries.count())
        return QVariant();

    const TaskbarEntry &entry = m_entries[index.row()];
    switch (role) {
    case AppIdRole:     return entry.appId;
    case TitleRole:     return entry.title;
    case IconNameRole:  return entry.iconName;
    case ActiveRole:    return entry.active;
    case MinimizedRole: return entry.minimized;
    case PinnedRole:    return entry.pinned;
    }
    return QVariant();
}

QHash<int, QByteArray> TaskbarModel::roleNames() const {
    return {
        {AppIdRole, "appId"},
        {TitleRole, "title"},
        {IconNameRole, "iconName"},
        {ActiveRole, "active"},
        {MinimizedRole, "minimized"},
        {PinnedRole, "pinned"},
    };
}

void TaskbarModel::activateWindow(int index) {
    if (index < 0 || index >= m_entries.count()) return;
    /* TODO: Send activate to compositor via IPC */
}

void TaskbarModel::closeWindow(int index) {
    if (index < 0 || index >= m_entries.count()) return;
    /* TODO: Send close to compositor via IPC */
}

void TaskbarModel::pinApp(const QString &appId) {
    Q_UNUSED(appId)
    /* TODO: Pin app to taskbar */
}
