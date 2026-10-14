#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineCertificateError>

class WebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

signals:
    void newTabRequested(const QUrl &url);

protected:
    // Виртуальный метод — можно переопределять
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString &message,
                                  int lineNumber,
                                  const QString &sourceID) override;

private slots:
    // Сигналы — подключаются через connect()
    void onFullScreenRequested(QWebEngineFullScreenRequest request);
    void onCertificateError(const QWebEngineCertificateError &error);
    void onFeaturePermissionRequested(const QUrl &origin, Feature feature);
    void onNewWindowRequested(QWebEngineNewWindowRequest &request);
};

#endif // WEBPAGE_H