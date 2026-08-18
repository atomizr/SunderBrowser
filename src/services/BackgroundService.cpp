#include "BackgroundService.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QByteArray>
#include <QStringList>

QString BackgroundService::getRandomBackgroundStyle()
{
    // Путь к папке с фонами (относительно директории приложения)
    QString backgroundsPath = QCoreApplication::applicationDirPath() + "/assets/backgrounds/";
    QStringList imageFiles;
    for (int i = 1; i <= 10; ++i) {
        imageFiles << QString("image_%1.jpg").arg(i);
    }

    // Выбираем случайное изображение
    QString selectedImage;
    if (!imageFiles.isEmpty()) {
        int randomIndex = QRandomGenerator::global()->bounded(imageFiles.size());
        selectedImage = imageFiles[randomIndex];
    }

    if (!selectedImage.isEmpty()) {
        QString fullPath = backgroundsPath + selectedImage;
        QFile imageFile(fullPath);
        if (imageFile.open(QIODevice::ReadOnly)) {
            QByteArray imageData = imageFile.readAll();
            QString base64 = QString::fromLatin1(imageData.toBase64());
            return QString(
                "background-image: url('data:image/jpeg;base64,%1'); "
                "background-size: cover; "
                "background-position: center; "
                "background-repeat: no-repeat;"
            ).arg(base64);
        }
    }

    // Дефолтный градиент, если изображение не загружено
    return "background: radial-gradient(120% 90% at 85% -10%, rgba(72,28,132,0.75) 0%, transparent 55%), "
           "radial-gradient(130% 130% at 12% -5%, #170b31 0%, #070312 62%); "
           "background-size: cover;";
}