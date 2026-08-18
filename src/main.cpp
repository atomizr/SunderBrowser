#include <QApplication>
#include "ui/MainWindow.h"
#include "controllers/BrowserController.h"
#include "models/SettingsModel.h"
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Atomizr");
    app.setApplicationName("SunderBrowser");
    app.setApplicationVersion("1.0.0");

    // Иконка окна – загружаем из файловой системы (там же, где и остальные иконки)
    QString iconPath = QCoreApplication::applicationDirPath() + "/assets/icons/app.ico";
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
    } else {
        // Если нет – попробуем просто "app.ico" в корне (старый вариант)
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