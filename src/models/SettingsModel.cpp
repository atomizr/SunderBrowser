#include "SettingsModel.h"

SettingsModel::SettingsModel()
{
    load();
}

void SettingsModel::load()
{
    m_homePage = m_settings.value("homePage", "https://www.google.com").toString();
    m_searchEngine = m_settings.value("searchEngine", "https://www.google.com/search?q=").toString();
    m_javaScriptEnabled = m_settings.value("javaScriptEnabled", true).toBool();
    m_zoom = m_settings.value("zoom", 1.0).toDouble();
    m_opacity = m_settings.value("opacity", 1.0).toDouble();
    m_theme = m_settings.value("theme", 0).toInt();
}

void SettingsModel::save()
{
    m_settings.setValue("homePage", m_homePage);
    m_settings.setValue("searchEngine", m_searchEngine);
    m_settings.setValue("javaScriptEnabled", m_javaScriptEnabled);
    m_settings.setValue("zoom", m_zoom);
    m_settings.setValue("opacity", m_opacity);
    m_settings.setValue("theme", m_theme);
}

// Геттеры
QString SettingsModel::homePage() const { return m_homePage; }
QString SettingsModel::searchEngine() const { return m_searchEngine; }
bool SettingsModel::javaScriptEnabled() const { return m_javaScriptEnabled; }
double SettingsModel::zoom() const { return m_zoom; }
double SettingsModel::windowOpacity() const { return m_opacity; }
int SettingsModel::theme() const { return m_theme; }

// Сеттеры (обновляют кэш и сохраняют)
void SettingsModel::setHomePage(const QString &page) { m_homePage = page; save(); }
void SettingsModel::setSearchEngine(const QString &engine) { m_searchEngine = engine; save(); }
void SettingsModel::setJavaScriptEnabled(bool enabled) { m_javaScriptEnabled = enabled; save(); }
void SettingsModel::setZoom(double zoom) { m_zoom = zoom; save(); }
void SettingsModel::setWindowOpacity(double opacity) { m_opacity = opacity; save(); }
void SettingsModel::setTheme(int theme) { m_theme = theme; save(); }