#include "UrlUtils.h"
#include <QUrl>
#include <QRegularExpression>

QUrl UrlUtils::fromUserInput(const QString &input, const QString &searchEngineTemplate)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty())
        return QUrl();

    // Проверяем, является ли строка корректным URL
    QUrl url = QUrl::fromUserInput(trimmed);
    if (url.isValid() && !url.scheme().isEmpty() && url.scheme() != "about") {
        // Если схема присутствует и это не about:blank и т.п.
        return url;
    }

    // Если строка содержит пробелы или не содержит точки – считаем поисковым запросом
    if (trimmed.contains(' ') || !trimmed.contains('.')) {
        // Ищем в поисковой системе
        QString encoded = QUrl::toPercentEncoding(trimmed);
        return QUrl(searchEngineTemplate + encoded);
    }

    // Иначе пробуем как URL
    return QUrl::fromUserInput(trimmed);
}