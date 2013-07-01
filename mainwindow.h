#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//moved from initial mainwindow.cpp due to strange linux compile errors
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QProcess>
#include "s7connection.h"
#include "connectionsettings.h"
#include "mainwindow.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    S7Connection MyS7Connection;
    void customMessageHandler();
    
private slots:
    void on_actionNeues_Projekt_triggered();
    void on_actionProjekt_ffnen_triggered();
    void on_Button_Connect_clicked();
    void on_Button_Get_Val_clicked();
    void TimeOut();
    void ChangeSettings(ConSets NewConSets);
    void on_pushButton_ConSets_clicked();
    void on_Button_read_slots_clicked();

private:
    Ui::MainWindow *ui;
    ConnectionSettings ConDiag;
};

#endif // MAINWINDOW_H
