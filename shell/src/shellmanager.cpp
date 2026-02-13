/*
 * lwindesk - shell/src/shellmanager.cpp - Shell manager implementation
 */

#include "shellmanager.h"
#include <QProcess>
#include <QTimer>
#include <QDateTime>

ShellManager::ShellManager(QObject *parent)
    : QObject(parent) {
    /* Update clock every second */
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &ShellManager::currentTimeChanged);
    m_clockTimer->start(1000);
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
