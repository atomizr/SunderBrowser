#include "UrlUtils.h"
#include <QUrl>
#include <QRegularExpression>
#include <QStringList>

namespace {

bool looksLikeScheme(const QString &s)
{
    static const QRegularExpression re(R"(^[a-zA-Z][a-zA-Z0-9+.\-]*$)");
    return re.match(s).hasMatch();
}

const QStringList &knownSchemes()
{
    static const QStringList schemes = {
        "http", "https", "ftp", "file", "about", "view-source", "data", "chrome"
    };
    return schemes;
}

const QStringList &bareLocalHosts()
{
    static const QStringList hosts = { "localhost" };
    return hosts;
}

} // namespace

QUrl UrlUtils::fromUserInput(const QString &input, const QString &searchEngineTemplate)
{
    const QString trimmed = input.trimmed();
    if (trimmed.isEmpty())
        return QUrl();

    // 1. Пробел → точно поисковый запрос
    if (trimmed.contains(' '))
        return QUrl(searchEngineTemplate + QString::fromLatin1(QUrl::toPercentEncoding(trimmed)));

    // 2. Явная схема (http://, https://, about:, view-source: и т.п.)
    const int colon = trimmed.indexOf(':');
    if (colon > 0) {
        const QString scheme = trimmed.left(colon).toLower();
        if (looksLikeScheme(scheme)) {
            if (knownSchemes().contains(scheme)) {
                QUrl url(trimmed);
                if (url.isValid())
                    return url;
            }
            // "localhost:8080", "myserver:3000" — вероятно host:port
            if (!scheme.contains('.')) {
                QUrl url("http://" + trimmed);
                if (url.isValid() && !url.host().isEmpty())
                    return url;
            }
        }
    }

    // 3. "localhost" без порта
    if (bareLocalHosts().contains(trimmed.toLower()))
        return QUrl("http://" + trimmed);

    // 4. Домен/IP/путь с точкой или слешем — пусть Qt попробует
    if (trimmed.contains('.') || trimmed.contains('/')) {
        QUrl url = QUrl::fromUserInput(trimmed);
        if (url.isValid() && !url.host().isEmpty() && url.scheme() != "file")
            return url;
        if (url.isValid() && (url.scheme() == "http" || url.scheme() == "https"))
            return url;
    }

    // 5. Иначе — поиск
    return QUrl(searchEngineTemplate + QString::fromLatin1(QUrl::toPercentEncoding(trimmed)));
}
