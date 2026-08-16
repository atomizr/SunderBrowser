#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QWebEngineView>
#include <QToolButton>
#include <QProgressBar>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

class BrowserTab : public QWidget
{
    Q_OBJECT
public:
    explicit BrowserTab(QWidget *parent = nullptr);
    QWebEngineView *webView() const { return m_webView; }
    QProgressBar *progressBar() const { return m_progressBar; }
    void navigateToUrl(const QUrl &url);
    QUrl currentUrl() const { return m_webView->url(); }

signals:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);

private slots:
    void onDownloadRequested(QWebEngineDownloadRequest *download);
    void onLoadFinished(bool ok);

private:
    QWebEngineView *m_webView;
    QProgressBar *m_progressBar;
    void setupContextMenu();
    void showContextMenu(const QPoint &pos);
};

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
public:
    BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void addNewTab(const QUrl &url = QUrl());
    void closeTab(int index);
    void onCurrentTabChanged(int index);
    void navigateToUrl();
    void goBack();
    void goForward();
    void reload();
    void stop();
    void home();
    void openSettings();
    void applyTheme(int themeIndex);

private:
    QTabWidget *m_tabWidget;
    QLineEdit *m_urlBar;
    QToolButton *m_backButton;
    QToolButton *m_forwardButton;
    QToolButton *m_reloadButton;
    QToolButton *m_stopButton;
    QToolButton *m_homeButton;
    QToolButton *m_addTabButton;
    QToolButton *m_settingsButton;

    QString m_homePage;
    QString m_searchEngine;
    bool m_javaScriptEnabled;
    double m_zoom;

    void setupUI();
    void createToolbar();
    void loadSettings();
    void saveSettings();
    BrowserTab* currentTab() const;
    void applySettingsToTab(BrowserTab *tab);
};

#endif // BROWSERWINDOW_H
