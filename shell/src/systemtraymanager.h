/*
 * lwindesk - shell/src/systemtraymanager.h - System tray (SNI protocol)
 */

#ifndef LWINDESK_SYSTEMTRAYMANAGER_H
#define LWINDESK_SYSTEMTRAYMANAGER_H

#include <QObject>
#include <QAbstractListModel>

class SystemTrayManager : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        IconNameRole,
        TooltipRole,
    };

    explicit SystemTrayManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /* TODO: Implement StatusNotifierHost D-Bus interface */
};

#endif /* LWINDESK_SYSTEMTRAYMANAGER_H */
