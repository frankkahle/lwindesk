/*
 * lwindesk - shell/src/startmenumodel.cpp - Start menu model implementation
 */

#include "startmenumodel.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <algorithm>

StartMenuModel::StartMenuModel(QObject *parent)
    : QAbstractListModel(parent) {
    loadApplications();
}

void StartMenuModel::loadApplications() {
    beginResetModel();
    m_allApps.clear();

    /* Scan .desktop files from standard locations */
    QStringList paths = QStandardPaths::standardLocations(
        QStandardPaths::ApplicationsLocation);
    paths.append("/usr/share/applications");
    paths.append("/usr/local/share/applications");

    QSet<QString> seen;

    for (const QString &path : paths) {
        QDir dir(path);
        for (const QString &file : dir.entryList({"*.desktop"}, QDir::Files)) {
            if (seen.contains(file)) continue;
            seen.insert(file);

            QFile f(dir.filePath(file));
            if (!f.open(QIODevice::ReadOnly)) continue;

            AppEntry entry;
            bool inDesktopEntry = false;
            bool noDisplay = false;

            QTextStream stream(&f);
            while (!stream.atEnd()) {
                QString line = stream.readLine().trimmed();
                if (line == "[Desktop Entry]") {
                    inDesktopEntry = true;
                    continue;
                }
                if (line.startsWith("[") && line != "[Desktop Entry]") {
                    inDesktopEntry = false;
                    continue;
                }
                if (!inDesktopEntry) continue;

                if (line.startsWith("Name="))
                    entry.name = line.mid(5);
                else if (line.startsWith("Icon="))
                    entry.iconName = line.mid(5);
                else if (line.startsWith("Exec="))
                    entry.exec = line.mid(5)
                        .remove("%u").remove("%U")
                        .remove("%f").remove("%F").trimmed();
                else if (line.startsWith("Categories="))
                    entry.category = line.mid(11).split(';').first();
                else if (line.startsWith("NoDisplay=true"))
                    noDisplay = true;
            }

            if (!entry.name.isEmpty() && !entry.exec.isEmpty() && !noDisplay) {
                m_allApps.append(entry);
            }
        }
    }

    /* Sort alphabetically */
    std::sort(m_allApps.begin(), m_allApps.end(),
              [](const AppEntry &a, const AppEntry &b) {
                  return a.name.toLower() < b.name.toLower();
              });

    m_filteredApps = m_allApps;
    endResetModel();
}

int StartMenuModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_filteredApps.count();
}

QVariant StartMenuModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= m_filteredApps.count())
        return QVariant();

    const AppEntry &entry = m_filteredApps[index.row()];
    switch (role) {
    case NameRole:     return entry.name;
    case IconNameRole: return entry.iconName;
    case ExecRole:     return entry.exec;
    case CategoryRole: return entry.category;
    case PinnedRole:   return entry.pinned;
    }
    return QVariant();
}

QHash<int, QByteArray> StartMenuModel::roleNames() const {
    return {
        {NameRole, "name"},
        {IconNameRole, "iconName"},
        {ExecRole, "exec"},
        {CategoryRole, "category"},
        {PinnedRole, "pinned"},
    };
}

void StartMenuModel::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        filterApps();
        emit searchQueryChanged();
    }
}

void StartMenuModel::filterApps() {
    beginResetModel();
    if (m_searchQuery.isEmpty()) {
        m_filteredApps = m_allApps;
    } else {
        m_filteredApps.clear();
        for (const AppEntry &entry : m_allApps) {
            if (entry.name.contains(m_searchQuery, Qt::CaseInsensitive)) {
                m_filteredApps.append(entry);
            }
        }
    }
    endResetModel();
}

void StartMenuModel::pinApp(int index) {
    if (index >= 0 && index < m_filteredApps.count()) {
        /* TODO: persist pinned apps */
    }
}
