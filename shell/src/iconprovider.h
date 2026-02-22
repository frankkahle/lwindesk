/*
 * lwindesk - shell/src/iconprovider.h - Freedesktop icon theme provider for QML
 */

#ifndef LWINDESK_ICONPROVIDER_H
#define LWINDESK_ICONPROVIDER_H

#include <QQuickImageProvider>
#include <QIcon>
#include <QPixmap>
#include <QDir>

/*
 * IconThemeProvider - QQuickImageProvider that resolves freedesktop icon names.
 *
 * Usage in QML:
 *   Image { source: "image://icon/icon-name" }
 *   Image { source: "image://icon/icon-name?color=white" }
 *
 * The optional "?color=<colorname>" suffix tints the icon to the given color,
 * preserving alpha. This is useful for showing dark-theme icons on a dark
 * taskbar background.
 *
 * Resolution order:
 *   1. QIcon::fromTheme() (uses the configured theme, e.g. Adwaita)
 *   2. Manual scan of /usr/share/icons/Adwaita/ for SVG/PNG fallback
 *   3. Returns a transparent pixmap if nothing is found
 */
class IconThemeProvider : public QQuickImageProvider {
public:
    IconThemeProvider();

    QPixmap requestPixmap(const QString &id, QSize *size,
                          const QSize &requestedSize) override;

private:
    QPixmap findFallbackIcon(const QString &name, int sz);
    QPixmap tintPixmap(const QPixmap &src, const QColor &color);
    QString m_adwaitaPath;
};

#endif /* LWINDESK_ICONPROVIDER_H */
