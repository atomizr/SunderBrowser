#include "BrowserWindow.h"
#include "SettingsDialog.h"
#include <QToolBar>
#include <QMessageBox>
#include <QUrl>
#include <QSettings>
#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineDownloadRequest>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMimeData>
#include <QMenu>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>

// -------------------- BrowserTab --------------------
BrowserTab::BrowserTab(QWidget *parent) : QWidget(parent)
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
    connect(m_webView, &QWebEngineView::loadFinished, [this](bool ok) {
        m_progressBar->setVisible(false);
        emit loadFinished(ok);
    });
    connect(m_webView, &QWebEngineView::urlChanged, this, &BrowserTab::urlChanged);
    connect(m_webView, &QWebEngineView::titleChanged, this, &BrowserTab::titleChanged);

    // Настройка загрузки файлов (сигнал downloadRequested теперь в QWebEngineProfile)
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
    QMenu *menu = new QMenu(this);
    QAction *viewSource = menu->addAction("Просмотреть исходный код");
    connect(viewSource, &QAction::triggered, [this]() {
        m_webView->page()->action(QWebEnginePage::ViewSource)->trigger();
    });
    menu->popup(m_webView->mapToGlobal(pos));
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

// -------------------- BrowserWindow --------------------
BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAcceptDrops(true);
    setupUI();
    loadSettings();
    addNewTab(QUrl(m_homePage));
}

BrowserWindow::~BrowserWindow()
{
}

void BrowserWindow::setupUI()
{
    createToolbar();

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::onCurrentTabChanged);
    setCentralWidget(m_tabWidget);
}

void BrowserWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    m_backButton = new QPushButton("←");
    m_forwardButton = new QPushButton("→");
    m_reloadButton = new QPushButton("↻");
    m_stopButton = new QPushButton("✖");
    m_homeButton = new QPushButton("🏠");
    m_addTabButton = new QPushButton("+");
    m_settingsButton = new QPushButton("⚙");

    m_urlBar = new QLineEdit();
    m_urlBar->setPlaceholderText("Введите URL или поисковый запрос...");
    m_urlBar->setMinimumWidth(400);

    toolbar->addWidget(m_backButton);
    toolbar->addWidget(m_forwardButton);
    toolbar->addWidget(m_reloadButton);
    toolbar->addWidget(m_stopButton);
    toolbar->addWidget(m_homeButton);
    toolbar->addSeparator();
    toolbar->addWidget(m_urlBar);
    toolbar->addWidget(m_addTabButton);
    toolbar->addWidget(m_settingsButton);

    connect(m_backButton, &QPushButton::clicked, this, &BrowserWindow::goBack);
    connect(m_forwardButton, &QPushButton::clicked, this, &BrowserWindow::goForward);
    connect(m_reloadButton, &QPushButton::clicked, this, &BrowserWindow::reload);
    connect(m_stopButton, &QPushButton::clicked, this, &BrowserWindow::stop);
    connect(m_homeButton, &QPushButton::clicked, this, &BrowserWindow::home);
    connect(m_addTabButton, &QPushButton::clicked, this, [this]() { addNewTab(QUrl(m_homePage)); });
    connect(m_settingsButton, &QPushButton::clicked, this, &BrowserWindow::openSettings);
    connect(m_urlBar, &QLineEdit::returnPressed, this, &BrowserWindow::navigateToUrl);
}

BrowserTab* BrowserWindow::currentTab() const
{
    return qobject_cast<BrowserTab*>(m_tabWidget->currentWidget());
}

void BrowserWindow::addNewTab(const QUrl &url)
{
    BrowserTab *tab = new BrowserTab(this);
    int index = m_tabWidget->addTab(tab, "Новая вкладка");
    m_tabWidget->setCurrentIndex(index);

    connect(tab, &BrowserTab::titleChanged, [this, tab, index](const QString &title) {
        m_tabWidget->setTabText(index, title);
        if (m_tabWidget->currentWidget() == tab)
            setWindowTitle(title + " - Simple Browser");
    });
    connect(tab, &BrowserTab::urlChanged, [this, tab](const QUrl &url) {
        if (m_tabWidget->currentWidget() == tab)
            m_urlBar->setText(url.toString());
    });
    connect(tab, &BrowserTab::loadStarted, [this, tab]() {
        if (m_tabWidget->currentWidget() == tab) {
            m_reloadButton->setEnabled(false);
            m_stopButton->setEnabled(true);
        }
    });
    connect(tab, &BrowserTab::loadFinished, [this, tab](bool ok) {
        if (m_tabWidget->currentWidget() == tab) {
            m_reloadButton->setEnabled(true);
            m_stopButton->setEnabled(false);
        }
    });

    if (!url.isEmpty())
        tab->navigateToUrl(url);
}

