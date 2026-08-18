#ifndef BROWSERTAB_H
#define BROWSERTAB_H

#include <QWidget>
#include <QWebEngineView>
#include <QProgressBar>

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
    // Сигнал для отображения сообщений в статус-баре (делегируется контроллеру)
    void statusMessage(const QString &message, int timeout = 2000);

private slots:
    void onDownloadRequested(QWebEngineDownloadRequest *download);
    void onLoadFinished(bool ok);

private:
    QWebEngineView *m_webView;
    QProgressBar *m_progressBar;
    void setupContextMenu();
    void showContextMenu(const QPoint &pos);
    void savePageSource();
};

#endif // BROWSERTAB_H