#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>
#include <QSettings>
#include <QtNetwork>
#include <QSound>
#include "json.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    json jsn;





protected:
    void closeEvent(QCloseEvent * event);

private slots:
    /* Слот, который будет принимать сигнал от события нажатия на иконку приложения в трее */
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    /* Слот на таймаут таймера */
    void onTimeout();
    /* Слот закрытия приложения */
    void ExitApp();
    /* Слот скрытия приложения в трей */
    void HideApp();

    void on_HideApp_triggered();

    void on_Settings_triggered();

    void on_Exit_triggered();
    /* Слот обработки текста проблемы, добавления его в таблицу, оповещения */
    void SetItem(QString text);

    void GetProblems();


    void on_pushButton_clicked();

private:
    // Объявление объекта будущей иконки приложения для трея
    QSystemTrayIcon *trayIcon;
    QNetworkAccessManager manager;
    Ui::MainWindow *ui;
    QJsonArray alreadyExists;
    QSound *sound;

};

#endif // MAINWINDOW_H
