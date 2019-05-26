#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingwindow.h"
#include <QtWidgets>
#include <QTimer>
#include <QMessageBox>
#include <QtMultimedia/QSound>
#include <QApplication>
#include <QSettings>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /* Забираем заданный в настройках интервал таймера из файла config.ini */
    QSettings settings("config.ini", QSettings::IniFormat);
    int timerInterval = settings.value("timer").toInt() * 1000;

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

/* Действие на таймаут таймера */
void MainWindow::onTimeout(){
   QSound* sound=new QSound(":\\new\\alarm_02.wav");
   sound->play();
   QMessageBox* msg = new QMessageBox(this);
   msg->setWindowTitle("Действие");
   msg->setText("Внимание");
   msg->setInformativeText("Прошло секунд!");
   msg->raise();
   msg->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}
/* отобразить логин и пароль из файла */
void MainWindow::on_pushButton_clicked()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QString log = settings.value("login").toString();
    QString pass = settings.value("password").toString();
    ui->label->setText(log + "; " + pass);
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

void MainWindow::on_action_3_triggered()
{
    ExitApp();
}

void MainWindow::on_action_triggered()
{
    HideApp();
}

void MainWindow::on_action_5_triggered()
{
    SettingWindow sw;
    sw.setModal(true);
    sw.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

    QTableWidgetItem* item1 = new QTableWidgetItem();
    item1->setText("3");

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setText("Зеленый");
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::green);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, item);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, item1);

    ui->tableWidget->sortItems(1);

}

void MainWindow::on_pushButton_3_clicked()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

    QTableWidgetItem* item1 = new QTableWidgetItem();
    item1->setText("2");

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setText("Синий");
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::blue);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, item);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, item1);

    ui->tableWidget->sortItems(1);

}

void MainWindow::on_pushButton_4_clicked()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

    QTableWidgetItem* item1 = new QTableWidgetItem();
    item1->setText("1");

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setText("Красный");
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::red);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, item);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, item1);

    ui->tableWidget->sortItems(1);

}
