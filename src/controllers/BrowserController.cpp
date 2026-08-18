#include "BrowserController.h"
#include "ui/MainWindow.h"
#include "ui/BrowserTab.h"
#include "services/SearchService.h"
#include "utils/UrlUtils.h"
#include <QWebEngineSettings>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QStatusBar>

BrowserController::BrowserController(SettingsModel &settings, QObject *parent)
    : QObject(parent), m_settings(settings)
{
}

// ----- Геттеры -----
QString BrowserController::homePage() const { return m_settings.homePage(); }
QString BrowserController::searchEngine() const { return m_settings.searchEngine(); }
bool BrowserController::javaScriptEnabled() const { return m_settings.javaScriptEnabled(); }
double BrowserController::zoom() const { return m_settings.zoom(); }
double BrowserController::windowOpacity() const { return m_settings.windowOpacity(); }
int BrowserController::theme() const { return m_settings.theme(); }

// ----- Сеттеры (с применением) -----
void BrowserController::setHomePage(const QString &page)
{
    m_settings.setHomePage(page);
}

void BrowserController::setSearchEngine(const QString &engine)
{
    m_settings.setSearchEngine(engine);
}

void BrowserController::setJavaScriptEnabled(bool enabled)
{
    m_settings.setJavaScriptEnabled(enabled);
    for (auto *tab : m_tabs)
        applySettingsToTab(tab);
}

void BrowserController::setZoom(double zoom)
{
    m_settings.setZoom(zoom);
    for (auto *tab : m_tabs)
        applySettingsToTab(tab);
}

void BrowserController::setWindowOpacity(double opacity)
{
    m_settings.setWindowOpacity(opacity);
    if (m_mainWindow)
        m_mainWindow->setWindowOpacity(opacity);
}

void BrowserController::setTheme(int theme)
{
    m_settings.setTheme(theme);
    applyTheme(theme);
}

// ----- Остальные методы -----
void BrowserController::setMainWindow(MainWindow *window)
{
    m_mainWindow = window;
}

void BrowserController::showStatusMessage(const QString &message, int timeout)
{
    if (m_mainWindow) {
        m_mainWindow->statusBar()->showMessage(message, timeout);
    }
}

BrowserTab* BrowserController::currentTab() const
{
    if (m_mainWindow)
        return m_mainWindow->currentTab();
    return nullptr;
}

int BrowserController::currentIndex() const
{
    if (m_mainWindow)
        return m_mainWindow->currentTabIndex();
    return -1;
}

int BrowserController::tabCount() const
{
    if (m_mainWindow)
        return m_mainWindow->tabCount();
    return 0;
}

void BrowserController::addTab(const QUrl &url)
{
    if (!m_mainWindow) return;
    BrowserTab *tab = m_mainWindow->createTab();
    m_tabs.append(tab);
    setupTabSignals(tab);
    applySettingsToTab(tab);

    if (url.isEmpty()) {
        m_mainWindow->loadStartPage(tab);
    } else {
        tab->navigateToUrl(url);
    }
    m_mainWindow->setCurrentTab(tab);
    emit tabCountChanged(m_tabs.size());
}

void BrowserController::closeTab(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    BrowserTab *tab = m_tabs.takeAt(index);
    // FIXED: Останавливаем загрузку перед удалением
    if (tab->webView())
        tab->webView()->stop();
    m_mainWindow->removeTab(index);
    delete tab;
    emit tabCountChanged(m_tabs.size());
    if (m_tabs.isEmpty()) {
        addTab();
    }
}

void BrowserController::closeCurrentTab()
{
    int idx = currentIndex();
    if (idx >= 0) closeTab(idx);
}

void BrowserController::closeOtherTabs()
{
    int current = currentIndex();
    if (current < 0) return;
    for (int i = m_tabs.size() - 1; i >= 0; --i) {
        if (i != current)
            closeTab(i);
    }
}

void BrowserController::duplicateCurrentTab()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;
    QUrl url = tab->currentUrl();
    if (url.isValid() && !url.isEmpty())
        addTab(url);
}

void BrowserController::switchTab(int index)
{
    if (m_mainWindow)
        m_mainWindow->setCurrentIndex(index);
}

