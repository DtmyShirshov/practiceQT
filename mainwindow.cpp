#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingwindow.h"
#include "json.h"
#include <QtWidgets>
#include <QTimer>
#include <QMessageBox>
#include <QtMultimedia/QSound>
#include <QApplication>
#include <QSettings>
#include <QtNetwork>
#include <QString>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    /* Забираем заданный в настройках интервал таймера из файла config.ini */
    QSettings settings("config.ini", QSettings::IniFormat);
    /* запоминаем время отправления первого запроса */
    launchDateTime = (int)QDateTime::currentDateTime().toTime_t() + settings.value("timer").toInt();

    int timerInterval = settings.value("timer").toInt() * 1000;

    jsn.Authorization(settings.value("IP").toString(), settings.value("login").toString(), settings.value("password").toString());

    ui->setupUi(this);
    this->setWindowTitle("My First Tray Programm");

    /* Задаем иконку и подсказку при наведении на неё */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_DesktopIcon));
    trayIcon->setToolTip("ZabbixNotifications");

    /* Создаем контекстное меню */
    QMenu* menu = new QMenu(this);

    QAction* hideWindow = new QAction("Свернуть окно", this);
    QAction* viewWindow = new QAction("Развернуть окно", this);
    QAction* quitAction = new QAction("Выход", this);

    /* подключаем сигналы нажатий на пункты меню к соответсвующим слотам.
       Первый пункт меню разворачивает приложение из трея,
       второй пункт сворачивает приложение в трей,
       третий пункт меню завершает приложение.
    */
    connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(HideApp()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(ExitApp()));



    /* Добавляем кнопки в контекстное меню */
    menu->addAction(viewWindow);
    menu->addAction(hideWindow);
    menu->addAction(quitAction);

    /* Задаем контекстное меню кнопке в трее и показываем её */
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    /* Объявляем таймер, задаем ему интервал, подключаем сигнал таймаута к слоту onTimeout() */
    QTimer* timer = new QTimer(this);
    timer->start(timerInterval);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    ui->tableWidget->setColumnHidden(1, true);

    sorting = "time";

    sound = new QSound(":\\new\\alarm_02.wav");
    qDebug()<< sorting;
    qDebug()<< QString::number(launchDateTime);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* Действие на таймаут таймера */
void MainWindow::onTimeout(){
    GetProblems();
    DeleteResolvedProblems();
    qDebug()<<"Отправил запрос";
}

void MainWindow::GetProblems(){

    QSettings settings("config.ini", QSettings::IniFormat);
    QJsonArray problemsIDs(jsn.GetProblemsIDs((int)QDateTime::currentDateTime().toTime_t() - settings.value("timer").toInt())); //массив ID полученных проблем
    QJsonArray problemIDsOK;

    for(int i = 0; i < problemsIDs.count(); ++i)
    {
        if(!alreadyExists.contains(problemsIDs.at(i)))
        {
            alreadyExists.append(QJsonValue(problemsIDs.at(i)));
            problemIDsOK.append(QJsonValue(problemsIDs.at(i)));
        }
        else
        {
            qDebug()<< "Проблема уже выведена в таблицу";
        }
    }

    QJsonArray problems = jsn.GetProblemsAlerts(problemIDsOK);

    for(auto i = problems.begin(); i != problems.end(); ++i)
    {
        QJsonObject jO = i->toObject();
        SetItem(jO["message"].toString());
    }
}

void MainWindow::SetItem(QString problemMessage)
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

    problemMessage.chop(1);
    QStringList problemMessagelist = problemMessage.split("\r\n");

    QString host = problemMessagelist.at(0);
    host.remove("HOST: ");
    qDebug()<<host;

    QString problem = problemMessagelist.at(1);
    problem.remove("TRIGGER_NAME: ");
    qDebug()<<problem;

    QString severitity = problemMessagelist.at(3);
    severitity.remove("TRIGGER_SEVERITY: ");

    QString event_id = problemMessagelist.at(9);
    event_id.remove("EVENT_ID: ");
    qDebug()<<severitity;



    QTableWidgetItem* itemSeveritity = new QTableWidgetItem();

    QTableWidgetItem* itemProblem = new QTableWidgetItem();
    itemProblem->setText(problem);

    QColor color;
    if(severitity == "Информация")
    {
        color.setRgb(30,144,255,255);
        itemSeveritity->setText("5");
        itemProblem->setBackgroundColor(color);
    }
    if(severitity == "Предупреждение")
    {
        color.setRgb(255,215,0,255);
        itemSeveritity->setText("4");
        itemProblem->setBackgroundColor(color);
    }
    if(severitity == "Средняя")
    {
        color.setRgb(255,165,0,255);
        itemSeveritity->setText("3");
        itemProblem->setBackgroundColor(color);
    }
    if(severitity == "Высокая")
    {
        color.setRgb(255,99,71,255);
        itemSeveritity->setText("2");
        itemProblem->setBackgroundColor(color);
    }
    if(severitity == "Чрезвычайная")
    {
        color.setRgb(255,0,0,255);
        itemSeveritity->setText("1");
        itemProblem->setBackgroundColor(color);
    }

    QString dateTime = problemMessagelist.at(4);
    dateTime.remove("DATETIME: ");

    QTableWidgetItem* itemHost = new QTableWidgetItem();
    itemHost->setText(host);

    QTableWidgetItem* itemDateTime = new QTableWidgetItem();
    itemDateTime->setText(dateTime);

    QTableWidgetItem* itemEvent_id = new QTableWidgetItem();
    itemEvent_id->setText(event_id);

    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, itemHost);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, itemProblem);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, itemSeveritity);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, itemDateTime);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 4, itemEvent_id);

    ui->tableWidget->resizeColumnsToContents();

    if(sorting == "time")
    {
        ui->tableWidget->sortItems(0,Qt::DescendingOrder);
    }
    if(sorting == "severitity")
    {
        ui->tableWidget->sortItems(1);
    }

    sound->play();

    QMessageBox* msg = new QMessageBox(this);
    msg->setWindowTitle(severitity + "!");
    msg->setText(dateTime + "\n" + host + "\n" + problem);
    msg->setWindowFlag(Qt::WindowStaysOnTopHint);
    msg->show();
}

