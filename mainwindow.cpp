#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingwindow.h"
#include "json.h"
#include <QtWidgets>
#include <QTimer>
#include <QApplication>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QApplication::setQuitOnLastWindowClosed(false);
    /* Забираем заданный в настройках интервал таймера из файла config.ini */
    QSettings settings("config.ini", QSettings::IniFormat);

    /* запоминаем время запуска приложения */
    launchDateTime = (int)QDateTime::currentDateTime().toTime_t();

    int timerInterval = settings.value("timer").toInt() * 1000;

    jsn.Authorization(settings.value("IP").toString(), settings.value("login").toString(), settings.value("password").toString());

    ui->setupUi(this);
    this->setWindowTitle("ZabbixDesktopClient");
    this->setWindowIcon(QIcon(":/new/ZabbixIco.ico"));

    /* Задаем иконку и подсказку при наведении на неё */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/new/ZabbixIco.ico"));
    trayIcon->setToolTip("ZabbixDesktopClient");

    /* Создаем контекстное меню */
    QMenu* menu = new QMenu(this);

    QAction* hideWindow = new QAction("Свернуть окно", this);
    QAction* viewWindow = new QAction("Развернуть окно", this);
    QAction* closeMsg = new QAction("Закрыть все оповещения", this);
    QAction* quitAction = new QAction("Выход", this);

    /* подключаем сигналы нажатий на пункты меню к соответсвующим слотам.
       Первый пункт меню разворачивает приложение из трея,
       второй пункт сворачивает приложение в трей,
       третий пункт меню завершает приложение.
    */
    connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(HideApp()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(ExitApp()));
    connect(closeMsg, SIGNAL(triggered()), this, SLOT(CloseAllMsg()));

    /* Добавляем кнопки в контекстное меню */
    menu->addAction(viewWindow);
    menu->addAction(hideWindow);
    menu->addAction(closeMsg);
    menu->addAction(quitAction);

    /* Задаем контекстное меню кнопке в трее и показываем её */
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    /* прячем 1 и 4 колонку */
    ui->tableWidget->setColumnHidden(1, true);
    ui->tableWidget->setColumnHidden(4, true);
    /* задаем ширину стобцов и из заголовки, запрещаем изменять ширину */
    QStringList name_table;
    name_table << "Дата/время" << "" << "Узел сети" << "Проблема";
    ui->tableWidget->setHorizontalHeaderLabels(name_table);
    ui->tableWidget->setColumnWidth(0, 170);
    ui->tableWidget->setColumnWidth(2, 320);
    ui->tableWidget->setColumnWidth(3, 820);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    sorting = "time";

    sound = new QSound(":\\new\\alarm_02.wav");

    /* Объявляем таймер, задаем ему интервал, подключаем сигнал таймаута к слоту*/
    QTimer* timerGetProblems = new QTimer(this);
    timerGetProblems->start(timerInterval);
    connect(timerGetProblems, SIGNAL(timeout()), this, SLOT(GetProblems()));

    QTimer* timerDeleteProblems = new QTimer(this);
    timerDeleteProblems->start(timerInterval * 2);
    connect(timerDeleteProblems, SIGNAL(timeout()), this, SLOT(DeleteResolvedProblems()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GetProblems()
{
    QSettings settings("config.ini", QSettings::IniFormat);

    QJsonArray problemsIDs(jsn.GetProblemsIDs(launchDateTime)); //массив ID полученных проблем
    QJsonArray problemIDsOK;

    if(!problemsIDs.isEmpty())
    {
        qDebug() << problemsIDs;
        for(int i = 0; i < problemsIDs.count(); ++i)
        {
            if(!alreadyExists.contains(problemsIDs.at(i)))
            {
                problemIDsOK.append(QJsonValue(problemsIDs.at(i)));
            }
        }
    }

    QJsonArray problems;
    while(problems.count() != problemIDsOK.count())
    {
        problems = jsn.GetProblemsAlerts(problemIDsOK);
    }
    if(problems.count() == problemIDsOK.count())
    {
        for(auto i = problems.begin(); i != problems.end(); ++i)
        {
            QJsonObject jO = i->toObject();
            SetItem(jO["message"].toString());
        }
    }
    ui->label_2->setText("Проблем показано: " + QString::number(ui->tableWidget->rowCount()));
}

void MainWindow::SetItem(QString problemMessage)
{
    /* парсим текст проблемы, удаляем лишнее */
    problemMessage.chop(1);
    QStringList problemMessagelist = problemMessage.split("\r\n");

    QString host = problemMessagelist.at(0);
    host.remove("HOST: ");

    QString problem = problemMessagelist.at(1);
    problem.remove("TRIGGER_NAME: ");

    QString severitity = problemMessagelist.at(3);
    severitity.remove("TRIGGER_SEVERITY: ");

    QString event_id = problemMessagelist.at(9);
    event_id.remove("EVENT_ID: ");

    QList<QTableWidgetItem*> item = ui->tableWidget->findItems(event_id, Qt::MatchExactly);
    if(!item.isEmpty())
    {
        alreadyExists.append(QJsonValue(event_id));
    }
    else
    {
        /* подготавливаем айтемы */
        QTableWidgetItem* itemSeveritity = new QTableWidgetItem();

        QTableWidgetItem* itemProblem = new QTableWidgetItem();
        itemProblem->setText(problem);

        /* устанавливем цвет фона в соответствии с важностью проблемы */
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

        /* вывод проблемы в таблицу */
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, itemHost);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, itemProblem);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, itemSeveritity);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, itemDateTime);
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 4, itemEvent_id);

        /* сортировка записей по времени в соответствии с настроеным фильтром */
        if(sorting == "time")
        {
            ui->tableWidget->sortItems(0, Qt::DescendingOrder);
        }
        if(sorting == "severitity")
        {
            ui->tableWidget->sortItems(0, Qt::DescendingOrder);
            ui->tableWidget->sortItems(1);
        }

        sound->play();

        int closeTimeOut;

        switch (ui->comboBox->currentIndex())
        {
        case 0:
            closeTimeOut = 10 * 60000;
            break;
        case 1:
            closeTimeOut = 30 * 60000;
            break;
        case 2:
            closeTimeOut = 60 * 60000;
            break;
        case 3:
            closeTimeOut = 1 * 60000;
            break;
        default:
            closeTimeOut = 30 * 60000;
            break;
        }

        msg = new QMessageBox(this);
        msg->setAttribute(Qt::WA_DeleteOnClose, true);
        msg->setStandardButtons(QMessageBox::Ok);
        msg->button(QMessageBox::Ok)->animateClick(closeTimeOut);
        msg->setWindowFlag(Qt::WindowStaysOnTopHint);

        /* вывод оповещения в соответствии с настроеным фильтром */
        if(ui->checkBox->isChecked() == true && severitity == "Информация")
        {
            msg->setWindowTitle(severitity + "!");
            msg->setText(dateTime + "\n" + host + "\n" + problem);
        }
        if(ui->checkBox_2->isChecked() == true && severitity == "Предупреждение")
        {
            msg->setWindowTitle(severitity + "!");
            msg->setText(dateTime + "\n" + host + "\n" + problem);
        }
        if(ui->checkBox_3->isChecked() == true && severitity == "Средняя")
        {
            msg->setWindowTitle(severitity + "!");
            msg->setText(dateTime + "\n" + host + "\n" + problem);
        }
        if(ui->checkBox_4->isChecked() == true && severitity == "Высокая")
        {
            msg->setWindowTitle(severitity + "!");
            msg->setText(dateTime + "\n" + host + "\n" + problem);
        }
        if(ui->checkBox_5->isChecked() == true && severitity == "Чрезвычайная")
        {
            msg->setWindowTitle(severitity + "!");
            msg->setText(dateTime + "\n" + host + "\n" + problem);
        }
        msglist.append(msg);
        msg->show();
    }
}

