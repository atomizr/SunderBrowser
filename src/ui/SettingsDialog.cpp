#include "SettingsDialog.h"
#include "controllers/BrowserController.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent, BrowserController &controller)
    : QDialog(parent), m_controller(controller)
{
    setupUI();
    loadFromController();
    connect(this, &QDialog::accepted, this, &SettingsDialog::onAccepted);
}

void SettingsDialog::setupUI()
{
    setWindowTitle("Настройки");

    m_homePageEdit = new QLineEdit();
    m_searchEngineCombo = new QComboBox();
    m_searchEngineCombo->addItem("Google", "https://www.google.com/search?q=");
    m_searchEngineCombo->addItem("Bing", "https://www.bing.com/search?q=");
    m_searchEngineCombo->addItem("DuckDuckGo", "https://duckduckgo.com/?q=");

    m_jsCheckBox = new QCheckBox("Включить JavaScript");

    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(50, 300);
    m_zoomSlider->setTickInterval(10);
    m_zoomSlider->setTickPosition(QSlider::TicksBelow);
    QLabel *zoomLabel = new QLabel("Масштаб страницы (%):");
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(m_zoomSlider);
    zoomLayout->addStretch();

    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    QLabel *opacityLabel = new QLabel("Прозрачность окна (%):");
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addStretch();

    m_themeCombo = new QComboBox();
    m_themeCombo->addItem("Светлая");
    m_themeCombo->addItem("Тёмная");
    m_themeCombo->addItem("Системная");

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Домашняя страница:", m_homePageEdit);
    formLayout->addRow("Поисковая система:", m_searchEngineCombo);
    formLayout->addRow(m_jsCheckBox);
    formLayout->addRow(zoomLabel, m_zoomSlider);
    formLayout->addRow(opacityLabel, m_opacitySlider);
    formLayout->addRow("Тема:", m_themeCombo);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

void SettingsDialog::loadFromController()
{
    m_homePageEdit->setText(m_controller.homePage());
    int index = m_searchEngineCombo->findData(m_controller.searchEngine());
    if (index >= 0) m_searchEngineCombo->setCurrentIndex(index);
    m_jsCheckBox->setChecked(m_controller.javaScriptEnabled());
    m_zoomSlider->setValue(int(m_controller.zoom() * 100));
    m_opacitySlider->setValue(int(m_controller.windowOpacity() * 100));
    m_themeCombo->setCurrentIndex(m_controller.theme());
}

void SettingsDialog::saveToController()
{
    m_controller.setHomePage(m_homePageEdit->text());
    m_controller.setSearchEngine(m_searchEngineCombo->currentData().toString());
    m_controller.setJavaScriptEnabled(m_jsCheckBox->isChecked());
    m_controller.setZoom(m_zoomSlider->value() / 100.0);
    m_controller.setWindowOpacity(m_opacitySlider->value() / 100.0);
    m_controller.setTheme(m_themeCombo->currentIndex());
}

void SettingsDialog::onAccepted()
{
    saveToController();
}