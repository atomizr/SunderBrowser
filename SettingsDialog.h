#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString getHomePage() const;
    QString getSearchEngine() const;
    bool isJavaScriptEnabled() const;
    double getZoom() const;
    double getWindowOpacity() const;
    int getTheme() const;

    void setHomePage(const QString &page);
    void setSearchEngine(const QString &engine);
    void setJavaScriptEnabled(bool enabled);
    void setZoom(double zoom);
    void setWindowOpacity(double opacity);
    void setTheme(int theme);

private:
    QLineEdit *m_homePageEdit;
    QComboBox *m_searchEngineCombo;
    QCheckBox *m_jsCheckBox;
    QSlider *m_zoomSlider;
    QSlider *m_opacitySlider;
    QComboBox *m_themeCombo;

    void setupUI();
};

#endif // SETTINGSDIALOG_H
