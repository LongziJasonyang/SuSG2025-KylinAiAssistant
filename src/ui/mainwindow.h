#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include "chatwindow.h"
#include "../core/role/rolemanager.h"
#include "../core/role/customrole.h"
#include "customroledialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void switchToChat();
    void switchToRoleManager();
    void handleRoleSelected(const QString &roleName);
    void updateRoleList();
    void handleNewRole(const RoleData &data);

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QWidget *chatPage;
    QWidget *rolePage;
    ChatWindow *chatWindow;
    RoleManager *roleManager;
    
    void setupUI();
    void setupConnections();
};

#endif // MAINWINDOW_H 