void MainWindow::DeleteResolvedProblems(){

    QJsonArray problemsIDs(jsn.GetProblemsIDs(launchDateTime));

    qDebug()<< QString::number(launchDateTime);

    for(int i = 0; i < alreadyExists.count(); ++i)
    {
        if(!problemsIDs.contains(alreadyExists.at(i)))
        {
            QList<QTableWidgetItem*> item = ui->tableWidget->findItems(alreadyExists.at(i).toString(), Qt::MatchExactly);
            if(!item.empty())
            {
                for(int j = 0; j < item.count(); j++)
                {
                 int row = item.at(j)->row();
                 ui->tableWidget->removeRow(row);
                }

                alreadyExists.removeAt(i);
            }
        }
        else
        {
            continue;
        }
    }
}



/* Метод, который обрабатывает событие закрытия окна приложения */
void MainWindow::closeEvent(QCloseEvent * event)
{
    /* Если окно видимо, то завершение приложения игнорируется, а окно просто скрывается */
    if(this->isVisible())
    {
        event->ignore();
        this->HideApp();
    }
}

/* Метод, который обрабатывает нажатие на иконку приложения в трее */
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
    /* если окно видимо, то оно скрывается, и наоборот, если скрыто, то разворачивается на экран */
        if(!this->isVisible())
        {
            this->show();
        }
        else
        {
            hide();
        }
        break;
    default:
        break;
    }
}

void MainWindow::on_Settings_triggered()
{
    SettingWindow sw;
    sw.exec();
}

void MainWindow::on_HideApp_triggered()
{
    HideApp();
}

void MainWindow::on_Exit_triggered()
{
    ExitApp();
}

/* Выход из приложения с подтверждением действия */
void MainWindow::ExitApp(){

    QSettings settings("config.ini", QSettings::IniFormat);
    /* проверка, нажимал ли пользователь на "Больше не спрашивать?"
       если да, то выйти из приложения без подтверждения действия,
       если нет, то запросить подтверждение.
    */
    if(settings.value("dontShowExitMSG").toBool() == true)
    {
        QApplication::quit();
    }
    else
    {
        QCheckBox* cb = new QCheckBox("Больше не спрашивать");

        QMessageBox *msgE = new QMessageBox(this);
        msgE->setIcon(QMessageBox::Information);
        msgE->setWindowTitle("Подвердите дейстиве");
        msgE->setText("Вы уверены, что хотите выйти?");
        msgE->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgE->setCheckBox(cb);
        msgE->raise();
        msgE->show();

        if(msgE->exec() == QMessageBox::Yes)
        {
           QApplication::quit();

           if(cb->isChecked() == true)
           {
               settings.setValue("dontShowExitMSG", true);
           }
        }
        else
        {
           msgE->close();

           if(cb->isChecked() == true)
           {
               settings.setValue("dontShowExitMSG", true);
           }
        }
    }

}

/* Сворачивание приложения в трей с подтверждением действия */
void MainWindow::HideApp(){
    QSettings settings("config.ini", QSettings::IniFormat);
    /* проверка, нажимал ли пользователь на "Больше не спрашивать?"
       если да, то свернуть приложение без подтверждения действия,
       если нет, то запросить подтверждение.
    */
    if(settings.value("dontShowHideMSG").toBool() == true)
    {
        hide();
    }
    else
    {
        if(this->isVisible())
        {
            QCheckBox* cb = new QCheckBox("Больше не спрашивать");
            QMessageBox *msgH = new QMessageBox;
            msgH->setIcon(QMessageBox::Information);
            msgH->setWindowTitle("Подвердите действие");
            msgH->setText("Приложение будет свернуто в трей!");
            msgH->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgH->setCheckBox(cb);
            msgH->raise();
            msgH->show();

            if(msgH->exec() == QMessageBox::Yes)
            {
               hide();
               if(cb->isChecked() == true)
               {
                   settings.setValue("dontShowHideMSG", true);
               }
            }
            else
            {
               msgH->close();
               if(cb->isChecked() == true)
               {
                   settings.setValue("dontShowHideMSG", true);
               }
            }
        }
    }
}

void MainWindow::on_radioButton_clicked()
{
    sorting = "time";
    ui->tableWidget->sortItems(0,Qt::DescendingOrder);
    qDebug()<< sorting;
}

void MainWindow::on_radioButton_2_clicked()
{
    sorting = "severitity";
    ui->tableWidget->sortItems(1);
    qDebug()<< sorting;
}

void MainWindow::on_pushButton_clicked()
{
    onTimeout();
}
