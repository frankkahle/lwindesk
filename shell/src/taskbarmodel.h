/*
 * lwindesk - shell/src/taskbarmodel.h - Taskbar window list model
 */

#ifndef LWINDESK_TASKBARMODEL_H
#define LWINDESK_TASKBARMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct TaskbarEntry {
    QString appId;
    QString title;
    QString iconName;
    bool active;
    bool minimized;
    bool pinned;
};

class TaskbarModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        AppIdRole = Qt::UserRole + 1,
        TitleRole,
        IconNameRole,
        ActiveRole,
        MinimizedRole,
        PinnedRole,
    };

    explicit TaskbarModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void activateWindow(int index);
    Q_INVOKABLE void closeWindow(int index);
    Q_INVOKABLE void pinApp(const QString &appId);

private:
    QVector<TaskbarEntry> m_entries;
};

#endif /* LWINDESK_TASKBARMODEL_H */
