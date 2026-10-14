#include "IconService.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QDir>

QIcon IconService::loadIcon(const QString &name)
{
    QString exeDir = QCoreApplication::applicationDirPath();
    QString filePath = exeDir + "/assets/icons/" + name;

    if (QFile::exists(filePath)) {
        QIcon icon(filePath);
        if (!icon.isNull()) {
            return icon;
        }
    }

    QString fallbackPath = exeDir + "/icons/" + name;
    if (QFile::exists(fallbackPath)) {
        QIcon icon(fallbackPath);
        if (!icon.isNull()) {
            return icon;
        }
    }

    qWarning() << "Icon not found:" << name << " searched in" << filePath;
    return QIcon();
}