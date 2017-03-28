#ifndef BOOKS_H
#define BOOKS_H

#include <QWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QPushButton>
#include <QCheckBox>
#include <QStack>
#include <QRegExp>
#include <QApplication>
#include <memory>

namespace Ui {
class Books;
}

class Books : public QWidget
{
    Q_OBJECT

public:
    explicit Books(QWidget *parent = 0, bool p_read=false, bool p_add=false, bool p_edit=false, bool p_deleting=false);
    ~Books();

private slots:
    void addClicked();
    void commitClicked();
    void clearClicked();
    void deletionClicked();
    void cellDoubleClicked(int row, int column);
    void cellChanged(int row,int column);

private:
    Ui::Books *ui;
    // uprawnienia
    bool reading;
    bool adding;
    bool editing;
    bool deleting;

    // komponenty wizualne
    QTableWidget *table;
    QList<std::shared_ptr<QTableWidgetItem>> table_content;
    QVector<std::shared_ptr<QCheckBox>> checkboxes; // id = wierszowi
    QPushButton *add;
    QPushButton *commit;
    QPushButton *clear;
    QPushButton *deletion;
    void buildTable();

    // inne
    QString getUniqueID(); // pobierz unikalne id
    QStack <int> rows_affected; // wiersze, które zostały zmienione (do edycji)
    QVector <int> ids; // id wierszy w bazie
    QString data2remember; // dane do zapamiętania, których nie powinno się ręcznie edytować
    //QString timestampToDate(qint64 timestamp);
    //qint64 dateToTimestamp(QString date);
};

#endif // BOOKS_H
