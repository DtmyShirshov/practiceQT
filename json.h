#ifndef JSON_H
#define JSON_H

#include <QObject>
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


class json : public QObject
{
    Q_OBJECT
public:
    explicit json(QObject *parent = nullptr);
private:
    QNetworkAccessManager manager;
    QUrl url;
    QJsonObject jsonRequest;
    QNetworkRequest rqs;
    QString authkey;

signals:

public slots:
    QJsonObject Authorization(QString URL, QString log, QString pass);
    QJsonArray GetProblemsIDs(int currentDateTime);
    QJsonArray GetProblemsAlerts(QJsonArray problemsIDs);
private slots:
    QJsonObject GetJsonResonse(QString strJsonRequest);
};

#endif // JSON_H
