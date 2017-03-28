#ifndef BOOKBACK_H
#define BOOKBACK_H

#include <QDialog>
#include <QMessageBox>
#include <QVector>
#include <QList>
#include <QMap>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

namespace Ui {
class BookBack;
}

class BookBack : public QDialog
{
    Q_OBJECT

public:
    explicit BookBack(QWidget *parent = 0);
    ~BookBack();

private slots:
    void on_pushButton_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::BookBack *ui;
    void fillData();
    QVector <int> ids;
    QList <QString> combobox_readers;
    QList <QString> combobox_books;
    QMap <QString, int> borrowed_books;
};

#endif // BOOKBACK_H
