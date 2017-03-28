#include "lendbook.h"
#include "ui_lendbook.h"

LendBook::LendBook(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LendBook)
{
    ui->setupUi(this);
    this->fillData();
}

void LendBook::fillData()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    ids.clear();
    ui->comboBox->clear();
    ui->comboBox_2->clear();
    borrowed_books.clear();
    if (query.exec("SELECT name, surname, borrowed_books FROM readers"))
    {
        while (query.next())
        {
            QString name = query.value("name").toString();
            QString surname = query.value("surname").toString();
            ui->comboBox->addItem(name+" "+surname);
            borrowed_books[surname]=query.value("borrowed_books").toInt();
        }
    }
    else
    {
        QMessageBox::critical(this, "Błąd zapytania!", query.lastError().text());
        return;
    }

    query.clear();
    if (query.exec("SELECT id,title FROM books WHERE borrowed_to IS NULL"))
    {
        while (query.next())
        {
            ui->comboBox_2->addItem(query.value("title").toString());
            ids.push_back(query.value("id").toInt());
        }
    }
    else
    {
        QMessageBox::critical(this, "Błąd zapytania!", query.lastError().text());
        return;
    }
    if (ui->comboBox_2->currentIndex() == -1) ui->comboBox_2->addItem("Brak książek");

    QDate current_date = QDate::currentDate();
    ui->dateEdit->setMinimumDate(current_date);
    ui->dateEdit->setDate(current_date);
}

LendBook::~LendBook()
{
    delete ui;
}

void LendBook::on_calendarWidget_clicked(const QDate &date)
{
    ui->dateEdit->setDate(date);
}

void LendBook::on_dateEdit_dateChanged(const QDate &date)
{
    if (!date.isValid())
    {
        QMessageBox::warning(this, "Uwaga!", "Taka data nie istnieje !");
        ui->pushButton->setEnabled(false);
        return;
    }

    QDate current_date = QDate::currentDate();
    if (date.year() != current_date.year() && date.year()+1 != current_date.year())
    {
        QMessageBox::critical(this, "Uwaga!", "Możesz pożyczyć tylko na aktualny i następny rok !");
        ui->pushButton->setEnabled(false);
        return;
    }

    ui->calendarWidget->setSelectedDate(date);
    ui->pushButton->setEnabled(true);
}

void LendBook::on_pushButton_clicked()
{
    QString reader = ui->comboBox->currentText();
    QString book = ui->comboBox_2->currentText();
    QString borrow_date = ui->dateEdit->text();
    QStringList reader_data = reader.split(" ");

    if (borrowed_books.value(reader_data[1]) == 3)
    {
        QMessageBox::warning(this, "Uwaga!", "Ten czytelnik pożyczył już 3 książki !");
        return;
    }
    if (reader.isEmpty())
    {
        QMessageBox::warning(this, "Uwaga!", "Nie wybrałeś żadnego czytelnika!");
        return;
    }
    if (book.isEmpty() || book == "Brak książek")
    {
        QMessageBox::warning(this, "Uwaga!", "Nie wybrałeś żadnej książki!");
        return;
    }
    int item_index = ui->comboBox_2->currentIndex();
    int book_id = ids[item_index];
    ids.remove(item_index);

    extern class QSqlDatabase db;
    QSqlQuery query(db);
    borrowed_books[reader_data[1]]++;
    if (query.exec("UPDATE books SET borrowed_to='"+reader_data[1]+"', borrowed_for='"+borrow_date+"' WHERE title='"+book+"' AND id="+QString::number(book_id)))
    {
        query.clear();
        if (query.exec("UPDATE readers SET borrowed_books=borrowed_books+1 WHERE surname='"+reader_data[1]+"'"))
        {
            QMessageBox::information(this, "Powodzenie", "Książka pożyczona !");
            this->fillData();
        }
        else QMessageBox::critical(this, "Błąd zapytania!", query.lastError().text());
    }
    else QMessageBox::critical(this, "Błąd zapytania!", query.lastError().text());
}
