/*
 * lwindesk - shell/src/notificationmanager.h - D-Bus notification handler
 */

#ifndef LWINDESK_NOTIFICATIONMANAGER_H
#define LWINDESK_NOTIFICATIONMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QDateTime>

struct Notification {
    uint32_t id;
    QString appName;
    QString summary;
    QString body;
    QString iconName;
    QDateTime timestamp;
};

class NotificationManager : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AppNameRole,
        SummaryRole,
        BodyRole,
        IconNameRole,
        TimestampRole,
    };

    explicit NotificationManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_notifications.count(); }

    Q_INVOKABLE void dismiss(uint32_t id);
    Q_INVOKABLE void clearAll();

signals:
    void countChanged();
    void newNotification(uint32_t id, const QString &summary,
                          const QString &body);

private:
    QVector<Notification> m_notifications;
    uint32_t m_nextId = 1;

    /* TODO: Connect to org.freedesktop.Notifications D-Bus interface */
};

#endif /* LWINDESK_NOTIFICATIONMANAGER_H */
