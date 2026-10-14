#ifndef ICONSERVICE_H
#define ICONSERVICE_H

#include <QIcon>
#include <QString>

class IconService
{
public:
    // Загружает иконку по имени (из ресурсов или из файловой системы)
    static QIcon loadIcon(const QString &name);
};

#endif // ICONSERVICE_H