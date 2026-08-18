#include "MainWindow.h"
#include "BrowserTab.h"
#include "SettingsDialog.h"
#include "controllers/BrowserController.h"
#include "services/IconService.h"
#include "services/BackgroundService.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QGuiApplication>
#include <QScreen>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QWebEngineHistory>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>

MainWindow::MainWindow(BrowserController &controller, QWidget *parent)
    : QMainWindow(parent), m_controller(controller)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect available = screen->availableGeometry();
        int width = qMax(available.width() * 0.8, 800.0);
        int height = qMax(available.height() * 0.8, 600.0);
        resize(width, height);
        move(available.x() + (available.width() - width) / 2,
             available.y() + (available.height() - height) / 2);
    } else {
        resize(1024, 768);
    }

    setAcceptDrops(true);
    setWindowOpacity(m_controller.windowOpacity());

    setupUI();
    applyTheme(m_controller.theme());
    m_controller.setMainWindow(this);
    m_controller.addTab();
    updateNavigationButtons();
}

MainWindow::~MainWindow()
{
    // FIXED: очищаем список вкладок в контроллере, чтобы избежать висячих указателей
    m_controller.clearTabs();
}

void MainWindow::setupUI()
{
    createToolbar();
    statusBar()->showMessage("Готово");

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    setupTabBarStyle();

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);

    setCentralWidget(m_tabWidget);
    setupTabContextMenu();
    m_tabWidget->tabBar()->installEventFilter(this);

    connect(m_backButton, &QToolButton::clicked, this, &MainWindow::onBackClicked);
    connect(m_forwardButton, &QToolButton::clicked, this, &MainWindow::onForwardClicked);
    connect(m_reloadButton, &QToolButton::clicked, this, &MainWindow::onReloadClicked);
    connect(m_addTabButton, &QToolButton::clicked, this, &MainWindow::onAddTabClicked);
    connect(m_homeButton, &QToolButton::clicked, this, &MainWindow::onHomeClicked);
    connect(m_settingsButton, &QToolButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_urlBar, &QLineEdit::returnPressed, this, &MainWindow::onUrlBarReturnPressed);
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    m_backButton = new QToolButton;
    m_backButton->setIcon(IconService::loadIcon("back.png"));
    m_backButton->setToolTip("Назад (Alt+Left)");

    m_forwardButton = new QToolButton;
    m_forwardButton->setIcon(IconService::loadIcon("forward.png"));
    m_forwardButton->setToolTip("Вперёд (Alt+Right)");

    m_reloadButton = new QToolButton;
    m_reloadButton->setIcon(IconService::loadIcon("reload.png"));
    m_reloadButton->setToolTip("Перезагрузить (Ctrl+R / F5)");

    m_addTabButton = new QToolButton;
    m_addTabButton->setIcon(IconService::loadIcon("add.png"));
    m_addTabButton->setToolTip("Новая вкладка (Ctrl+T)");

    m_homeButton = new QToolButton;
    m_homeButton->setIcon(IconService::loadIcon("home.png"));
    m_homeButton->setToolTip("Домашняя страница");

    m_settingsButton = new QToolButton;
    m_settingsButton->setIcon(IconService::loadIcon("settings.png"));
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
}

void MainWindow::setupTabBarStyle()
{
    m_tabWidget->tabBar()->setStyleSheet("");
}

void MainWindow::onTabCloseRequested(int index)
{
    m_controller.closeTab(index);
}

void MainWindow::onCurrentTabChanged(int index)
{
    updateNavigationButtons();
    m_controller.updateCurrentTabUI();
}

void MainWindow::onUrlBarReturnPressed()
{
    m_controller.navigateToUrl(m_urlBar->text());
}

void MainWindow::onBackClicked() { m_controller.goBack(); }
void MainWindow::onForwardClicked() { m_controller.goForward(); }
void MainWindow::onReloadClicked() { m_controller.reload(); }
void MainWindow::onAddTabClicked() { m_controller.addTab(); }
void MainWindow::onHomeClicked() { m_controller.home(); }

void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this, m_controller);
    if (dialog.exec() == QDialog::Accepted) {
        statusBar()->showMessage("Настройки сохранены", 2000);
    }
}

void MainWindow::setupTabContextMenu()
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

void MainWindow::closeCurrentTab()
{
    int index = m_tabWidget->currentIndex();
    if (index != -1)
        m_controller.closeTab(index);
}

void MainWindow::closeOtherTabs()
{
    m_controller.closeOtherTabs();
}

void MainWindow::reloadCurrentTab()
{
    m_controller.reload();
}

void MainWindow::duplicateCurrentTab()
{
    m_controller.duplicateCurrentTab();
}

BrowserTab* MainWindow::currentTab() const
{
    return qobject_cast<BrowserTab*>(m_tabWidget->currentWidget());
}

int MainWindow::currentTabIndex() const
{
    return m_tabWidget->currentIndex();
}

int MainWindow::tabCount() const
{
    return m_tabWidget->count();
}

void MainWindow::setCurrentTab(BrowserTab *tab)
{
    int idx = m_tabWidget->indexOf(tab);
    if (idx >= 0)
        m_tabWidget->setCurrentIndex(idx);
}

void MainWindow::setCurrentIndex(int index)
{
    if (index >= 0 && index < m_tabWidget->count())
        m_tabWidget->setCurrentIndex(index);
}

void MainWindow::setTabTitle(int index, const QString &title)
{
    if (index >= 0 && index < m_tabWidget->count())
        m_tabWidget->setTabText(index, title);
}

void MainWindow::setUrlBarText(const QString &text)
{
    m_urlBar->setText(text);
}

BrowserTab* MainWindow::createTab()
{
    BrowserTab *tab = new BrowserTab(this);
    int index = m_tabWidget->addTab(tab, "Новая вкладка");
    return tab;
}

