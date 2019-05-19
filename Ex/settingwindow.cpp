#include "settingwindow.h"
#include "ui_settingwindow.h"
#include <QProcess>
#include <QSettings>
#include <QMessageBox>
#include <QFile>

SettingWindow::SettingWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    QSettings settings("config.ini", QSettings::IniFormat);

    if(QFile("config.ini").exists())
    {
        QString log = settings.value("login").toString();
        QString pass = settings.value("password").toString();
        int timerInterval = settings.value("timer").toInt();

        this->ui->log->setText(log);
        this->ui->pass->setText(pass);

        switch (timerInterval)
        {
        case 10:
            this->ui->comboBox->setCurrentText("10");
            break;
        case 15:
            this->ui->comboBox->setCurrentText("15");
            break;
        case 20:
            this->ui->comboBox->setCurrentText("20");
            break;
        case 25:
            this->ui->comboBox->setCurrentText("25");
            break;
        case 30:
            this->ui->comboBox->setCurrentText("30");
            break;
        }
    }

}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::on_pushButton_2_clicked()
{
    this->close();
}
/* сохранение настроек в файл */
void SettingWindow::on_pushButton_clicked()
{
    QString log = ui->log->text();
    QString pass = ui->pass->text();
    if(log != "" && pass != "")
    {
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.setValue("login", log);
        settings.setValue("password", pass);
        switch (ui->comboBox->currentIndex())
        {
        case 0:
            settings.setValue("timer", 10);
            break;
        case 1:
            settings.setValue("timer", 15);
            break;
        case 2:
            settings.setValue("timer", 20);
            break;
        case 3:
            settings.setValue("timer", 25);
            break;
        case 4:
            settings.setValue("timer", 30);
            break;
        default:
            break;
        }

        QMessageBox msg;
        msg.setWindowTitle("Подтвердите действие");
        msg.setText("Приложение будет перезапущено");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msg.raise();
        msg.open();
        if(msg.exec() == QMessageBox::Yes)
        {
            qDebug("Сохранил настройки");
            //перезапуск приложения!
            qApp->quit();
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        }
        else
        {
           msg.close();
        }
    }
    else
    {

    }
}
