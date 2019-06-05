#ifndef JSON_H
#define JSON_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QSettings>


class json : public QObject
{
    Q_OBJECT
public:
    explicit json(QObject *parent = nullptr);
    QNetworkAccessManager manager;
    QUrl url;
    QJsonObject jsonRequest;
    QNetworkRequest rqs;

signals:

public slots:
    QJsonObject Authorization(QString URL, QString log, QString pass);
    QJsonArray GetProblemsIDs();
    QJsonArray GetProblemsAlerts(QJsonArray problemsIDs);
    void GetProblemsAlertsText(QJsonObject jObj);
};

#endif // JSON_H
