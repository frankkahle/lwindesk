/*
 * lwindesk - shell/src/shellmanager.cpp - Shell manager implementation
 */

#include "shellmanager.h"
#include <QProcess>
#include <QTimer>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

ShellManager::ShellManager(QObject *parent)
    : QObject(parent) {
    /* Update clock every second */
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &ShellManager::currentTimeChanged);
    m_clockTimer->start(1000);

    /* Set up IPC socket to compositor */
    m_ipcSocket = new QLocalSocket(this);
    connect(m_ipcSocket, &QLocalSocket::connected,
            this, &ShellManager::onIpcConnected);
    connect(m_ipcSocket, &QLocalSocket::disconnected,
            this, &ShellManager::onIpcDisconnected);
    connect(m_ipcSocket, &QLocalSocket::readyRead,
            this, &ShellManager::onIpcReadyRead);
    connect(m_ipcSocket, &QLocalSocket::errorOccurred,
            this, &ShellManager::onIpcError);

    /* Reconnect timer for when compositor IPC isn't ready yet */
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &ShellManager::connectToCompositor);

    /* Connect to compositor IPC on startup */
    connectToCompositor();
}

QString ShellManager::currentTime() const {
    return QDateTime::currentDateTime().toString("h:mm AP");
}

QString ShellManager::currentDate() const {
    return QDateTime::currentDateTime().toString("M/d/yyyy");
}

void ShellManager::setStartMenuVisible(bool visible) {
    if (m_startMenuVisible != visible) {
        m_startMenuVisible = visible;
        /* Close other panels when start menu opens */
        if (visible) {
            setNotificationCenterVisible(false);
            setQuickSettingsVisible(false);
        }
        emit startMenuVisibleChanged();
    }
}

void ShellManager::setNotificationCenterVisible(bool visible) {
    if (m_notificationCenterVisible != visible) {
        m_notificationCenterVisible = visible;
        if (visible) {
            setStartMenuVisible(false);
            setQuickSettingsVisible(false);
        }
        emit notificationCenterVisibleChanged();
    }
}

void ShellManager::setQuickSettingsVisible(bool visible) {
    if (m_quickSettingsVisible != visible) {
        m_quickSettingsVisible = visible;
        if (visible) {
            setStartMenuVisible(false);
            setNotificationCenterVisible(false);
        }
        emit quickSettingsVisibleChanged();
    }
}

void ShellManager::switchWorkspace(int index) {
    if (m_activeWorkspace != index) {
        m_activeWorkspace = index;
        /* TODO: Send IPC to compositor to switch workspace */
        emit activeWorkspaceChanged();
    }
}

void ShellManager::showDesktop() {
    /* TODO: Send IPC to compositor to minimize all windows */
}

void ShellManager::lockScreen() {
    /* TODO: Activate lock screen overlay */
}

void ShellManager::launchApp(const QString &command) {
    QProcess *proc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("DISPLAY");
    env.insert("QT_QPA_PLATFORM", "wayland");
    env.insert("GDK_BACKEND", "wayland");
    proc->setProcessEnvironment(env);
    proc->setProgram("/bin/sh");
    proc->setArguments({"-c", command});
    proc->startDetached();
    proc->deleteLater();
    setStartMenuVisible(false);
}

void ShellManager::toggleStartMenu() {
    setStartMenuVisible(!m_startMenuVisible);
}

void ShellManager::setSearchText(const QString &text) {
    if (m_searchText != text) {
        m_searchText = text;
        emit searchTextChanged();
    }
}

void ShellManager::openSearch() {
    m_searchFocusRequested = true;
    emit searchFocusRequestedChanged();
    setStartMenuVisible(true);
}

void ShellManager::clearSearchFocusRequest() {
    if (m_searchFocusRequested) {
        m_searchFocusRequested = false;
        emit searchFocusRequestedChanged();
    }
}

void ShellManager::openTerminal() {
    launchApp("foot");
}

/* --- IPC connection to compositor --- */

void ShellManager::connectToCompositor() {
    if (m_ipcSocket->state() == QLocalSocket::ConnectedState) return;

    /* Build the socket path: $XDG_RUNTIME_DIR/lwindesk-ipc */
    QString runtimeDir = QStandardPaths::writableLocation(
        QStandardPaths::RuntimeLocation);
    if (runtimeDir.isEmpty()) {
        runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    }
    if (runtimeDir.isEmpty()) {
        runtimeDir = QStringLiteral("/run/user/1000");
    }

    QString socketPath = runtimeDir + QStringLiteral("/lwindesk-ipc");
    qDebug("ShellManager: connecting to IPC at %s",
           qPrintable(socketPath));
    m_ipcSocket->connectToServer(socketPath);
}

void ShellManager::onIpcConnected() {
    qDebug("ShellManager: IPC connected to compositor");
    m_ipcBuffer.clear();
}

void ShellManager::onIpcDisconnected() {
    qDebug("ShellManager: IPC disconnected, will retry in 2s");
    m_ipcBuffer.clear();
    m_reconnectTimer->start(2000);
}

void ShellManager::onIpcError(QLocalSocket::LocalSocketError error) {
    Q_UNUSED(error);
    qDebug("ShellManager: IPC error: %s",
           qPrintable(m_ipcSocket->errorString()));
    /* Retry connection after a delay */
    if (m_ipcSocket->state() != QLocalSocket::ConnectedState) {
        m_reconnectTimer->start(2000);
    }
}

void ShellManager::onIpcReadyRead() {
    m_ipcBuffer.append(m_ipcSocket->readAll());

    /* Process complete newline-delimited commands */
    while (true) {
        int idx = m_ipcBuffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_ipcBuffer.left(idx).trimmed();
        m_ipcBuffer.remove(0, idx + 1);

        if (!line.isEmpty()) {
            handleIpcCommand(QString::fromUtf8(line));
        }
    }
}

void ShellManager::handleIpcCommand(const QString &command) {
    qDebug("ShellManager: IPC command received: %s",
           qPrintable(command));

    if (command == QStringLiteral("toggle-start-menu")) {
        toggleStartMenu();
    } else if (command == QStringLiteral("show-desktop")) {
        showDesktop();
    } else if (command == QStringLiteral("cycle-window")) {
        /* Alt+Tab notification from compositor.
         * The compositor handles the actual window focus change;
         * the shell can update any task switcher UI here. */
        qDebug("ShellManager: window cycle event");
    } else {
        qDebug("ShellManager: unknown IPC command: %s",
               qPrintable(command));
    }
}
