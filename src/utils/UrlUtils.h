#ifndef URLUTILS_H
#define URLUTILS_H

#include <QUrl>
#include <QString>

class UrlUtils
{
public:
    // Преобразует пользовательский ввод в QUrl.
    // Если строка похожа на URL – возвращает его, иначе формирует поисковый URL.
    static QUrl fromUserInput(const QString &input, const QString &searchEngineTemplate);
};

#endif // URLUTILS_H