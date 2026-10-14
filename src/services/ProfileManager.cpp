#include "ProfileManager.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QWebEngineSettings>

namespace {

QWebEngineProfile *g_profile = nullptr;

// Полный User-Agent, имитирующий Chrome 131.
QString chromeUserAgent()
{
    return QStringLiteral(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/131.0.0.0 Safari/537.36");
}

} // namespace

QWebEngineProfile* ProfileManager::instance()
{
    if (!g_profile)
        init();
    return g_profile;
}

void ProfileManager::init()
{
    if (g_profile)
        return;

    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base + "/profile");
    QDir().mkpath(base + "/cache");

    g_profile = new QWebEngineProfile(QStringLiteral("sunder"), qApp);

    // Настраиваем постоянное хранилище
    g_profile->setPersistentStoragePath(base + "/profile");
    g_profile->setCachePath(base + "/cache");
    g_profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    g_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    // *** Устанавливаем User-Agent, имитирующий Chrome ***
    // Это может помочь, но не гарантирует обход проверок Google.
    g_profile->setHttpUserAgent(chromeUserAgent());

    // Разрешаем всё нужное для «взрослого» веба
    auto *settings = g_profile->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
}