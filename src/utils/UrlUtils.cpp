#include "UrlUtils.h"
#include <QUrl>
#include <QRegularExpression>

QUrl UrlUtils::fromUserInput(const QString &input, const QString &searchEngineTemplate)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty())
        return QUrl();

    // Сначала пробуем стандартный QUrl::fromUserInput (он распознает "localhost", "myserver" и т.п.)
    QUrl url = QUrl::fromUserInput(trimmed);
    // Если получился валидный URL и у него есть схема (http, https, ftp, file и т.д.)
    // или это относительный путь, но не пустая схема – используем его.
    if (url.isValid() && !url.scheme().isEmpty() && url.scheme() != "about") {
        return url;
    }

    // FIXED: улучшенная проверка на поисковый запрос
    // Если строка содержит пробелы или не содержит точки (и не похожа на IP-адрес или локальное имя)
    // – считаем поисковым запросом.
    // Но также проверяем, что это не что-то вроде "192.168.1.1" (содержит точки и цифры)
    if (trimmed.contains(' ') || !trimmed.contains('.')) {
        QString encoded = QUrl::toPercentEncoding(trimmed);
        return QUrl(searchEngineTemplate + encoded);
    }

    // Если всё ещё не распознали – пробуем ещё раз с QUrl::fromUserInput (может сработать как относительный путь)
    return QUrl::fromUserInput(trimmed);
}
