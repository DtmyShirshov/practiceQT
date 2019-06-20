#include "json.h"
#include "mainwindow.h"
#include <QString>
#include <QSettings>

json::json(QObject *parent) : QObject(parent)
{

}

QJsonObject json::Authorization(QString URL, QString log, QString pass)
{
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

    QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));

    QJsonObject jsonResponse = GetJsonResonse(strJsonRequest);

    if(jsonResponse.contains("error") || jsonResponse.isEmpty())
    {
        while (jsonResponse.contains("error") || jsonResponse.isEmpty())
        {
            jsonResponse = GetJsonResonse(strJsonRequest);
        }
    }

    authkey = jsonResponse["result"].toString();
    qDebug() << jsonResponse;
    return jsonResponse;
}

QJsonArray json::GetProblemsIDs(int currentDateTime)
{
    QJsonArray result;

    jsonRequest["jsonrpc"] = "2.0";
    jsonRequest["method"]="problem.get";
    jsonRequest["id"] = 1;
    jsonRequest["auth"] = authkey;

    QJsonObject paramsObj;
    paramsObj["time_from"] = currentDateTime;
    jsonRequest["params"] = paramsObj;

    QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));


    QJsonObject jsonResponse = GetJsonResonse(strJsonRequest);

    if(jsonResponse.contains("error") || jsonResponse.isEmpty())
    {
        while (jsonResponse.contains("error") || jsonResponse.isEmpty())
        {
            jsonResponse = GetJsonResonse(strJsonRequest);
        }
    }

    QJsonArray resultArr = jsonResponse["result"].toArray();
    for (auto i = resultArr.begin();i != resultArr.end(); ++i)
    {
        QJsonObject jObj = i->toObject();
        result.append(QJsonValue(jObj["eventid"]));
    }

    return result;
}

QJsonArray json::GetProblemsAlerts(QJsonArray problemsIDs)
{
    QJsonArray result;

    jsonRequest["jsonrpc"] = "2.0";
    jsonRequest["method"]="alert.get";
    jsonRequest["id"] = 1;
    jsonRequest["auth"] = authkey;

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

    QJsonObject jsonResponse = GetJsonResonse(strJsonRequest);

    if(jsonResponse.contains("error") || jsonResponse.isEmpty())
    {
        while (jsonResponse.contains("error") || jsonResponse.isEmpty())
        {
            jsonResponse = GetJsonResonse(strJsonRequest);
        }
    }

    result = jsonResponse["result"].toArray();

    return result;
}

QJsonObject json::GetJsonResonse(QString strJsonRequest)
{
    QEventLoop loop;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    auto reply = manager.post(rqs, strJsonRequest.toUtf8());
    loop.exec();

    QJsonObject jsonResponse = QJsonDocument::fromJson(reply->readAll()).object();

    return jsonResponse;
}