void MainWindow::removeTab(int index)
{
    QWidget *tab = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    // удаление происходит в контроллере
}

void MainWindow::loadStartPage(BrowserTab *tab)
{
    if (!tab) return;

    QString backgroundStyle = BackgroundService::getRandomBackgroundStyle();

    QString html = QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Новая вкладка</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body {
            height: 100%;
            overflow: hidden;
        }
        body {
            margin: 0;
            padding: 0;
            font-family: "Segoe UI", Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            position: relative;
        }
        .bg-blur {
            position: fixed;
            top: -20px;
            left: -20px;
            right: -20px;
            bottom: -20px;
            z-index: 0;
            %1
            filter: blur(8px);
            transform: scale(1.05);
        }
        .bg-overlay {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            z-index: 1;
            background: rgba(0,0,0,0.15);
        }
        .search-container {
            position: relative;
            z-index: 2;
            background: rgba(245, 240, 235, 0.85);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            padding: 40px 50px;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.15);
            text-align: center;
            width: 540px;
            max-width: 92%;
            border: 1px solid rgba(200, 180, 160, 0.2);
        }
        h1 {
            font-family: "Bahnschrift", "Arial Narrow", sans-serif;
            font-weight: 300;
            font-size: 2.8em;
            letter-spacing: 0.3em;
            color: #2C1810;
            margin-bottom: 4px;
            text-shadow: 0 0 30px rgba(180, 150, 100, 0.15);
        }
        .sub {
            font-size: 0.7em;
            letter-spacing: 0.4em;
            color: #6E5C3E;
            margin-bottom: 25px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 12px;
        }
        .sub::before, .sub::after {
            content: "";
            height: 1px;
            width: 40px;
            background: linear-gradient(90deg, transparent, #B8A088);
        }
        .sub::after {
            background: linear-gradient(90deg, #B8A088, transparent);
        }
        .search-box {
            display: flex;
            align-items: center;
            background: white;
            border-radius: 50px;
            padding: 4px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.08);
            border: 1px solid rgba(200, 180, 160, 0.3);
            transition: all 0.3s ease;
        }
        .search-box:focus-within {
            box-shadow: 0 4px 30px rgba(180, 150, 100, 0.2);
            border-color: #C8A87A;
        }
        input[type="text"] {
            flex: 1;
            padding: 14px 20px;
            font-size: 16px;
            border: none;
            outline: none;
            background: transparent;
            color: #2C1810;
            font-family: "Segoe UI", Arial, sans-serif;
        }
        input[type="text"]::placeholder {
            color: #A09080;
        }
        button {
            padding: 12px 28px;
            font-size: 16px;
            background: linear-gradient(135deg, #DDB880, #C8A87A);
            color: white;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-weight: 500;
            letter-spacing: 0.5px;
            margin: 4px;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(200, 168, 122, 0.4);
        }
        button:active {
            transform: translateY(0px);
        }
        .hint {
            margin-top: 16px;
            font-size: 0.75em;
            color: #8A7A6A;
            letter-spacing: 0.05em;
        }
        .hint kbd {
            display: inline-block;
            padding: 2px 8px;
            border-radius: 4px;
            background: rgba(200, 180, 160, 0.2);
            border: 1px solid rgba(200, 180, 160, 0.3);
            font-size: 0.85em;
            font-family: "Consolas", monospace;
            color: #5C4C3C;
            margin: 0 2px;
        }
    </style>
</head>
<body>
    <div class="bg-blur" style="%1"></div>
    <div class="bg-overlay"></div>
    <div class="search-container">
        <h1>SUNDER</h1>
        <div class="sub">THE FUTURE OF BROWSING</div>
        <form action="https://www.google.com/search" method="get" class="search-box">
            <input type="text" name="q" placeholder="Поиск или введите адрес..." autofocus>
            <button type="submit">Поиск</button>
        </form>
        <div class="hint">
            Нажмите <kbd>Ctrl+L</kbd> или <kbd>Alt+D</kbd> для перехода в адресную строку
        </div>
    </div>
</body>
</html>
)").arg(backgroundStyle).arg(backgroundStyle);

    tab->webView()->setHtml(html, QUrl("about:blank"));
}

void MainWindow::applyTheme(int themeIndex)
{
    QPalette pal;
    switch (themeIndex) {
    case 1: // Тёмная
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
    case 2: // Системная – сброс к системной палитре
        qApp->setPalette(QPalette());  // FIXED: сброс к системной
        break;
    default: // Светлая
        qApp->setPalette(QApplication::style()->standardPalette());
        break;
    }
}

void MainWindow::updateNavigationButtons()
{
    BrowserTab *tab = currentTab();
    if (tab) {
        m_backButton->setEnabled(tab->webView()->history()->canGoBack());
        m_forwardButton->setEnabled(tab->webView()->history()->canGoForward());
    } else {
        m_backButton->setEnabled(false);
        m_forwardButton->setEnabled(false);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isValid()) {
            BrowserTab *tab = currentTab();
            if (tab) {
                tab->navigateToUrl(url);
                break;
            }
        }
    }
    event->acceptProposedAction();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
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
            m_controller.addTab();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_W) {
            closeCurrentTab();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_L) {
            m_urlBar->setFocus();
            m_urlBar->selectAll();
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
    if ((event->modifiers() & Qt::AltModifier) && event->key() == Qt::Key_D) {
        m_urlBar->setFocus();
        m_urlBar->selectAll();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        m_controller.stop();
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_tabWidget->tabBar() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::MiddleButton) {
            int index = m_tabWidget->tabBar()->tabAt(mouseEvent->pos());
            if (index != -1) {
                m_controller.closeTab(index);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
