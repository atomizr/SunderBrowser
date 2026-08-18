#include "BrowserTab.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>
#include <QStatusBar>
#include <QMainWindow>
#include <QWebEngineProfile>
#include <QWebEnginePage>

BrowserTab::BrowserTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_webView = new QWebEngineView(this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setVisible(false);

    layout->addWidget(m_webView);
    layout->addWidget(m_progressBar);

    connect(m_webView, &QWebEngineView::loadStarted, [this]() {
        m_progressBar->setVisible(true);
        m_progressBar->setMaximum(100);
        m_progressBar->setValue(0);
        emit loadStarted();
    });
    connect(m_webView, &QWebEngineView::loadProgress, [this](int progress) {
        m_progressBar->setValue(progress);
        emit loadProgress(progress);
    });
    connect(m_webView, &QWebEngineView::loadFinished, this, &BrowserTab::onLoadFinished);
    connect(m_webView, &QWebEngineView::urlChanged, this, &BrowserTab::urlChanged);
    connect(m_webView, &QWebEngineView::titleChanged, this, &BrowserTab::titleChanged);

    auto profile = m_webView->page()->profile();
    profile->setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    connect(profile, &QWebEngineProfile::downloadRequested, this, &BrowserTab::onDownloadRequested);

    setupContextMenu();
}

void BrowserTab::navigateToUrl(const QUrl &url)
{
    m_webView->load(url);
}

void BrowserTab::setupContextMenu()
{
    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_webView, &QWebEngineView::customContextMenuRequested, this, &BrowserTab::showContextMenu);
}

void BrowserTab::showContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *viewSource = menu->addAction("Просмотреть исходный код");
    connect(viewSource, &QAction::triggered, [this]() {
        QUrl currentUrl = m_webView->url();
        if (currentUrl.isValid() && !currentUrl.isEmpty()) {
            QUrl viewSourceUrl("view-source:" + currentUrl.toString());
            m_webView->load(viewSourceUrl);
        } else {
            emit statusMessage("Нет страницы для просмотра исходного кода", 2000);
        }
    });

    QAction *saveSource = menu->addAction("Сохранить исходный код...");
    connect(saveSource, &QAction::triggered, [this]() {
        savePageSource();
    });

    menu->popup(m_webView->mapToGlobal(pos));
}

void BrowserTab::savePageSource()
{
    QWebEnginePage *page = m_webView->page();
    if (!page) return;

    page->toHtml([this](const QString &html) {
        if (html.isEmpty()) {
            emit statusMessage("Не удалось получить исходный код страницы", 2000);
            return;
        }

        QString defaultName = "page.html";
        QString title = m_webView->title();
        if (!title.isEmpty()) {
            defaultName = title + ".html";
            defaultName.replace(QRegularExpression("[<>:\"/\\|?*]"), "_");
        }

        QString savePath = QFileDialog::getSaveFileName(
            this,
            "Сохранить исходный код",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + defaultName,
            "HTML файлы (*.html *.htm);;Все файлы (*)"
        );

        if (savePath.isEmpty())
            return;

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            emit statusMessage("Не удалось сохранить файл", 2000);
            return;
        }

        QTextStream out(&file);
        out << html;
        file.close();

        emit statusMessage("Исходный код сохранён: " + QFileInfo(savePath).fileName(), 3000);
    });
}

void BrowserTab::onDownloadRequested(QWebEngineDownloadRequest *download)
{
    QString suggestedFileName = QFileInfo(download->downloadFileName()).fileName();
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить файл",
                                                     QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + suggestedFileName,
                                                     QString());
    if (!savePath.isEmpty()) {
        download->setDownloadDirectory(QFileInfo(savePath).absolutePath());
        download->setDownloadFileName(QFileInfo(savePath).fileName());
        download->accept();
    } else {
        download->cancel();
    }
}

void BrowserTab::onLoadFinished(bool ok)
{
    m_progressBar->setVisible(false);
    if (!ok) {
        emit statusMessage("Ошибка загрузки страницы", 3000);
    }
    emit loadFinished(ok);
}