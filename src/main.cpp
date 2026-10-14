#include <QApplication>
#include "ui/MainWindow.h"
#include "controllers/BrowserController.h"
#include "models/SettingsModel.h"
#include "utils/Logger.h"
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    // --log-level=3 подавляет шумные сообщения Chromium в stderr (INFO, WARNING, ERROR).
    // --use-system-ca-certificates заставляет Chromium использовать системное хранилище сертификатов,
    // что может помочь при корпоративных или антивирусных SSL-сертификатах.
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--log-level=3 "
            "--use-system-ca-certificates");

    QApplication app(argc, argv);
    app.setOrganizationName("Atomizr");
    app.setApplicationName("SunderBrowser");
    app.setApplicationVersion("1.0.0");

    Logger::init("logs");

    QString iconPath = QCoreApplication::applicationDirPath() + "/assets/icons/app.ico";
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
    } else {
        app.setWindowIcon(QIcon("app.ico"));
    }

    SettingsModel settings;
    BrowserController controller(settings);

    MainWindow window(controller);
    window.show();
    window.raise();
    window.activateWindow();

    return app.exec();
}