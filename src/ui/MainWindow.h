#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QToolButton>
#include <QTabWidget>
#include <QStatusBar>

class BrowserController;
class BrowserTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(BrowserController &controller, QWidget *parent = nullptr);
    ~MainWindow();

    // Методы для контроллера
    BrowserTab* currentTab() const;
    int currentTabIndex() const;
    int tabCount() const;
    void setCurrentTab(BrowserTab *tab);
    void setCurrentIndex(int index);
    void setTabTitle(int index, const QString &title);
    void setUrlBarText(const QString &text);
    BrowserTab* createTab();
    void removeTab(int index);
    void loadStartPage(BrowserTab *tab);
    void applyTheme(int themeIndex);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onTabCloseRequested(int index);
    void onCurrentTabChanged(int index);
    void onUrlBarReturnPressed();
    void onBackClicked();
    void onForwardClicked();
    void onReloadClicked();
    void onAddTabClicked();
    void onHomeClicked();
    void onSettingsClicked();
    void setupTabContextMenu();
    void closeCurrentTab();
    void closeOtherTabs();
    void reloadCurrentTab();
    void duplicateCurrentTab();

private:
    BrowserController &m_controller;
    QTabWidget *m_tabWidget;
    QLineEdit *m_urlBar;
    QToolButton *m_backButton;
    QToolButton *m_forwardButton;
    QToolButton *m_reloadButton;
    QToolButton *m_addTabButton;
    QToolButton *m_homeButton;
    QToolButton *m_settingsButton;

    void setupUI();
    void createToolbar();
    void setupTabBarStyle();
    void updateNavigationButtons();
};

#endif // MAINWINDOW_H