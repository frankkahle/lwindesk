/*
 * lwindesk - shell/src/iconprovider.cpp - Freedesktop icon theme provider
 */

#include "iconprovider.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QDebug>

IconThemeProvider::IconThemeProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap),
      m_adwaitaPath(QStringLiteral("/usr/share/icons/Adwaita")) {
}

QPixmap IconThemeProvider::requestPixmap(const QString &id, QSize *size,
                                         const QSize &requestedSize) {
    /* Parse optional color parameter: "icon-name?color=white" */
    QString iconName = id;
    QColor tintColor;

    int queryIdx = id.indexOf('?');
    if (queryIdx >= 0) {
        iconName = id.left(queryIdx);
        QString params = id.mid(queryIdx + 1);
        for (const QString &param : params.split('&')) {
            if (param.startsWith("color=")) {
                tintColor = QColor(param.mid(6));
            }
        }
    }

    int sz = 32;
    if (requestedSize.isValid() && requestedSize.width() > 0)
        sz = requestedSize.width();

    QPixmap result;

    /* 1. Try QIcon::fromTheme() first - this is the standard approach */
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        QPixmap pm = icon.pixmap(sz, sz);
        if (!pm.isNull())
            result = pm;
    }

    /* 2. Fallback: manually scan the Adwaita icon directory */
    if (result.isNull())
        result = findFallbackIcon(iconName, sz);

    /* 3. Nothing found - return a transparent pixmap so QML doesn't error */
    if (result.isNull()) {
        qWarning() << "IconThemeProvider: icon not found:" << iconName;
        result = QPixmap(sz, sz);
        result.fill(Qt::transparent);
    }

    /* Apply tint color if requested */
    if (tintColor.isValid() && !result.isNull())
        result = tintPixmap(result, tintColor);

    if (size)
        *size = result.size();
    return result;
}

QPixmap IconThemeProvider::tintPixmap(const QPixmap &src, const QColor &color) {
    QPixmap tinted(src.size());
    tinted.fill(Qt::transparent);

    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, src);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    painter.end();

    return tinted;
}

QPixmap IconThemeProvider::findFallbackIcon(const QString &name, int sz) {
    if (!QDir(m_adwaitaPath).exists())
        return QPixmap();

    /*
     * Search for SVG or PNG files matching the icon name.
     * Adwaita stores icons in subdirectories like:
     *   scalable/actions/icon-name.svg
     *   16x16/actions/icon-name.png
     *   symbolic/actions/icon-name-symbolic.svg
     */
    QStringList patterns;
    patterns << name + ".svg" << name + ".png";

    /* Also try with -symbolic suffix if not already present */
    if (!name.endsWith("-symbolic")) {
        patterns << name + "-symbolic.svg" << name + "-symbolic.png";
    }

    /* Prefer scalable SVGs first */
    QStringList searchDirs;
    searchDirs << "scalable" << "symbolic";

    /* Add sized directories, preferring closest to requested size */
    QDir adwaita(m_adwaitaPath);
    QStringList allDirs = adwaita.entryList(QDir::Dirs | QDir::NoDotAndDotDot,
                                            QDir::Name);
    for (const QString &d : allDirs) {
        if (!searchDirs.contains(d))
            searchDirs.append(d);
    }

    for (const QString &subDir : searchDirs) {
        QString dirPath = m_adwaitaPath + "/" + subDir;
        if (!QDir(dirPath).exists())
            continue;

        QDirIterator it(dirPath, patterns,
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();

            if (filePath.endsWith(".svg", Qt::CaseInsensitive)) {
                /* Render SVG at the requested size */
                QSvgRenderer renderer(filePath);
                if (renderer.isValid()) {
                    QPixmap pm(sz, sz);
                    pm.fill(Qt::transparent);
                    QPainter painter(&pm);
                    renderer.render(&painter);
                    painter.end();
                    return pm;
                }
            } else {
                /* PNG - load and scale */
                QPixmap pm(filePath);
                if (!pm.isNull()) {
                    return pm.scaled(sz, sz, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
                }
            }
        }
    }

    return QPixmap();
}
