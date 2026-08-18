#ifndef BROWSERCONTROLLER_H
#define BROWSERCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QList>
#include "models/SettingsModel.h"

// Forward declarations
class BrowserTab;
class MainWindow;

class BrowserController : public QObject
{
    Q_OBJECT
public:
    explicit BrowserController(SettingsModel &settings, QObject *parent = nullptr);

    // Геттеры настроек (делегируют модели)
    QString homePage() const;
    QString searchEngine() const;
    bool javaScriptEnabled() const;
    double zoom() const;
    double windowOpacity() const;
    int theme() const;
    // Сеттеры настроек (изменяют модель и применяют)
    void setHomePage(const QString &page);
    void setSearchEngine(const QString &engine);
    void setJavaScriptEnabled(bool enabled);
    void setZoom(double zoom);
    void setWindowOpacity(double opacity);
    void setTheme(int theme);
    // Действия
    void addTab(const QUrl &url = QUrl());
    void closeTab(int index);
    void closeCurrentTab();
    void closeOtherTabs();
    void duplicateCurrentTab();
    void switchTab(int index);
    void navigateToUrl(const QString &input);
    void goBack();
    void goForward();
    void reload();
    void stop();
    void home();
    void applySettingsToTab(BrowserTab *tab);
    void applyTheme(int themeIndex);
    // Управление вкладками
    BrowserTab* currentTab() const;
    int currentIndex() const;
    int tabCount() const;
    // Регистрация главного окна для обратной связи (обновление UI)
    void setMainWindow(MainWindow *window);
    // Обновление статусной строки
    void showStatusMessage(const QString &message, int timeout = 2000);
    // Обновление UI текущей вкладки (адрес, заголовок) – теперь публичный
    void updateCurrentTabUI();
    void clearTabs();

signals:
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);
    void loadProgress(int progress);
    void loadStarted();
    void loadFinished(bool ok);
    void tabCountChanged(int count);
    void themeApplied();

public slots:
    void onTabTitleChanged(BrowserTab *tab, const QString &title);
    void onTabUrlChanged(BrowserTab *tab, const QUrl &url);
    void onTabLoadStarted(BrowserTab *tab);
    void onTabLoadFinished(BrowserTab *tab, bool ok);
    void onTabLoadProgress(BrowserTab *tab, int progress);

private:
    SettingsModel &m_settings;
    MainWindow *m_mainWindow = nullptr;
    QList<BrowserTab*> m_tabs; // все открытые вкладки

    // Вспомогательные методы
    void setupTabSignals(BrowserTab *tab);
};

#endif // BROWSERCONTROLLER_H
