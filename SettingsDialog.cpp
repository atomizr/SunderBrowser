#include "SettingsDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();
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

QString SettingsDialog::getHomePage() const { return m_homePageEdit->text(); }
QString SettingsDialog::getSearchEngine() const { return m_searchEngineCombo->currentData().toString(); }
bool SettingsDialog::isJavaScriptEnabled() const { return m_jsCheckBox->isChecked(); }
double SettingsDialog::getZoom() const { return m_zoomSlider->value() / 100.0; }
double SettingsDialog::getWindowOpacity() const { return m_opacitySlider->value() / 100.0; }
int SettingsDialog::getTheme() const { return m_themeCombo->currentIndex(); }

void SettingsDialog::setHomePage(const QString &page) { m_homePageEdit->setText(page); }
void SettingsDialog::setSearchEngine(const QString &engine) {
    int index = m_searchEngineCombo->findData(engine);
    if (index >= 0) m_searchEngineCombo->setCurrentIndex(index);
}
void SettingsDialog::setJavaScriptEnabled(bool enabled) { m_jsCheckBox->setChecked(enabled); }
void SettingsDialog::setZoom(double zoom) { m_zoomSlider->setValue(int(zoom * 100)); }
void SettingsDialog::setWindowOpacity(double opacity) { m_opacitySlider->setValue(int(opacity * 100)); }
void SettingsDialog::setTheme(int theme) { m_themeCombo->setCurrentIndex(theme); }