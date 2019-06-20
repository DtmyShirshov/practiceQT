#include "mainwindow.h"
#include "settingwindow.h"
#include <QApplication>
#include <QFile>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /* проверка на наличие файла с настройками.
       если файла нет, то запустить окно настроек,
       а если файл есть, то запустить основное окно.
    */
    if(QFile("config.ini").exists())
    {
        qDebug("Файл есть");
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
    {
        qDebug("Файла нет");
        SettingWindow sw;
        sw.setModal(true);
        sw.setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        sw.show();
        return a.exec();
    }
}
