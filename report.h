#ifndef REPORT_H
#define REPORT_H

#include <QDialog>
#include <QListWidget>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QDateTime>
#include <QStringList>

namespace Ui {
class Report;
}

class Report : public QDialog
{
    Q_OBJECT

public:
    explicit Report(QWidget *parent = 0);
    ~Report();

private:
    Ui::Report *ui;
    QWidget *dialog_parent;
    QListWidget list;
    qint64 current_unix_time;

    bool is_user_late(QString s_date_time);
    void buildList();
};

#endif // REPORT_H
