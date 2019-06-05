#include "json.h"
#include "mainwindow.h"
#include <QString>
#include <QSettings>
#include <QtNetwork>
#include <QDateTime>
#include <QDate>

json::json(QObject *parent) : QObject(parent)
{

}

QJsonObject json::Authorization(QString URL, QString log, QString pass)
{
    QSettings settings("config.ini", QSettings::IniFormat);

    url.setUrl(URL +"/api_jsonrpc.php");

    rqs.setUrl(url);
    rqs.setHeader(QNetworkRequest::ContentTypeHeader, "application/json-rpc");

    jsonRequest["jsonrpc"] = "2.0";
    jsonRequest["method"]="user.login";
    jsonRequest["id"] = 1;
    jsonRequest["auth"] = QJsonValue();

    QJsonObject paramsObj;
    paramsObj["user"] = log;
    paramsObj["password"] = pass;
    jsonRequest["params"] = paramsObj;

    qDebug()<<jsonRequest;

    QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    auto reply = manager.post(rqs, strJsonRequest.toUtf8());
    loop.exec();

    QJsonObject jsonResponse = QJsonDocument::fromJson(reply->readAll()).object();
    settings.setValue("authkey",jsonResponse["result"].toString());

    return jsonResponse;
}

QJsonArray json::GetProblemsIDs()
{
    QJsonArray result;
    QSettings settings("config.ini", QSettings::IniFormat);
    int currentDateTime;

    currentDateTime = (int)QDateTime::currentDateTime().toTime_t() - settings.value("timer").toInt();

    if(settings.contains("authkey"))
    {
        QString URL = settings.value("IP").toString();

        url.setUrl(URL +"/api_jsonrpc.php");

        QNetworkRequest rqs(url);
        rqs.setHeader(QNetworkRequest::ContentTypeHeader, "application/json-rpc");

        jsonRequest["jsonrpc"] = "2.0";
        jsonRequest["method"]="problem.get";
        jsonRequest["id"] = 1;
        jsonRequest["auth"] = settings.value("authkey").toString();

        QJsonObject paramsObj;
        paramsObj["time_from"] = currentDateTime;
        jsonRequest["params"] = paramsObj;

        //qDebug()<< jsonRequest;

        QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));

        QEventLoop loop;
        connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
        auto reply = manager.post(rqs, strJsonRequest.toUtf8());
        loop.exec();

        QJsonArray jsonResponse = QJsonObject(QJsonDocument::fromJson(reply->readAll()).object())["result"].toArray();

        for (auto i = jsonResponse.begin();i != jsonResponse.end(); ++i)
        {
            QJsonObject jObj = i->toObject();
            result.append(QJsonValue(jObj["eventid"]));
        }
    }
    else
    {

    }

    return result;
}

QJsonArray json::GetProblemsAlerts(QJsonArray problemsIDs)
{
    QSettings settings("config.ini", QSettings::IniFormat);

    QString URL = settings.value("IP").toString();
    url.setUrl(URL +"/api_jsonrpc.php");

    QNetworkRequest rqs(url);
    rqs.setHeader(QNetworkRequest::ContentTypeHeader, "application/json-rpc");

    jsonRequest["jsonrpc"] = "2.0";
    jsonRequest["method"]="alert.get";
    jsonRequest["id"] = 1;
    jsonRequest["auth"] = settings.value("authkey").toString();

    QJsonObject filter;
    QJsonObject search;
    filter["sendto"] = "alerts";
    search["message"] = "HOST:";

    QJsonObject paramsObj;
    paramsObj["eventids"] = problemsIDs;
    paramsObj["filter"] = filter;
    paramsObj["search"] = search;
    paramsObj["startSearch"] = QJsonValue(true);
    paramsObj["sortfield"] = "clock";
    paramsObj["sortorder"] = "DESC";

    jsonRequest["params"] = paramsObj;

    QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));
    //qDebug() << jsonRequest;

    QEventLoop loop;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    auto reply = manager.post(rqs, strJsonRequest.toUtf8());
    loop.exec();

    QJsonObject jsonResponse = QJsonDocument::fromJson(reply->readAll()).object();

   // qDebug() << jsonResponse;

    QJsonArray result = jsonResponse["result"].toArray();

    return result;
}

void json::GetProblemsAlertsText(QJsonObject jObj)
{











/*

    text.chop(1);
    QStringList list = text.split("\r\n");
    for(auto i = 0; i != 5; ++i)
    {
       qDebug()<<list.at(i);
    }

*/
}





