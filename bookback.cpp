#include "bookback.h"
#include "ui_bookback.h"

BookBack::BookBack(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookBack)
{
    ui->setupUi(this);
    this->fillData();
}

// 1 -> ksiazka
// 2 -> czytelnik

void BookBack::fillData()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);

    ui->comboBox->clear();
    ui->comboBox_2->clear();
    ids.clear();
    combobox_readers.clear();
    combobox_books.clear();
    borrowed_books.clear();

    if (query.exec("SELECT id, title, borrowed_to FROM books WHERE borrowed_to IS NOT NULL"))
    {
        while (query.next())
        {
            if (!combobox_books.contains(query.value("title").toString())) // zapobieganie zdublowaniu książek (wbudowana właściwość "duplicatesEnabled" nie zapobiega ręcznego dodaniu przez program)
            {
                ui->comboBox->addItem(query.value("title").toString());
                combobox_books.push_back(query.value("title").toString());
                ids.push_back(query.value("id").toInt());
            }

            QString reader = query.value("borrowed_to").toString();
            if (borrowed_books.contains(reader)) borrowed_books[reader]++; // zlicza książki czytelników - zaoszczędza dodatkowego zapytania do tabeli czytelników (mapa typu <key=QString, value=int>)
            else borrowed_books[reader] = 1;

            if (!combobox_readers.contains(query.value("borrowed_to").toString())) // zapobieganie zdublowaniu czytelników
            {
                ui->comboBox_2->addItem(query.value("borrowed_to").toString());
                combobox_readers.push_back(query.value("borrowed_to").toString());
            }
        }
    }
    else
    {
        QMessageBox::critical(this, "Błąd!", query.lastError().text());
        return;
    }
    if (ui->comboBox->count() == 0) ui->comboBox->addItem("Brak");
}

BookBack::~BookBack()
{
    delete ui;
}

void BookBack::on_pushButton_clicked()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    QString book = ui->comboBox->currentText();
    QString reader = ui->comboBox_2->currentText();

    if (book == "Brak")
    {
        QMessageBox::warning(this, "Uwaga!", "Brak książek do oddania !");
        return;
    }
    int item_index = ui->comboBox->currentIndex();
    int book_id = ids[item_index];
    ids.remove(item_index);

    if (query.exec("UPDATE books SET borrowed_to=null, borrowed_for=null WHERE title='"+book+"' AND borrowed_to='"+reader+"' AND id="+QString::number(book_id)))
    {
        query.clear();
        borrowed_books[reader]--;
        if (query.exec("UPDATE readers SET borrowed_books="+QString::number(borrowed_books[reader])+" WHERE surname='"+reader+"'")) QMessageBox::information(this, "Powodzenie!", "Książka zwrócona !");
        else QMessageBox::critical(this, "Błąd zapytania", query.lastError().text());
    }
    else QMessageBox::critical(this, "Błąd zapytania", query.lastError().text());

    this->fillData();
}

void BookBack::on_comboBox_currentIndexChanged(const QString &arg1) // po wybraniu książki zmieniają się czytelnicy
{
    QString book = arg1;
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT borrowed_to FROM books WHERE title='"+book+"' AND borrowed_to IS NOT NULL"))
    {
        ui->comboBox_2->clear();
        combobox_readers.clear();
        while (query.next())
        {
            if (!combobox_readers.contains(query.value("borrowed_to").toString()))
            {
                ui->comboBox_2->addItem(query.value("borrowed_to").toString());
                combobox_readers.push_back(query.value("borrowed_to").toString());
            }
        }
    }
    else QMessageBox::critical(this, "Błąd zapytania", query.lastError().text());
}
    /*QString book = arg1;
    extern class QSqlDatabase db;
    QSqlQuery query(db);

    if (query.exec("SELECT borrowed_to FROM books WHERE title='"+book+"' AND borrowed_to IS NOT NULL")) // przypadek, gdy 2 różne osoby pożyczyły książkę o tym samym tytule
    {
        ui->comboBox_2->clear();
        ids.clear();
        combobox_readers.clear();
        while (query.next())
        {
            if (!combobox_readers.contains(query.value("borrowed_to").toString()))
            {
            ui->comboBox_2->addItem(query.value("borrowed_to").toString());
            ids.push_back(query.value("id").toInt());
        }
    }
    else QMessageBox::critical(this, "Błąd zapytania", query.lastError().text());*/
