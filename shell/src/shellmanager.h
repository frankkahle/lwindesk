/*
 * lwindesk - shell/src/shellmanager.h - Shell manager (IPC with compositor)
 */

#ifndef LWINDESK_SHELLMANAGER_H
#define LWINDESK_SHELLMANAGER_H

#include <QObject>
#include <QDateTime>

class ShellManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int activeWorkspace READ activeWorkspace
               NOTIFY activeWorkspaceChanged)
    Q_PROPERTY(bool startMenuVisible READ startMenuVisible
               WRITE setStartMenuVisible NOTIFY startMenuVisibleChanged)
    Q_PROPERTY(bool notificationCenterVisible READ notificationCenterVisible
               WRITE setNotificationCenterVisible
               NOTIFY notificationCenterVisibleChanged)
    Q_PROPERTY(bool quickSettingsVisible READ quickSettingsVisible
               WRITE setQuickSettingsVisible
               NOTIFY quickSettingsVisibleChanged)
    Q_PROPERTY(QString currentTime READ currentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY currentTimeChanged)

public:
    explicit ShellManager(QObject *parent = nullptr);

    int activeWorkspace() const { return m_activeWorkspace; }

    bool startMenuVisible() const { return m_startMenuVisible; }
    void setStartMenuVisible(bool visible);

    bool notificationCenterVisible() const { return m_notificationCenterVisible; }
    void setNotificationCenterVisible(bool visible);

    bool quickSettingsVisible() const { return m_quickSettingsVisible; }
    void setQuickSettingsVisible(bool visible);

    QString currentTime() const;
    QString currentDate() const;

public slots:
    void switchWorkspace(int index);
    void showDesktop();
    void lockScreen();
    void launchApp(const QString &command);
    void toggleStartMenu();

signals:
    void activeWorkspaceChanged();
    void startMenuVisibleChanged();
    void notificationCenterVisibleChanged();
    void quickSettingsVisibleChanged();
    void currentTimeChanged();
    void snapZoneChanged(const QString &zone);

private:
    int m_activeWorkspace = 0;
    bool m_startMenuVisible = false;
    bool m_notificationCenterVisible = false;
    bool m_quickSettingsVisible = false;
    class QTimer *m_clockTimer;
};

#endif /* LWINDESK_SHELLMANAGER_H */