void BrowserController::navigateToUrl(const QString &input)
{
    BrowserTab *tab = currentTab();
    if (!tab) return;

    QUrl url = UrlUtils::fromUserInput(input, searchEngine());
    if (url.isValid()) {
        tab->navigateToUrl(url);
    } else {
        showStatusMessage("Некорректный URL", 2000);
    }
}

void BrowserController::goBack()
{
    BrowserTab *tab = currentTab();
    if (tab && tab->webView()->history()->canGoBack())
        tab->webView()->back();
}

void BrowserController::goForward()
{
    BrowserTab *tab = currentTab();
    if (tab && tab->webView()->history()->canGoForward())
        tab->webView()->forward();
}

void BrowserController::reload()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;
    QWebEnginePage *page = tab->webView()->page();
    if (page && page->isLoading())
        page->triggerAction(QWebEnginePage::Stop);
    tab->webView()->reload();
}

void BrowserController::stop()
{
    BrowserTab *tab = currentTab();
    if (tab) tab->webView()->stop();
}

void BrowserController::home()
{
    BrowserTab *tab = currentTab();
    if (tab) tab->navigateToUrl(QUrl(homePage()));
}

void BrowserController::applySettingsToTab(BrowserTab *tab)
{
    if (!tab) return;
    tab->webView()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, javaScriptEnabled());
    tab->webView()->setZoomFactor(zoom());
}

void BrowserController::applyTheme(int themeIndex)
{
    if (!m_mainWindow) return;
    m_mainWindow->applyTheme(themeIndex);
    emit themeApplied();
}

void BrowserController::setupTabSignals(BrowserTab *tab)
{
    connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString &title) {
        onTabTitleChanged(tab, title);
    });
    connect(tab, &BrowserTab::urlChanged, this, [this, tab](const QUrl &url) {
        onTabUrlChanged(tab, url);
    });
    connect(tab, &BrowserTab::loadStarted, this, [this, tab]() {
        onTabLoadStarted(tab);
    });
    connect(tab, &BrowserTab::loadProgress, this, [this, tab](int progress) {
        onTabLoadProgress(tab, progress);
    });
    connect(tab, &BrowserTab::loadFinished, this, [this, tab](bool ok) {
        onTabLoadFinished(tab, ok);
    });
    // FIXED: подключаем сигнал статусных сообщений от вкладки
    connect(tab, &BrowserTab::statusMessage, this, &BrowserController::showStatusMessage);
}

void BrowserController::onTabTitleChanged(BrowserTab *tab, const QString &title)
{
    if (m_mainWindow) {
        int idx = m_tabs.indexOf(tab);
        if (idx >= 0) {
            m_mainWindow->setTabTitle(idx, title);
        }
        if (tab == currentTab())
            m_mainWindow->setWindowTitle(title + " - Sunder Browser");
    }
    emit titleChanged(title);
}

void BrowserController::onTabUrlChanged(BrowserTab *tab, const QUrl &url)
{
    if (m_mainWindow && tab == currentTab()) {
        m_mainWindow->setUrlBarText(url.toString());
    }
    emit urlChanged(url);
}

void BrowserController::onTabLoadStarted(BrowserTab *tab)
{
    showStatusMessage("Загрузка...", 0);
    emit loadStarted();
}

void BrowserController::onTabLoadFinished(BrowserTab *tab, bool ok)
{
    if (!ok) {
        showStatusMessage("Ошибка загрузки страницы", 3000);
    } else {
        showStatusMessage("Готово", 2000);
    }
    emit loadFinished(ok);
}

void BrowserController::onTabLoadProgress(BrowserTab *tab, int progress)
{
    emit loadProgress(progress);
}

void BrowserController::updateCurrentTabUI()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;
    onTabUrlChanged(tab, tab->currentUrl());
    onTabTitleChanged(tab, tab->webView()->title());
}

// FIXED: реализация clearTabs
void BrowserController::clearTabs()
{
    // Просто очищаем список, не удаляя объекты – они уже удалены вместе с MainWindow
    m_tabs.clear();
}