void BrowserWindow::closeTab(int index)
{
    QWidget *tab = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete tab;

    if (m_tabWidget->count() == 0)
        addNewTab(QUrl(m_homePage));
}

void BrowserWindow::onCurrentTabChanged(int index)
{
    BrowserTab *tab = currentTab();
    if (!tab) return;

    m_urlBar->setText(tab->currentUrl().toString());
    setWindowTitle(tab->webView()->title() + " - Simple Browser");

    m_backButton->setEnabled(tab->webView()->history()->canGoBack());
    m_forwardButton->setEnabled(tab->webView()->history()->canGoForward());
}

void BrowserWindow::navigateToUrl()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;

    QString text = m_urlBar->text().trimmed();
    if (text.isEmpty()) return;

    QUrl url;
    if (text.contains(".") && !text.contains(" ")) {
        if (!text.startsWith("http://") && !text.startsWith("https://"))
            text = "http://" + text;
        url = QUrl(text);
    } else {
        url = QUrl(m_searchEngine + QUrl::toPercentEncoding(text));
    }
    tab->navigateToUrl(url);
}

void BrowserWindow::goBack()
{
    if (BrowserTab *tab = currentTab())
        tab->webView()->back();
}
void BrowserWindow::goForward()
{
    if (BrowserTab *tab = currentTab())
        tab->webView()->forward();
}
void BrowserWindow::reload()
{
    if (BrowserTab *tab = currentTab())
        tab->webView()->reload();
}
void BrowserWindow::stop()
{
    if (BrowserTab *tab = currentTab())
        tab->webView()->stop();
}
void BrowserWindow::home()
{
    if (BrowserTab *tab = currentTab())
        tab->navigateToUrl(QUrl(m_homePage));
}

void BrowserWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BrowserWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            if (filePath.endsWith(".html", Qt::CaseInsensitive) ||
                filePath.endsWith(".htm", Qt::CaseInsensitive)) {
                if (BrowserTab *tab = currentTab()) {
                    tab->navigateToUrl(url);
                }
            }
        }
    }
    event->acceptProposedAction();
}

void BrowserWindow::loadSettings()
{
    QSettings settings("MyCompany", "SimpleBrowser");
    m_homePage = settings.value("homePage", "https://www.google.com").toString();
    m_searchEngine = settings.value("searchEngine", "https://www.google.com/search?q=").toString();
    m_javaScriptEnabled = settings.value("javaScriptEnabled", true).toBool();

    double zoom = settings.value("zoom", 1.0).toDouble();
    double opacity = settings.value("opacity", 1.0).toDouble();

    window()->setWindowOpacity(opacity);
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        BrowserTab *tab = qobject_cast<BrowserTab*>(m_tabWidget->widget(i));
        if (tab)
            tab->webView()->setZoomFactor(zoom);
    }
}

void BrowserWindow::saveSettings()
{
    QSettings settings("MyCompany", "SimpleBrowser");
    settings.setValue("homePage", m_homePage);
    settings.setValue("searchEngine", m_searchEngine);
    settings.setValue("javaScriptEnabled", m_javaScriptEnabled);
    if (BrowserTab *tab = currentTab()) {
        settings.setValue("zoom", tab->webView()->zoomFactor());
    }
    settings.setValue("opacity", windowOpacity());
}

void BrowserWindow::openSettings()
{
    SettingsDialog dialog(this);
    dialog.setHomePage(m_homePage);
    dialog.setSearchEngine(m_searchEngine);
    dialog.setJavaScriptEnabled(m_javaScriptEnabled);
    if (BrowserTab *tab = currentTab())
        dialog.setZoom(tab->webView()->zoomFactor());
    dialog.setWindowOpacity(windowOpacity());

    QSettings appSettings("MyCompany", "SimpleBrowser");
    dialog.setTheme(appSettings.value("theme", 0).toInt());

    if (dialog.exec() == QDialog::Accepted) {
        m_homePage = dialog.getHomePage();
        m_searchEngine = dialog.getSearchEngine();
        m_javaScriptEnabled = dialog.isJavaScriptEnabled();

        double zoom = dialog.getZoom();
        double opacity = dialog.getWindowOpacity();
        int theme = dialog.getTheme();

        for (int i = 0; i < m_tabWidget->count(); ++i) {
            BrowserTab *tab = qobject_cast<BrowserTab*>(m_tabWidget->widget(i));
            if (tab) {
                tab->webView()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, m_javaScriptEnabled);
                tab->webView()->setZoomFactor(zoom);
            }
        }
        window()->setWindowOpacity(opacity);
        appSettings.setValue("theme", theme);

        // Применить тему (если реализовано)
        // applyTheme(theme);

        saveSettings();
    }
}