void MainWindow::DeleteResolvedProblems()
{
    if(!alreadyExists.isEmpty())
    {
        QJsonArray problemsIDs(jsn.GetProblemsIDs(launchDateTime));
        qDebug() << "ПроблемИД для проверки на удаление";
        qDebug() << problemsIDs;
        for(int i = 0; i < alreadyExists.count(); ++i)
        {
            if(!problemsIDs.contains(alreadyExists.at(i)))
            {
                qDebug() << "Я нашел проблему на удаление! " + alreadyExists.at(i).toString();
                QList<QTableWidgetItem*> item = ui->tableWidget->findItems(alreadyExists.at(i).toString(), Qt::MatchExactly);

                if(!item.empty())
                {
                    for(int j = 0; j < item.count(); j++)
                    {
                     int row = item.at(j)->row();
                     ui->tableWidget->removeRow(row);

                     qDebug() << "Удалил решенную проблему с ID: " + alreadyExists.at(i).toString();
                    }
                    alreadyExists.removeAt(i);
                }
            }
        }
    }
    ui->label_2->setText("Проблем показано: " + QString::number(ui->tableWidget->rowCount()));
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
    sw.setModal(true);
    sw.setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
void MainWindow::ExitApp()
{

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
        msgE->setWindowTitle("Подвердите дейстиве");
        msgE->setText("Вы уверены, что хотите выйти?");
        msgE->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgE->setButtonText(QMessageBox::Yes, tr("Да"));
        msgE->setButtonText(QMessageBox::No, tr("Нет"));
        msgE->setCheckBox(cb);
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

void MainWindow::CloseAllMsg()
{
    for(int i = 0; i < msglist.count(); ++i)
    {
        msglist.at(i)->close();
    }
    msglist.clear();
}

/* Сворачивание приложения в трей с подтверждением действия */
void MainWindow::HideApp()
{
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

            QMessageBox *msgH = new QMessageBox(this);
            msgH->setWindowTitle("Подвердите действие");
            msgH->setText("Приложение будет свернуто в трей!");
            msgH->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgH->setButtonText(QMessageBox::Yes, tr("Да"));
            msgH->setButtonText(QMessageBox::No, tr("Нет"));
            msgH->setCheckBox(cb);
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

