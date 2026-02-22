/*
 * lwindesk - shell/src/shellmanager.h - Shell manager (IPC with compositor)
 */

#ifndef LWINDESK_SHELLMANAGER_H
#define LWINDESK_SHELLMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QLocalSocket>

class ShellManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int activeWorkspace READ activeWorkspace
               NOTIFY activeWorkspaceChanged)
    Q_PROPERTY(bool startMenuVisible READ startMenuVisible
               WRITE setStartMenuVisible NOTIFY startMenuVisibleChanged)
    Q_PROPERTY(bool searchFocusRequested READ searchFocusRequested
               NOTIFY searchFocusRequestedChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText
               NOTIFY searchTextChanged)
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

    bool searchFocusRequested() const { return m_searchFocusRequested; }

    QString searchText() const { return m_searchText; }
    void setSearchText(const QString &text);

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
    void openSearch();
    void openTerminal();
    void clearSearchFocusRequest();

signals:
    void activeWorkspaceChanged();
    void startMenuVisibleChanged();
    void searchFocusRequestedChanged();
    void searchTextChanged();
    void notificationCenterVisibleChanged();
    void quickSettingsVisibleChanged();
    void currentTimeChanged();
    void snapZoneChanged(const QString &zone);

private slots:
    void onIpcConnected();
    void onIpcDisconnected();
    void onIpcReadyRead();
    void onIpcError(QLocalSocket::LocalSocketError error);

private:
    void connectToCompositor();
    void handleIpcCommand(const QString &command);

    int m_activeWorkspace = 0;
    bool m_startMenuVisible = false;
    bool m_searchFocusRequested = false;
    QString m_searchText;
    bool m_notificationCenterVisible = false;
    bool m_quickSettingsVisible = false;
    class QTimer *m_clockTimer;

    /* IPC connection to compositor */
    QLocalSocket *m_ipcSocket = nullptr;
    QByteArray m_ipcBuffer;
    class QTimer *m_reconnectTimer = nullptr;
};

#endif /* LWINDESK_SHELLMANAGER_H */
