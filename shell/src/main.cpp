/*
 * lwindesk - shell/src/main.cpp - Shell entry point (Qt6/QML)
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "shellmanager.h"
#include "taskbarmodel.h"
#include "startmenumodel.h"
#include "notificationmanager.h"
#include "systemtraymanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("lwindesk-shell");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("lwindesk");

    /* Register C++ types for QML */
    qmlRegisterType<ShellManager>("LWinDesk.Shell", 1, 0, "ShellManager");
    qmlRegisterType<TaskbarModel>("LWinDesk.Shell", 1, 0, "TaskbarModel");
    qmlRegisterType<StartMenuModel>("LWinDesk.Shell", 1, 0, "StartMenuModel");
    qmlRegisterType<NotificationManager>("LWinDesk.Shell", 1, 0,
                                          "NotificationManager");
    qmlRegisterType<SystemTrayManager>("LWinDesk.Shell", 1, 0,
                                        "SystemTrayManager");

    QQmlApplicationEngine engine;

    /* Create shell manager (handles IPC with compositor) */
    ShellManager shellManager;
    engine.rootContext()->setContextProperty("shellManager", &shellManager);

    engine.loadFromModule("LWinDesk", "Main");

    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load QML shell");
        return -1;
    }

    return app.exec();
}
