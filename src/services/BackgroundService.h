#ifndef BACKGROUNDSERVICE_H
#define BACKGROUNDSERVICE_H

#include <QString>

class BackgroundService
{
public:
    // Возвращает HTML-стиль для фона (background-image или градиент)
    static QString getRandomBackgroundStyle();
};

#endif // BACKGROUNDSERVICE_H