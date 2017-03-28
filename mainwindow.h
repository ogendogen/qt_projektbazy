#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include <QDebug>
#include <QDesktopWidget>
#include "auth.h"
#include "users.h"
#include "books.h"
#include "lendbook.h"
#include "bookback.h"
#include "readers.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void givePermissions(bool &read, bool &add, bool &edit, bool &deletion);
    ~MainWindow();

public slots:
    void receiveData(QString login, QStringList permissions);

private slots:
    void on_actionInformacje_o_koncie_triggered();

    void on_actionWyloguj_i_wyjd_triggered();

    void on_actionOd_wie_po_czenie_triggered();

    void on_actionWyloguj_triggered();

    void on_actionKonta_triggered();

    void on_actionKsi_ki_3_triggered();

    void on_actionPo_ycz_ksi_ke_triggered();

    void on_actionOddaj_ksi_ke_triggered();

    void on_actionCzytelnicy_triggered();

private:
    // uchwyty do okien
    Ui::MainWindow *ui;
    Users *konta;
    Auth *autoryzacja;
    Books *ksiazki;
    LendBook *pozyczanie;
    BookBack *oddawanie;
    Readers *czytelnicy;

    // dane konta
    bool logged_in;
    QString login;
    bool read;
    bool add;
    bool edit;
    bool deletion;
    bool users;
    bool root;
};

#endif // MAINWINDOW_H
