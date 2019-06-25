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

    QJsonObject jsonRequest;
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

    if(jsonResponse.isEmpty())
    {
        while (jsonResponse.isEmpty())
        {
            jsonResponse = GetJsonResonse(strJsonRequest);
        }
    }

    authkey = jsonResponse["result"].toString();
    qDebug() << "Ключ: " + authkey;
    return jsonResponse;
}

QJsonArray json::GetProblems(int launchTime)
{
    //создаем объект запроса
    QJsonObject jsonRequest;
    //заполняем запрос, указываем версию jsonrpc, указываем метод, даем id,
    //указываем, ранеее полученный, код аутентификации
    jsonRequest["jsonrpc"] = "2.0";
    jsonRequest["method"]="trigger.get";
    jsonRequest["id"] = 1;
    jsonRequest["auth"] = authkey;
    //заполняем параметры
    QJsonObject paramsObj;
    paramsObj["lastChangeSince"] = launchTime;
    QJsonArray select;
    select.append("name");
    select.append("lastvalue");
    paramsObj["selectHosts"] = select;
    paramsObj["selectItems"] = select;
    paramsObj["skipDependent"] = 1;
    paramsObj["expandComment"] = 1;
    paramsObj["monitored"] = 1;

    QJsonArray output;
    output.append("triggerid");
    output.append("description");
    output.append("priority");
    output.append("lastchange");

    QJsonObject filter;
    filter["value"] = 1;

    paramsObj["output"] = output;
    paramsObj["filter"] = filter;

    jsonRequest["params"] = paramsObj;

    //преобразуем сериализованный запрос в строку
    QString strJsonRequest(QJsonDocument(jsonRequest).toJson(QJsonDocument::Compact));

    //отправляем на сервер
    QJsonObject jsonResponse = GetJsonResonse(strJsonRequest);

    //получем ответ до тех пор, пока он не будет содержать ошибку или не будет пустым
    if(jsonResponse.contains("error") || jsonResponse.isEmpty())
    {
        while (jsonResponse.contains("error") || jsonResponse.isEmpty())
        {
            jsonResponse = GetJsonResonse(strJsonRequest);
        }
    }
    //преобразуем полученный результат в QJsonArray
    QJsonArray resultArr = jsonResponse["result"].toArray();
    //возвращем результат
    return resultArr;
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





