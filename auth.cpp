#include "auth.h"
#include "ui_auth.h"

Auth::Auth(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Auth)
{
    ui->setupUi(this);
    extern class QSqlDatabase db;
    if (db.isOpen()) return;
    if (db.driverName().isEmpty()) db = QSqlDatabase::addDatabase("QOCI");
    db.setHostName("155.158.112.45");
    db.setUserName("msbd20");
    db.setPassword("M@rcin12#");
    db.setDatabaseName("oltpstud");
    db.setPort(1521);
}

Auth::~Auth()
{
    delete ui;
}

void Auth::on_pushButton_clicked()
{
    extern class QSqlDatabase db;
    this->setCursor(Qt::WaitCursor);
    if (db.isOpen() || db.open())
    {
        QSqlQuery query(db);
        if (query.exec("SELECT login, password, priviliges FROM accounts"))
        {
            QString form_login = ui->lineEdit->text();
            QString form_password = ui->lineEdit_2->text();

            const char *tab = form_password.toStdString().c_str();
            int len = strlen(tab);
            QCryptographicHash hash(QCryptographicHash::Md5);
            hash.addData(tab,len);
            form_password = QString(hash.result().toHex());
            if (tab) delete tab;

            form_password = "63a9f0ea7bb98050796b649e85481845"; // odkomentować do debuga, bo hasher wtedy różnie działa

            while(query.next())
            {
                QString login = query.value("login").toString();
                QString password = query.value("password").toString();

                if (login == form_login && password == form_password)
                {
                    QString priviliges = query.value("priviliges").toString();
                    QStringList permissions = priviliges.split("|",QString::SkipEmptyParts);

                    emit sendData(login, permissions);

                    this->setCursor(Qt::ArrowCursor);
                    QMessageBox::information(this,"Informacja","Połączono jako "+login, QMessageBox::Ok);

                    this->close();
                    return;
                }
            }
            QMessageBox::critical(this,"Błąd","Nie znaleziono takiego użytkownika lub złe hasło!",QMessageBox::Ok);
            this->setCursor(Qt::ArrowCursor);
        }
        else
        {
            int ret = QMessageBox::critical(this,"Błąd",query.lastError().text(),QMessageBox::Ok, QMessageBox::Retry);
            if (ret == 0) emit on_pushButton_clicked();
        }
    }
    else QMessageBox::critical(this,"Problem z bazą danych",db.lastError().text());
}
