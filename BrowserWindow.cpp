#include "BrowserWindow.h"
#include "SettingsDialog.h"
#include <QToolBar>
#include <QMessageBox>
#include <QUrl>
#include <QSettings>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
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
#include <QStyle>
#include <QStatusBar>
#include <QFile>
#include <QTextStream>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QIcon>
#include <QDir>
#include <QCoreApplication>

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
            if (QMainWindow *mw = qobject_cast<QMainWindow*>(window())) {
                if (QStatusBar *sb = mw->statusBar()) {
                    sb->showMessage("Нет страницы для просмотра исходного кода", 2000);
                }
            }
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
            if (QMainWindow *mw = qobject_cast<QMainWindow*>(window())) {
                if (QStatusBar *sb = mw->statusBar()) {
                    sb->showMessage("Не удалось получить исходный код страницы", 2000);
                }
            }
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
            if (QMainWindow *mw = qobject_cast<QMainWindow*>(window())) {
                if (QStatusBar *sb = mw->statusBar()) {
                    sb->showMessage("Не удалось сохранить файл", 2000);
                }
            }
            return;
        }

        QTextStream out(&file);
        out << html;
        file.close();

        if (QMainWindow *mw = qobject_cast<QMainWindow*>(window())) {
            if (QStatusBar *sb = mw->statusBar()) {
                sb->showMessage("Исходный код сохранён: " + QFileInfo(savePath).fileName(), 3000);
            }
        }
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
        if (QMainWindow *mw = qobject_cast<QMainWindow*>(window())) {
            if (QStatusBar *sb = mw->statusBar()) {
                sb->showMessage("Ошибка загрузки страницы", 3000);
            }
        }
    }
    emit loadFinished(ok);
}

// -------------------- BrowserWindow --------------------
BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent), m_zoom(1.0), m_javaScriptEnabled(true)
{
    setAcceptDrops(true);
    setWindowOpacity(1.0);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect available = screen->availableGeometry();
        int width = available.width() * 0.8;
        int height = available.height() * 0.8;
        width = qMax(width, 800);
        height = qMax(height, 600);
        resize(width, height);
        move(available.x() + (available.width() - width) / 2,
             available.y() + (available.height() - height) / 2);
    } else {
        resize(1024, 768);
    }

    setupUI();
    loadSettings();
    addNewTab(QUrl(m_homePage));

    QSettings settings;
    int theme = settings.value("theme", 0).toInt();
    applyTheme(theme);
}

BrowserWindow::~BrowserWindow()
{
}

// ---- Загрузка иконок из ресурсов ----
QIcon BrowserWindow::loadIcon(const QString &name)
{
    QIcon icon;
    QString resPath = ":/icons/" + name;
    if (QFile::exists(resPath)) {
        icon.addFile(resPath);
    } else {
        QString filePath = QCoreApplication::applicationDirPath() + "/icons/" + name;
        if (QFile::exists(filePath)) {
            icon.addFile(filePath);
        } else {
            qWarning() << "Icon not found:" << name;
        }
    }
    return icon;
}

void BrowserWindow::setupUI()
{
    createToolbar();
    statusBar()->showMessage("Готово");
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    // Обновлённый стиль для кнопки закрытия вкладки – теперь точно будет видна
QString styleSheet = QString(
    "QTabBar::close-button {"
    "  image: url(:/icons/icons/stop.png);"   // добавить дополнительную папку icons/
    "  subcontrol-position: right;"
    "  subcontrol-origin: padding;"
    "  width: 16px;"
    "  height: 16px;"
    "}"
);
    m_tabWidget->tabBar()->setStyleSheet(styleSheet);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::onCurrentTabChanged);
    setCentralWidget(m_tabWidget);

    setupTabContextMenu();
    m_tabWidget->tabBar()->installEventFilter(this);
}

void BrowserWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    m_backButton = new QToolButton;
    m_backButton->setIcon(loadIcon("back.png"));
    m_backButton->setToolTip("Назад (Alt+Left)");

    m_forwardButton = new QToolButton;
    m_forwardButton->setIcon(loadIcon("forward.png"));
    m_forwardButton->setToolTip("Вперёд (Alt+Right)");

    m_reloadButton = new QToolButton;
    m_reloadButton->setIcon(loadIcon("reload.png"));
    m_reloadButton->setToolTip("Перезагрузить (Ctrl+R / F5)");

    m_addTabButton = new QToolButton;
    m_addTabButton->setIcon(loadIcon("add.png"));
    m_addTabButton->setToolTip("Новая вкладка (Ctrl+T)");

    m_homeButton = new QToolButton;
    m_homeButton->setIcon(loadIcon("home.png"));
    m_homeButton->setToolTip("Домашняя страница");

    m_settingsButton = new QToolButton;
    m_settingsButton->setIcon(loadIcon("settings.png"));
    m_settingsButton->setToolTip("Настройки");

    m_urlBar = new QLineEdit();
    m_urlBar->setPlaceholderText("Введите URL или поисковый запрос...");
    m_urlBar->setMinimumWidth(400);
    m_urlBar->setToolTip("Адресная строка (Ctrl+L / Alt+D)");

    toolbar->addWidget(m_backButton);
    toolbar->addWidget(m_forwardButton);
    toolbar->addWidget(m_reloadButton);
    toolbar->addWidget(m_addTabButton);
    toolbar->addWidget(m_homeButton);
    toolbar->addSeparator();
    toolbar->addWidget(m_urlBar);
    toolbar->addWidget(m_settingsButton);

    connect(m_backButton, &QToolButton::clicked, this, &BrowserWindow::goBack);
    connect(m_forwardButton, &QToolButton::clicked, this, &BrowserWindow::goForward);
    connect(m_reloadButton, &QToolButton::clicked, this, &BrowserWindow::reload);
    connect(m_addTabButton, &QToolButton::clicked, this, [this]() { addNewTab(QUrl(m_homePage)); });
    connect(m_homeButton, &QToolButton::clicked, this, &BrowserWindow::home);
    connect(m_settingsButton, &QToolButton::clicked, this, &BrowserWindow::openSettings);
    connect(m_urlBar, &QLineEdit::returnPressed, this, &BrowserWindow::navigateToUrl);
}

BrowserTab* BrowserWindow::currentTab() const
{
    return qobject_cast<BrowserTab*>(m_tabWidget->currentWidget());
}

void BrowserWindow::applySettingsToTab(BrowserTab *tab)
{
    if (!tab) return;
    tab->webView()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, m_javaScriptEnabled);
    tab->webView()->setZoomFactor(m_zoom);
}

void BrowserWindow::addNewTab(const QUrl &url)
{
    BrowserTab *tab = new BrowserTab(this);
    int index = m_tabWidget->addTab(tab, "Новая вкладка");
    m_tabWidget->setCurrentIndex(index);

    applySettingsToTab(tab);

    connect(tab, &BrowserTab::titleChanged, [this, tab, index](const QString &title) {
        m_tabWidget->setTabText(index, title);
        if (m_tabWidget->currentWidget() == tab)
            setWindowTitle(title + " - Sunder Browser");
    });
    connect(tab, &BrowserTab::urlChanged, [this, tab](const QUrl &url) {
        if (m_tabWidget->currentWidget() == tab)
            m_urlBar->setText(url.toString());
    });
    connect(tab, &BrowserTab::loadStarted, [this, tab]() {});
    connect(tab, &BrowserTab::loadFinished, [this, tab](bool ok) {});

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
    setWindowTitle(tab->webView()->title() + " - Sunder Browser");
}

void BrowserWindow::navigateToUrl()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;

    QString text = m_urlBar->text().trimmed();
    if (text.isEmpty()) return;

    QUrl url = QUrl::fromUserInput(text);
    if (!url.isValid()) {
        statusBar()->showMessage("Некорректный URL", 2000);
        return;
    }
    tab->navigateToUrl(url);
}

void BrowserWindow::goBack()
{
    BrowserTab *tab = currentTab();
    if (tab && tab->webView()->history()->canGoBack())
        tab->webView()->back();
}

void BrowserWindow::goForward()
{
    BrowserTab *tab = currentTab();
    if (tab && tab->webView()->history()->canGoForward())
        tab->webView()->forward();
}

void BrowserWindow::reload()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;
    QWebEnginePage *page = tab->webView()->page();
    if (page && page->isLoading())
        page->triggerAction(QWebEnginePage::Stop);
    tab->webView()->reload();
}

void BrowserWindow::stop()
{
    // Кнопка stop удалена, но метод оставлен на случай использования извне
    if (BrowserTab *tab = currentTab())
        tab->webView()->stop();
}

void BrowserWindow::home()
{
    if (BrowserTab *tab = currentTab())
        tab->navigateToUrl(QUrl(m_homePage));
}

