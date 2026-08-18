#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QString>
#include <QSettings>

class SettingsModel
{
public:
    SettingsModel();

    // Геттеры
    QString homePage() const;
    QString searchEngine() const;
    bool javaScriptEnabled() const;
    double zoom() const;
    double windowOpacity() const;
    int theme() const;

    // Сеттеры (сохраняют в QSettings)
    void setHomePage(const QString &page);
    void setSearchEngine(const QString &engine);
    void setJavaScriptEnabled(bool enabled);
    void setZoom(double zoom);
    void setWindowOpacity(double opacity);
    void setTheme(int theme);

    // Загрузка/сохранение всех настроек (можно вызывать при старте/закрытии)
    void load();
    void save();

private:
    QSettings m_settings;
    // Кэшированные значения для быстрого доступа (опционально)
    QString m_homePage;
    QString m_searchEngine;
    bool m_javaScriptEnabled;
    double m_zoom;
    double m_opacity;
    int m_theme;
};

#endif // SETTINGSMODEL_H