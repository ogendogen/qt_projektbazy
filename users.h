#ifndef USERS_H
#define USERS_H

#include <QWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVector>
#include <QPushButton>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QByteArray>
#include <QPoint>
#include <QStack>
#include <QList>
#include <memory>

namespace Ui {
class Users;
}

class Users : public QWidget
{
    Q_OBJECT

public:
    explicit Users(QWidget *parent = 0);
    ~Users();

private slots:
    void addClicked();
    void commitClicked();
    void clearClicked();
    void deletionClicked();
    void cellChanged(int row, int column);

private:
    Ui::Users *ui;
    QTableWidget *table;
    QList <std::shared_ptr<QTableWidgetItem>> table_content;
    QPushButton *commit;
    QPushButton *clear;
    QPushButton *add;
    QPushButton *deletion;

    QStack <int> rows_affected;
    QVector<std::shared_ptr<QCheckBox>> checkboxes; // id = wierszowi
    QVector <int> ids;
    QString getUniqueID();
    void buildTable();
};

#endif // USERS_H