// ---- Обработка клавиш ----
void BrowserWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
        statusBar()->showMessage(isFullScreen() ? "Полноэкранный режим включён (F11)" : "Полноэкранный режим выключен", 2000);
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_T) {
            addNewTab(QUrl(m_homePage));
            event->accept();
            return;
        } else if (event->key() == Qt::Key_W) {
            closeCurrentTab();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Tab) {
            int count = m_tabWidget->count();
            if (count > 1) {
                int next = (m_tabWidget->currentIndex() + 1) % count;
                m_tabWidget->setCurrentIndex(next);
            }
            event->accept();
            return;
        }
    }
    if ((event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (event->key() == Qt::Key_Tab) {
            int count = m_tabWidget->count();
            if (count > 1) {
                int prev = (m_tabWidget->currentIndex() - 1 + count) % count;
                m_tabWidget->setCurrentIndex(prev);
            }
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

// ---- Закрытие по средней кнопке ----
bool BrowserWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_tabWidget->tabBar() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::MiddleButton) {
            int index = m_tabWidget->tabBar()->tabAt(mouseEvent->pos());
            if (index != -1) {
                closeTab(index);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ---- Контекстное меню вкладок ----
void BrowserWindow::setupTabContextMenu()
{
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested,
            [this](const QPoint &pos) {
                int index = m_tabWidget->tabBar()->tabAt(pos);
                if (index == -1) return;
                m_tabWidget->setCurrentIndex(index);

                QMenu menu;
                QAction *closeAct = menu.addAction("Закрыть вкладку");
                QAction *closeOthersAct = menu.addAction("Закрыть другие вкладки");
                QAction *reloadAct = menu.addAction("Перезагрузить");
                QAction *duplicateAct = menu.addAction("Дублировать вкладку");

                QAction *selected = menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
                if (selected == closeAct) {
                    closeCurrentTab();
                } else if (selected == closeOthersAct) {
                    closeOtherTabs();
                } else if (selected == reloadAct) {
                    reloadCurrentTab();
                } else if (selected == duplicateAct) {
                    duplicateCurrentTab();
                }
            });
}

void BrowserWindow::closeCurrentTab()
{
    int index = m_tabWidget->currentIndex();
    if (index != -1)
        closeTab(index);
}

void BrowserWindow::closeOtherTabs()
{
    int current = m_tabWidget->currentIndex();
    if (current == -1) return;
    for (int i = m_tabWidget->count() - 1; i >= 0; --i) {
        if (i != current)
            closeTab(i);
    }
}

void BrowserWindow::reloadCurrentTab()
{
    reload();
}

void BrowserWindow::duplicateCurrentTab()
{
    BrowserTab *tab = currentTab();
    if (!tab) return;
    QUrl url = tab->currentUrl();
    if (url.isValid() && !url.isEmpty())
        addNewTab(url);
}

// ---- Drag & Drop ----
void BrowserWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BrowserWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isValid()) {
            if (BrowserTab *tab = currentTab()) {
                tab->navigateToUrl(url);
                break;
            }
        }
    }
    event->acceptProposedAction();
}

// ---- Настройки ----
void BrowserWindow::loadSettings()
{
    QSettings settings;
    m_homePage = settings.value("homePage", "https://www.google.com").toString();
    m_searchEngine = settings.value("searchEngine", "https://www.google.com/search?q=").toString();
    m_javaScriptEnabled = settings.value("javaScriptEnabled", true).toBool();
    m_zoom = settings.value("zoom", 1.0).toDouble();

    double opacity = settings.value("opacity", 1.0).toDouble();
    if (opacity <= 0.0 || opacity > 1.0)
        opacity = 1.0;
    window()->setWindowOpacity(opacity);
}

void BrowserWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("homePage", m_homePage);
    settings.setValue("searchEngine", m_searchEngine);
    settings.setValue("javaScriptEnabled", m_javaScriptEnabled);
    settings.setValue("zoom", m_zoom);
    settings.setValue("opacity", windowOpacity());
}

void BrowserWindow::applyTheme(int themeIndex)
{
    QPalette pal;
    switch (themeIndex) {
    case 1:
        pal.setColor(QPalette::Window, QColor(53, 53, 53));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base, QColor(25, 25, 25));
        pal.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        pal.setColor(QPalette::ToolTipBase, Qt::white);
        pal.setColor(QPalette::ToolTipText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::Button, QColor(53, 53, 53));
        pal.setColor(QPalette::ButtonText, Qt::white);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Link, QColor(42, 130, 218));
        pal.setColor(QPalette::Highlight, QColor(42, 130, 218));
        pal.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(pal);
        break;
    default:
        qApp->setPalette(QApplication::style()->standardPalette());
        break;
    }
}

void BrowserWindow::openSettings()
{
    SettingsDialog dialog(this);
    dialog.setHomePage(m_homePage);
    dialog.setSearchEngine(m_searchEngine);
    dialog.setJavaScriptEnabled(m_javaScriptEnabled);
    dialog.setZoom(m_zoom);
    dialog.setWindowOpacity(windowOpacity());

    QSettings settings;
    dialog.setTheme(settings.value("theme", 0).toInt());

    if (dialog.exec() == QDialog::Accepted) {
        m_homePage = dialog.getHomePage();
        m_searchEngine = dialog.getSearchEngine();
        m_javaScriptEnabled = dialog.isJavaScriptEnabled();
        m_zoom = dialog.getZoom();
        double opacity = dialog.getWindowOpacity();
        int theme = dialog.getTheme();

        for (int i = 0; i < m_tabWidget->count(); ++i) {
            BrowserTab *tab = qobject_cast<BrowserTab*>(m_tabWidget->widget(i));
            if (tab) {
                applySettingsToTab(tab);
            }
        }
        window()->setWindowOpacity(opacity);
        settings.setValue("theme", theme);

        applyTheme(theme);
        saveSettings();
        statusBar()->showMessage("Настройки сохранены", 2000);
    }
}
