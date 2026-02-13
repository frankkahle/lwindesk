/*
 * lwindesk - shell/src/systemtraymanager.cpp
 */

#include "systemtraymanager.h"

SystemTrayManager::SystemTrayManager(QObject *parent)
    : QAbstractListModel(parent) {
}

int SystemTrayManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 0;
}

QVariant SystemTrayManager::data(const QModelIndex &index, int role) const {
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

QHash<int, QByteArray> SystemTrayManager::roleNames() const {
    return {
        {IdRole, "trayId"},
        {TitleRole, "title"},
        {IconNameRole, "iconName"},
        {TooltipRole, "tooltip"},
    };
}
