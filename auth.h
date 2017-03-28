#ifndef AUTH_H
#define AUTH_H

#include <QDialog>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include <QCryptographicHash>
#include <QByteArray>

namespace Ui {
class Auth;
}

class Auth : public QDialog
{
    Q_OBJECT

public:
    explicit Auth(QWidget *parent = 0);
    ~Auth();

signals:
    void sendData(QString login, QStringList permissions);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Auth *ui;
};

#endif // AUTH_H
