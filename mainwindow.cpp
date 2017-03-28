#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    logged_in = false;
    konta = nullptr;
    ksiazki = nullptr;
    pozyczanie = nullptr;
    oddawanie = nullptr;
    czytelnicy = nullptr;
    autoryzacja = new Auth();
    autoryzacja->setModal(true);
    autoryzacja->setFocus();
    connect(autoryzacja, SIGNAL(sendData(QString,QStringList)), this, SLOT(receiveData(QString,QStringList))); // wysłanie informacji o loginie i uprawnieniach do głównego okna
    autoryzacja->exec();
}

MainWindow::~MainWindow()
{
    delete autoryzacja;
    delete konta;
    delete ksiazki;
    delete ui;
}

void MainWindow::receiveData(QString login, QStringList permissions)
{
    this->login = login;
    if (permissions[0] == "1") this->read = true; else this->read = false;
    if (permissions[1] == "1") this->add = true; else this->add = false;
    if (permissions[2] == "1") this->edit = true; else this->edit = false;
    if (permissions[3] == "1") this->deletion = true; else this->deletion = false;
    if (permissions[4] == "1") this->users = true; else this->users = false;

    if (permissions[5] == "1")
    {
        this->root = true;
        this->read = true;
        this->users = true;
        this->edit = true;
        this->deletion = true;
        this->add = true;
    }
    else this->root = false;
    logged_in = true;
    ui->actionWyloguj->setText("Wyloguj");
}

void MainWindow::on_actionInformacje_o_koncie_triggered()
{
    if (!logged_in)
    {
        QMessageBox::information(this, "Uwaga!", "Jesteś niezalogowany!", QMessageBox::Ok);
        return;
    }

    QString info = "Login: ";
    info += this->login+"\n";
    if (read) info += "Odczyt: TAK\n"; else info +="Odczyt: NIE\n";
    if (add) info += "Zapis: TAK\n"; else info +="Zapis: NIE\n";
    if (edit) info += "Edycja: TAK\n"; else info +="Edycja: NIE\n";
    if (deletion) info += "Usuwanie: TAK\n"; else info +="Usuwanie: NIE\n";
    if (users) info += "Konta: TAK\n"; else info +="Konta: NIE\n";
    if (root) info += "ROOT: TAK\n"; else info +="ROOT: NIE\n";
    QMessageBox::information(this, "Informacje o koncie", info, QMessageBox::Ok);
}

void MainWindow::on_actionWyloguj_i_wyjd_triggered()
{
    extern class QSqlDatabase db;
    db.close();
    this->close();
}

void MainWindow::on_actionOd_wie_po_czenie_triggered()
{
    if (!logged_in)
    {
        QMessageBox::critical(this, "Błąd!", "Nie jesteś zalogowany !");
        return;
    }
    extern class QSqlDatabase db;
    if (db.isOpen()) QMessageBox::information(this, "Uwaga!", "Połączenie jest nadal aktywne!", QMessageBox::Ok);
    else if (db.open()) QMessageBox::information(this, "Uwaga!", "Połączenie zostało odnowione!", QMessageBox::Ok);
    else QMessageBox::critical(this, "Uwaga!", "Problem z odnowieniem połączenia!", QMessageBox::Ok);
}

void MainWindow::on_actionWyloguj_triggered()
{
    read = false;
    add = false;
    edit = false;
    deletion = false;
    users = false;
    root = false;
    login = "";
    logged_in = false;
    ui->actionWyloguj->setText("Zaloguj");
    if (autoryzacja != nullptr) delete autoryzacja;
    autoryzacja = nullptr;

    if (konta != nullptr) delete konta;
    konta = nullptr;
    if (ksiazki != nullptr) delete ksiazki;
    ksiazki = nullptr;
    if (czytelnicy != nullptr) delete czytelnicy;
    czytelnicy = nullptr;
    this->setCentralWidget(nullptr);

    autoryzacja = new Auth();
    autoryzacja->setModal(true);
    autoryzacja->setFocus();
    connect(autoryzacja, SIGNAL(sendData(QString,QStringList)), this, SLOT(receiveData(QString,QStringList)));
    autoryzacja->exec();
}

void MainWindow::on_actionKonta_triggered()
{
    if (!users)
    {
        QMessageBox::critical(this, "Błąd!", "Nie masz uprawnień do tego !");
        return;
    }
    if (konta == nullptr) konta = new Users(this);
    if (ksiazki)
    {
        delete ksiazki;
        ksiazki = nullptr;
    }
    if (czytelnicy)
    {
        delete czytelnicy;
        czytelnicy = nullptr;
    }
    this->showMaximized();
    this->setCentralWidget(konta);
}

void MainWindow::on_actionKsi_ki_3_triggered()
{
    if (!read)
    {
        QMessageBox::critical(this, "Brak dostępu !","Nie masz uprawnień aby to przeglądać !");
        return;
    }
    if (ksiazki == nullptr) ksiazki = new Books(this, read, add, edit, deletion);
    if (konta)
    {
        delete konta;
        konta = nullptr;
    }
    if (czytelnicy)
    {
        delete czytelnicy;
        czytelnicy = nullptr;
    }

    this->showMaximized();
    this->setCentralWidget(ksiazki);
}

void MainWindow::on_actionCzytelnicy_triggered()
{
    if (!read)
    {
        QMessageBox::critical(this, "Błąd!", "Nie masz uprawnień do tego !");
        return;
    }
    if (ksiazki)
    {
        delete ksiazki;
        ksiazki = nullptr;
    }
    if (konta)
    {
        delete konta;
        konta = nullptr;
    }
    if (czytelnicy == nullptr) czytelnicy = new Readers(this, read, add, edit, deletion);

    this->setCentralWidget(czytelnicy);
    this->showMaximized();
}

void MainWindow::on_actionPo_ycz_ksi_ke_triggered() // pożycz książkę
{
    if (!edit)
    {
        QMessageBox::critical(this, "Błąd!", "Nie masz do tego uprawnień! (edycja)");
        return;
    }

    if (pozyczanie == nullptr)
    {
        pozyczanie = new LendBook(this);
        pozyczanie->setModal(true);
        pozyczanie->setFocus();
        pozyczanie->exec();
    }

    if (pozyczanie)
    {
        delete pozyczanie;
        pozyczanie = nullptr;
    }
}

void MainWindow::on_actionOddaj_ksi_ke_triggered()
{
    if (!edit)
    {
        QMessageBox::critical(this, "Błąd!", "Nie masz do tego uprawnień! (edycja)");
        return;
    }

    if (oddawanie == nullptr)
    {
        oddawanie = new BookBack(this);
        oddawanie->setModal(true);
        oddawanie->setFocus();
        oddawanie->exec();
    }

    if (oddawanie)
    {
        delete oddawanie;
        oddawanie = nullptr;
    }
}
