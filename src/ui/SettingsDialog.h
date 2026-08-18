#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>

class BrowserController;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent, BrowserController &controller);

private slots:
    void onAccepted();

private:
    BrowserController &m_controller;

    QLineEdit *m_homePageEdit;
    QComboBox *m_searchEngineCombo;
    QCheckBox *m_jsCheckBox;
    QSlider *m_zoomSlider;
    QSlider *m_opacitySlider;
    QComboBox *m_themeCombo;

    void setupUI();
    void loadFromController();
    void saveToController();
};

#endif // SETTINGSDIALOG_H