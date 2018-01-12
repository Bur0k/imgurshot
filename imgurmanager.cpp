#include "imgurmanager.h"

#include <QtNetwork/QNetworkRequest>
#include <QUrlQuery>
#include <QUrl>
#include <QByteArray>
#include <string>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QHttpMultiPart>
#include <QHttpPart>

#include <fstream>

ImgurManager::ImgurManager()
    : settings(QSettings::IniFormat, QSettings::UserScope, "imgurshot"),
      currentState(State::ClientIdAndSecretNeeded)
{
    connect(&nam, &QNetworkAccessManager::finished,
            this, &ImgurManager::processFinishedReply);

    clientId = settings.value("client-id", "").toString();
    clientSecret = settings.value("client-secret", "").toString();

    accessToken = settings.value("access_token", "").toString();
    refreshToken = settings.value("refresh_token", "").toString();
}

ImgurManager::State ImgurManager::getState()
{
    return currentState;
}

void ImgurManager::checkState()
{
    if (clientId == "" || clientSecret == "")
    {
        currentState = State::ClientIdAndSecretNeeded;
    }
    else if (accessToken == "" || refreshToken == "")
    {
        currentState = State::AccessAndRefreshTokenNeeded;
    }
    else
    {
        currentState = State::ReadyToUpload;
    }


    if (currentState == State::ClientIdAndSecretNeeded)
    {
        emit requestClientIdAndSecret(QUrl("https://api.imgur.com/oauth2/addclient"));
    }
    else if (currentState == State::AccessAndRefreshTokenNeeded)
    {
        currentState = State::WaitingForPin;
        emit requestPin("https://api.imgur.com/oauth2/authorize?client_id=" + clientId + "&response_type=pin");
    }
}

void ImgurManager::setClientIdAndSecret(QString ClientId, QString ClientSecret)
{
    clientId = ClientId;
    clientSecret = ClientSecret;

    settings.setValue("client-id", clientId);
    settings.setValue("client-secret", clientSecret);

    checkState();
}

void ImgurManager::processPin(QString pin)
{
    currentState = State::AccessAndRefreshTokenNeeded;
    QString postString = "client_id=" + clientId + "&client_secret=" + clientSecret + "&grant_type=pin&pin=" + pin;

    QNetworkRequest generateNewTokenRequest(QUrl("https://api.imgur.com/oauth2/token.json"));
    generateNewTokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    nam.post(generateNewTokenRequest, postString.toUtf8());
}

void ImgurManager::upload(QString filename, QString title)
{
    if (currentState != State::ReadyToUpload)
    {
        qDebug() << "Not ready to Upload" << endl;
        return;
    }
    currentState = State::Uploading;

    std::ifstream imageStream(filename.toStdString(), std::ios::binary | std::ios::ate);

    auto fileSize = imageStream.tellg();
    if (fileSize == -1)
    {
        qDebug() << "Filesize of -1" << endl;
        return;
    }

    imageStream.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!imageStream.read(buffer.data(), fileSize))
    {
        qDebug() << "Reading file failed" << endl;
        return;
    }

    QByteArray imageByteArray(buffer.data(), fileSize);


    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);


    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
    imagePart.setBody(imageByteArray);

    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"type\""));
    typePart.setBody("file");

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());


    multiPart->append(imagePart);
    multiPart->append(typePart);
    multiPart->append(titlePart);

    QByteArray boundary = multiPart->boundary();

    QNetworkRequest generateNewTokenRequest(QUrl("https://api.imgur.com/3/image.json"));
    generateNewTokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    generateNewTokenRequest.setRawHeader("Authorization", "Client-ID " + clientId.toUtf8());
    generateNewTokenRequest.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());

    nam.post(generateNewTokenRequest, multiPart);
}

void ImgurManager::processFinishedReply(QNetworkReply *reply)
{
    reply->deleteLater();

    auto error = reply->error();
    handleHTTPSRespons(reply->readAll());

    if (error != QNetworkReply::NoError)
    {
        emit networkErrorOccured(error);
        return;
    }

}

void ImgurManager::handleHTTPSRespons(QByteArray byteArray)
{
    QJsonDocument rootDocument = QJsonDocument::fromJson(byteArray);

    qDebug() << rootDocument;
    QJsonObject root;

    if(!rootDocument.isNull())
    {
        if(rootDocument.isObject())
            root = rootDocument.object();
        else
            qDebug() << "Document is not an object" << endl;
    }
    else
        qDebug() << "Invalid JSON...\n" << endl;

    if (currentState == State::AccessAndRefreshTokenNeeded)
    {
        accessToken = root["access_token"].toString();
        refreshToken = root["refresh_token"].toString();

        settings.setValue("access_token", accessToken);
        settings.setValue("refresh_token", refreshToken);

        checkState();

        settings.sync();
        exit(-1);
    }
    else if (currentState == State::Uploading)
    {
        currentState = State::ReadyToUpload;
        auto dataObject = root["data"].toObject();
        QString link = dataObject["link"].toString();
        emit uploaded(link);
    }
}



