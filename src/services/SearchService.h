#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include <QString>
#include <QUrl>

class SearchService
{
public:
    // Принимает поисковый запрос и шаблон поисковой системы (например, "https://google.com/search?q=")
    // Возвращает готовый QUrl для поиска
    static QUrl buildSearchUrl(const QString &query, const QString &searchEngineTemplate);
};

#endif // SEARCHSERVICE_H