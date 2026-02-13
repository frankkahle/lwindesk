/*
 * lwindesk - shell/src/startmenumodel.h - Start menu application model
 */

#ifndef LWINDESK_STARTMENUMODEL_H
#define LWINDESK_STARTMENUMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct AppEntry {
    QString name;
    QString iconName;
    QString exec;
    QString category;
    bool pinned;
};

class StartMenuModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery
               NOTIFY searchQueryChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IconNameRole,
        ExecRole,
        CategoryRole,
        PinnedRole,
    };

    explicit StartMenuModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    Q_INVOKABLE void loadApplications();
    Q_INVOKABLE void pinApp(int index);

signals:
    void searchQueryChanged();

private:
    QVector<AppEntry> m_allApps;
    QVector<AppEntry> m_filteredApps;
    QString m_searchQuery;
    void filterApps();
};

#endif /* LWINDESK_STARTMENUMODEL_H */
