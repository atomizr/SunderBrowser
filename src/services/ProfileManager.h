#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QWebEngineProfile>

// Глобальный постоянный профиль браузера.
// Хранит cookies, localStorage, IndexedDB, кэш и настройки между запусками.
class ProfileManager
{
public:
    static QWebEngineProfile* instance();
    static void init();

private:
    ProfileManager() = default;
};

#endif // PROFILEMANAGER_H