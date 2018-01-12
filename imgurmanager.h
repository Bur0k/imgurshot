#ifndef IMGURMANAGER_H
#define IMGURMANAGER_H

#include <QObject>
#include <QSettings>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class ImgurManager : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        ClientIdAndSecretNeeded,
        WaitingForPin,
        AccessAndRefreshTokenNeeded,
        ReadyToUpload,
        Uploading
    };
private:
    QSettings settings;

    QNetworkAccessManager nam;

    QString clientId;
    QString clientSecret;

    QString accessToken;
    QString refreshToken;

    State currentState;

    void handleHTTPSRespons(QByteArray byteArray);
public:
    ImgurManager();

    State getState();
    void checkState();
    void setClientIdAndSecret(QString clientId, QString clientSecret);

    void processPin(QString pin);

    void upload(QString filename, QString title);
public slots:
    void processFinishedReply(QNetworkReply *reply);

signals:
    void networkErrorOccured(int code);

    void requestClientIdAndSecret(QUrl url);
    void requestPin(QUrl url);

    void uploaded(QString link);
};

#endif // IMGURMANAGER_H
