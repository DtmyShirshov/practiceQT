#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QTimer>
#include <QMessageBox>
#include <QtMultimedia/QSound>
#include <QApplication>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("My First Tray Programm");

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
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(hide()));
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
    timer->start(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

}


void MainWindow::ExitApp(){

    QMessageBox *msgE = new QMessageBox;
    msgE->setIcon(QMessageBox::Information);
    msgE->setWindowTitle("Вы уверены?");
    msgE->setText("Вы уверены, что хотите выйти?");
    msgE->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgE->open();

    if(msgE->exec() == QMessageBox::Yes)
    {
       QApplication::quit();
    }
    else
    {
       msgE->close();
    }
}

void MainWindow::onTimeout(){

   QSound* sound=new QSound(":\\new\\alarm_02.wav");
   sound->play();

   QMessageBox* msg = new QMessageBox(this);
   msg->setWindowTitle("Действие");
   msg->setText("Внимание");
   msg->setInformativeText("Прошло 15 секунд!");
//   msg->raise();
   msg->show();

   if(msg->exec() == QMessageBox::Ok && !this->isVisible())
   {
       this->show();
       this->hide();
   }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->label->setText("Приветик");
}

/* Метод, который обрабатывает событие закрытия окна приложения */
void MainWindow::closeEvent(QCloseEvent * event)
{
    /* Если окно видимо, то завершение приложения игнорируется, а окно просто скрывается */
    if(this->isVisible()){
        event->ignore();
        this->hide();
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
            this->hide();
        }

        break;
    default:
        break;
    }
}
