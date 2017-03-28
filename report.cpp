#include "report.h"
#include "ui_report.h"

Report::Report(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Report)
{
    ui->setupUi(this);
    this->dialog_parent = parent;
    this->buildList();
}

void Report::buildList()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT title, borrowed_to, borrowed_for FROM books WHERE borrowed_to IS NOT NULL"))
    {
        while(query.next())
        {
            if (is_user_late(query.value("borrowed_for").toString())) list.addItem(query.value("title").toString()+" nie oddane przez "+query.value("borrowed_to").toString());
        }
    }
    list.setParent(this);
    list.setGeometry(this->geometry());
    if (list.item(0) == 0) QMessageBox::information(dialog_parent, "Uwaga!", "Brak zalegających książek !");
}

bool Report::is_user_late(QString s_user_date)
{
    s_user_date.replace('-','.');
    QDateTime user_date = QDateTime::fromString(s_user_date,"dd.MM.yyyy");
    qint64 current_unix = QDateTime::currentMSecsSinceEpoch() / 1000;
    qint64 user_unix = user_date.toMSecsSinceEpoch() / 1000;

    QMessageBox::information(dialog_parent, "elo", QString::number(current_unix)+" "+QString::number(user_unix));

    if (user_unix < current_unix) return true;
    return false;
}

Report::~Report()
{
    delete ui;
}
