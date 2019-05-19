#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>
#include <QSettings>

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
    /* Слот закрытия приложения */
    void ExitApp();
    /* Слот скрытия приложения в трей */
    void HideApp();

    void on_action_triggered();

    void on_action_3_triggered();

    void on_action_5_triggered();

private:
    Ui::MainWindow *ui;
    // Объявление объекта будущей иконки приложения для трея
    QSystemTrayIcon *trayIcon;

};

#endif // MAINWINDOW_H
