#include "SearchService.h"
#include <QUrl>

QUrl SearchService::buildSearchUrl(const QString &query, const QString &searchEngineTemplate)
{
    if (query.isEmpty())
        return QUrl();
    QString encoded = QUrl::toPercentEncoding(query);
    return QUrl(searchEngineTemplate + encoded);
}