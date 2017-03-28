#ifndef LENDBOOK_H
#define LENDBOOK_H

#include <QDialog>
#include <QMessageBox>
#include <QVector>
#include <QMap>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

namespace Ui {
class LendBook;
}

class LendBook : public QDialog
{
    Q_OBJECT

public:
    explicit LendBook(QWidget *parent = 0);
    void checkAccess(bool access);
    ~LendBook();

private slots:
    void on_calendarWidget_clicked(const QDate &date);

    void on_dateEdit_dateChanged(const QDate &date);

    void on_pushButton_clicked();

private:
    Ui::LendBook *ui;
    bool access;
    void fillData();
    QVector <int> ids;
    QMap <QString, int> borrowed_books;
};

#endif // LENDBOOK_H
