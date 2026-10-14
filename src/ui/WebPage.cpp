#include "WebPage.h"
#include "utils/Logger.h"

#include <QMessageBox>
#include <QWebEngineView>
#include <QWebEngineNewWindowRequest>

namespace {

// Паттерны безвредного шума от сайтов — не наш код, не наша проблема.
// Если сработал фильтр — сообщение уходит в debug.log, а не в sunder.log.
bool isNoise(const QString &msg)
{
    static const char *patterns[] = {
        "Refused to prefetch content",     // <link rel=prefetch> при CSP default-src 'none'
        "Refused to load the script",      // CSP-блокировка сторонних скриптов
        "Refused to connect to",           // CSP-блокировка XHR/WS
        "Refused to frame",                // CSP-блокировка iframe
        "ChunkLoadError",                  // обрыв загрузки куска JS у SPA
        "Unrecognized feature",            // старые feature-policy / permissions-policy
        "Permissions-Policy header",       // устаревшие политики сайта
        "Deprecated API for given entry type",
        "Origin trial controlled feature", // 'unload', 'interest-cohort'
    };
    for (const char *p : patterns) {
        if (msg.contains(QLatin1String(p)))
            return true;
    }
    return false;
}

} // namespace

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    connect(this, &QWebEnginePage::fullScreenRequested,
            this, &WebPage::onFullScreenRequested);
    connect(this, &QWebEnginePage::certificateError,
            this, &WebPage::onCertificateError);
    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &WebPage::onFeaturePermissionRequested);
    connect(this, &QWebEnginePage::newWindowRequested,
            this, &WebPage::onNewWindowRequested);
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                       const QString &message,
                                       int lineNumber,
                                       const QString &sourceID)
{
    if (level != ErrorMessageLevel)
        return;

    // Шум от сайтов — в debug.log, не в основной.
    if (isNoise(message)) {
        LOG_DEBUG_ONLY("[JS-noise] {}", message.toStdString());
        return;
    }

    // Что-то реально нестандартное — пишем в основной лог.
    LOG_WARN("[JS] {} ({}:{})",
             message.toStdString(),
             sourceID.toStdString(),
             lineNumber);
}

void WebPage::onFullScreenRequested(QWebEngineFullScreenRequest request)
{
    request.accept();

    auto *view = qobject_cast<QWebEngineView*>(parent());
    if (!view)
        return;
    QWidget *top = view->window();
    if (!top)
        return;

    if (request.toggleOn()) {
        top->setProperty("wasMaximized", top->isMaximized());
        top->showFullScreen();
    } else {
        if (top->property("wasMaximized").toBool())
            top->showMaximized();
        else
            top->showNormal();
    }
}

void WebPage::onCertificateError(const QWebEngineCertificateError &error)
{
    if (!error.isOverridable()) {
        // const_cast: методы acceptCertificate()/rejectCertificate() не const
        // в Qt 6.5.3, хотя сигнал передаёт const-ссылку.
        const_cast<QWebEngineCertificateError&>(error).rejectCertificate();
        LOG_WARN("SSL: non-overridable error for {}: {}",
                 error.url().toString().toStdString(),
                 error.description().toStdString());
        return;
    }

    const auto answer = QMessageBox::warning(
        nullptr,
        tr("Ошибка сертификата"),
        tr("Сертификат сайта %1 не может быть проверен:\n%2\n\nПродолжить на свой риск?")
            .arg(error.url().toString(), error.description()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer == QMessageBox::Yes)
        const_cast<QWebEngineCertificateError&>(error).acceptCertificate();
    else
        const_cast<QWebEngineCertificateError&>(error).rejectCertificate();
}

void WebPage::onFeaturePermissionRequested(const QUrl &origin, Feature feature)
{
    QString name;
    switch (feature) {
    case MediaAudioCapture:          name = tr("микрофон");          break;
    case MediaVideoCapture:          name = tr("камера");            break;
    case MediaAudioVideoCapture:     name = tr("камера и микрофон"); break;
    case DesktopVideoCapture:
    case DesktopAudioVideoCapture:   name = tr("запись экрана");     break;
    case Geolocation:                name = tr("геолокация");        break;
    case Notifications:              name = tr("уведомления");       break;
    case MouseLock:                  name = tr("захват курсора");    break;
    default:                         name = tr("неизвестный ресурс");break;
    }

    const auto answer = QMessageBox::question(
        nullptr,
        tr("Запрос разрешения"),
        tr("Сайт %1 запрашивает доступ:\n%2\n\nРазрешить?")
            .arg(origin.host(), name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    setFeaturePermission(origin, feature,
                         answer == QMessageBox::Yes
                             ? PermissionGrantedByUser
                             : PermissionDeniedByUser);
}

void WebPage::onNewWindowRequested(QWebEngineNewWindowRequest &request)
{
    emit newTabRequested(request.requestedUrl());
}