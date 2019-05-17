#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent * event);

private slots:
    void on_pushButton_clicked();
    /* Слот, который будет принимать сигнал от события нажатия на иконку приложения в трее */
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    /* Слот на таймаут таймера */
    void onTimeout();
    /* Слот на нажатие кнопки выход в меню трея */
    void ExitApp();

private:
    Ui::MainWindow *ui;
    // Объявление объекта будущей иконки приложения для трея
    QSystemTrayIcon *trayIcon;

};

#endif // MAINWINDOW_H
