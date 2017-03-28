#include "books.h"
#include "ui_books.h"

static QSize getQTableSize(QTableWidget *t) {
   int w = t->verticalHeader()->width() + 20; // +4 seems to be needed
   for (int i = 0; i < t->columnCount(); i++)
      w += t->columnWidth(i); // seems to include gridline (on my machine)
   int h = t->horizontalHeader()->height() + 8;
   for (int i = 0; i < t->rowCount(); i++)
      h += t->rowHeight(i);
   return QSize(w, h);
}

Books::Books(QWidget *parent, bool p_read, bool p_add, bool p_edit, bool p_deleting) :
    QWidget(parent),
    ui(new Ui::Books)
{
    ui->setupUi(this);
    this->reading = p_read;
    this->adding = p_add;
    this->editing = p_edit;
    this->deleting = p_deleting;

    //this->setGeometry(0,0,QApplication::desktop()->width(), QApplication::desktop()->height());
    add = nullptr;
    commit = nullptr;
    clear = nullptr;
    deletion = nullptr;
    table = nullptr;
    this->buildTable();
}

/*QString Books::timestampToDate(qint64 timestamp)
{
    QDateTime dt;
    dt.setTime_t(timestamp);
    return dt.toString("yyyy-MM-dd");
}

void Books::dateToTimestamp(QString date)
{
    QDateTime dt = QDateTime::fromString(date, "yyyy-MM-dd");
    return dt.toTime_t();
}*/

void Books::buildTable()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT * FROM books ORDER BY title"))
    {
        if (!table_content.isEmpty()) table_content.clear();
        if (table) delete table;
        table = new QTableWidget(this); // utwórz tabele
        table->setParent(this);
        table->setColumnCount(6);
        table->setRowCount(1);
        table->move(0,0);
        QStringList labels;
        labels<<"ISBN"<<"Tytuł"<<"Autor"<<"Gatunek"<<"Pożyczający"<<"Dzień oddania";
        table->setHorizontalHeaderLabels(labels);
        int row = 0;
        if (!checkboxes.isEmpty()) checkboxes.clear();
        if (!ids.isEmpty()) ids.clear();
        while(query.next())
        {
            table->insertRow(table->rowCount());

            int id = query.value(0).toInt();
            ids.push_back(id);

            QString isbn = query.value(1).toString();
            isbn.insert(3,'-'); // zamiana na właściwą formę isbn
            isbn.insert(5,'-');
            isbn.insert(10,'-');
            isbn.insert(15,'-');
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(isbn)));
            table->setItem(row,0,table_content.constLast().get());

            QString title = query.value(2).toString();
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(title)));
            table->setItem(row,1,table_content.constLast().get());

            QString author = query.value(3).toString();
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(author)));
            table->setItem(row,2,table_content.constLast().get());

            QString genre = query.value(4).toString();
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(genre)));
            table->setItem(row,3,table_content.constLast().get());

            QString borrower = query.value(5).toString();
            if (borrower.isEmpty()) borrower = "";
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(borrower)));
            table->setItem(row,4,table_content.constLast().get());

            QString back_time = query.value(6).toString();
            if (back_time.isEmpty()) back_time = "";
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(back_time)));
            table->setItem(row,5,table_content.constLast().get());

            // budowanie checkbox'ów
            checkboxes.push_back(std::shared_ptr<QCheckBox>(new QCheckBox(this)));
            checkboxes.constLast()->move(getQTableSize(table).width()+25, row*table->rowHeight(row)+table->horizontalHeader()->height());
            checkboxes.constLast()->show();

            row++;
        }

        if (table->rowCount() == 0) table->insertRow(table->rowCount()); // dodawanie wiersza do dodawania

        for (int i=0; i<6; i++) // wypełnianie pustymi itemami ostatniego wiersza
        {
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
            table->setItem(table->rowCount()-1, i, table_content.constLast().get());
        }

        QSize table_size = getQTableSize(this->table);

        table->setGeometry(0,0,table_size.width(), table_size.height());
        table->show();

        connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(cellChanged(int,int)));
        connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellDoubleClicked(int,int)));

        // budowanie przycisków
        if (add) delete add;
        add = new QPushButton("Dodaj", this);
        add->move(0, table_size.height()+25);
        connect(add, SIGNAL(clicked()), this, SLOT(addClicked()));
        if (adding) add->setEnabled(true); else add->setEnabled(false);
        add->show();

        if (commit) delete commit;
        commit = new QPushButton("Zatwierdź edycje", this);
        commit->move(add->x()+add->width()+10, table_size.height()+25);
        connect(commit, SIGNAL(clicked()), this, SLOT(commitClicked()));
        if (editing) commit->setEnabled(true); else commit->setEnabled(false);
        commit->show();

        if (clear) delete clear;
        clear = new QPushButton("Cofnij edycje", this);
        clear->move(commit->x()+commit->width()+10, table_size.height()+25);
        connect(clear, SIGNAL(clicked()), this, SLOT(clearClicked()));
        if (editing) clear->setEnabled(true); else clear->setEnabled(false);
        clear->show();

        if (deletion) delete deletion;
        deletion = new QPushButton("Usuń zaznaczone", this);
        deletion->move(clear->x()+clear->width()+10, table_size.height()+25);
        connect(deletion, SIGNAL(clicked()), this, SLOT(deletionClicked()));
        if (deleting) deletion->setEnabled(true); else deletion->setEnabled(false);
        deletion->show();

        this->update(); // przerysuj cały widget (na wszelki wypadek)
    }
    else QMessageBox::critical(this,"Bład zapytania",query.lastError().text(),QMessageBox::Ok);
}

void Books::addClicked()
{
    int last_row = table->rowCount()-1;

    extern class QSqlDatabase db;
    QSqlQuery query(db);
    QString id = this->getUniqueID(); // symulacja właściwości auto_increment z mysql
    if (id == "select error" || id == "error")
    {
        QMessageBox::critical(this, "Błąd", "Problem z ustaleniem nowego id");
        return;
    }

    QString isbn = table->item(last_row, 0)->text();
    QRegExp regex("\\d{3}-\\d{1}-\\d{4}-\\d{4}-\\d{1}", Qt::CaseSensitive);
    if (!regex.exactMatch(isbn))
    {
        QMessageBox::warning(this, "Uwaga!", "ISBN 13 musi być w odpowiednim formacie !", QMessageBox::Ok);
        return;
    }
    // isbn jest poprawny, usuń myślniki
    isbn.remove('-');

    QString title = table->item(last_row, 1)->text();
    if (title.length() > 64)
    {
        QMessageBox::warning(this, "Uwaga!", "Tytuł może mieć maksymalnie 64 znaki !");
        return;
    }

    QString author = table->item(last_row, 2)->text();
    if (title.length() > 32)
    {
        QMessageBox::warning(this, "Uwaga!", "Autor moze miec maksymalnie 32 znaki !");
        return;
    }

    QString genre = table->item(last_row, 3)->text();
    if (genre.length() > 32)
    {
        QMessageBox::warning(this, "Uwaga!", "Gatunek może mieć maksymalnie 32 znaki !");
        return;
    }

    QString borrower = table->item(last_row,4)->text();
    bool found = false;
    if (!borrower.isEmpty() && query.exec("SELECT surname FROM readers")) // czy czytelnik istnieje ?
    {
        while (query.next())
        {
            if (borrower == query.value("surname").toString()) found = true;
        }
    }
    if (!found && !borrower.isEmpty())
    {
        QMessageBox::critical(this, "Błąd", "Taki czytelnik nie istnieje!");
        return;
    }
    if (borrower.isEmpty()) borrower = "NULL";

    QString borrow_time = table->item(last_row, 5)->text();
    if (!borrow_time.isEmpty())
    {
        if (borrow_time.length() != 10)
        {
            QMessageBox::warning(this, "Błąd", "Data składa się z 10 znaków !");
            return;
        }

        regex.setPattern("\\d{4}-\\d{2}-\\d{2}");
        if (!regex.exactMatch(borrow_time))
        {
            QMessageBox::warning(this, "Błąd", "Data jest w złym formacie!\nPoprawny format: yyyy-mm-dd");
            return;
        }
    }
    else borrow_time = "NULL";

    // dane przefiltrowane, można wysyłać
    query.clear();
    QString s_query = "INSERT INTO books (id, isbn, title, author, genre, borrowed_to, borrowed_for) VALUES('"+id+"','"+isbn+"','"+title+"','"+author+"','"+genre+"',";
    if (borrower == "NULL") s_query.append("null,");
    else s_query.append("'"+borrower+"',");

    if (borrow_time == "NULL") s_query.append("null)");
    else s_query.append("'"+borrow_time+")");

    if (query.exec(s_query))
    {
        QMessageBox::information(this, "Powodzenie!", "Nowa książka poprawnie dodana !");
        for (int i=0; i<6; i++)
        {
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
            table->setItem(table->rowCount(), i, table_content.constLast().get());
        }
        ids.push_back(id.toInt());
    }
    else
    {
        QMessageBox::critical(this, "Błąd zapytania", query.lastError().text());
        return;
    }

    this->buildTable();
}

void Books::commitClicked()
{
    if (rows_affected.isEmpty())
    {
        QMessageBox::information(this, "Brak wierszy", "Nic nie edytowałeś !");
        return;
    }
    int stack_size = rows_affected.size();
    QString s_query = "";
    extern class QSqlDatabase db;
    QSqlQuery query(db);

    QString correct_rows = "";

    for (int i=0; i<stack_size; i++)
    {
        int row = rows_affected.pop();
        if (table->item(row,0) == 0) continue; // rząd już nie istnieje
        s_query = "UPDATE books SET ";
        QString isbn = table->item(row,0)->text();
        QRegExp regex("\\d{3}-\\d{1}-\\d{4}-\\d{4}-\\d{1}", Qt::CaseSensitive);
        if (!regex.exactMatch(isbn)) // czy isbn jest poprawny ?
        {
            QMessageBox::warning(this, "Uwaga!", "ISBN 13 musi być w odpowiednim formacie ! (wiersz "+QString::number(row+1)+")", QMessageBox::Ok);
            continue;
        }

        // usuń myślniki
        isbn.remove('-');

        s_query.append("isbn='"+isbn);
        s_query.append("', title='"+table->item(row,1)->text());
        s_query.append("', author='"+table->item(row,2)->text());
        s_query.append("', genre='"+table->item(row,3)->text());
        QString reader = table->item(row,4)->text();
        bool found = false;
        if (!reader.isEmpty() && query.exec("SELECT surname FROM readers")) // czy czytelnik istnieje ?
        {
            while (query.next())
            {
                if (reader == query.value("surname").toString()) found = true;
            }
        }

        if (!found && !reader.isEmpty())
        {
            QMessageBox::critical(this, "Błąd", "Czytelnik w wierszu "+QString::number(row+1)+" nie istnieje!");
            continue;
        }

        if (reader.isEmpty()) s_query.append("', borrowed_to=null");
        else s_query.append("', borrowed_to='"+reader);

        QString borrow_time = table->item(row,5)->text();
        regex.setPattern("\\d{4}-\\d{2}-\\d{2}");
        if (!borrow_time.isEmpty() && !regex.exactMatch(borrow_time))
        {
            QMessageBox::warning(this, "Błąd", "Data jest w złym formacie!\nPoprawny format: yyyy-mm-dd (wiersz "+QString::number(row+1)+")");
            continue;
        }

        if ((reader.isEmpty() && !borrow_time.isEmpty()) || (!reader.isEmpty() && borrow_time.isEmpty())) // przypadki null + coś lub coś + null
        {
            QMessageBox::warning(this, "Uwaga", "Brakuje czytelnika lub daty !");
            continue;
        }

        if (borrow_time.isEmpty()) s_query.append(", borrowed_for=null");
        else s_query.append("', borrowed_for='"+borrow_time+"'");

        // dane już w zapytaniu, dodanie warunku
        s_query.append(" WHERE id="+QString::number(ids[row]));

        if (query.exec(s_query)) correct_rows.append(QString::number(row+1)+",");
        else QMessageBox::critical(this,"Błąd zapytania",query.lastError().text());
    }

    if (!correct_rows.isEmpty())
    {
        correct_rows.chop(1); // ucięcie znaku
        QMessageBox::information(this, "Powodzenie!", "Poprawnie edytowano wiersze: "+correct_rows);
    }
}

void Books::clearClicked()
{
   rows_affected.clear();
   ids.clear();
   this->buildTable();
}

void Books::deletionClicked()
{
    bool any_checked = false;
    for (int i=0; i<checkboxes.size(); i++)
    {
        if (checkboxes[i]->checkState() == Qt::Checked)
        {
            any_checked = true;
            break;
        }
    }
    if (!any_checked)
    {
        QMessageBox::warning(this,"Uwaga!","Nie zaznaczyłeś żadnego rzędu !");
        return;
    }

    QString s_query = "DELETE FROM books WHERE ";

    bool first_checkbox = true;
    for (int i=0; i<checkboxes.size(); i++)
    {
        if (checkboxes[i]->checkState() == Qt::Checked)
        {
            if (first_checkbox)
            {
                s_query.append("id="+QString::number(ids[i]));
                first_checkbox = false;
                continue;
            }
            s_query.append("OR id="+QString::number(ids[i]));
        }
    }

    extern class QSqlDatabase db;
    QSqlQuery query(db);
    //QMessageBox::information(this, "elo", s_query);
    if (query.exec(s_query))
    {
        QMessageBox::information(this, "Powodzenie!", "Książki poprawnie usunięte !");
        this->buildTable();
    }
    else QMessageBox::critical(this, "Błąd zapytania",query.lastError().text());
}

void Books::cellDoubleClicked(int row, int column)
{
    if (row == -1 || column == -1) return;
    if (column == 4 || column == 5) data2remember = table->item(row, column)->text();
}

void Books::cellChanged(int row, int column)
{
    if (!editing || row == -1 || column == -1 || table->item(row, column) == 0 || table->rowCount()-1 == row) return; // jeśli brak uprawnień, komórka nie istnieje, został edytowany wiersz do dodawania, wiersz jest już na stosie
    if (table->item(row,column)->text() == data2remember) return;
    if ((column == 4 || column == 5) && !table->item(row, column)->text().isEmpty())
    {
        int ret = QMessageBox::warning(this, "Uwaga!", "Zamierzasz edytować pożyczającego lub date!\n Jej ręczna zmiana może spowodować niespójność danych!\n Aby poprawnie wykonać tę operację należy użyć opcji pożycz/oddaj książkę.\n Czy chcesz kontynuować ?", QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            if (!rows_affected.contains(row)) rows_affected.push_back(row);
            return;
        }
        else
        {
            if (data2remember.isEmpty()) return;
            table->item(row, column)->setText(data2remember);
            data2remember = "";
            return;
        }
    }
    if (!rows_affected.contains(row)) rows_affected.push_back(row);
}

/*
void Readers::cellDoubleClicked(int row, int column)
{
    if (row == -1 || column == -1) return;
    if (column == 2) books2remember = table->item(row, column)->text(); // zapamiętuje poprzednią wartość w przypadku, gdy podejmowana jest próba edycji kolumny książek
}

void Readers::cellChanged(int row, int column)
{
    if (!editing || row == -1 || column == -1 || table->item(row, column) == 0 || table->rowCount()-1 == row || rows_affected.contains(row)) return;
    if (column == 2 && table->item(row, column)->text() != "0")
    {
        int ret = QMessageBox::warning(this, "Uwaga!", "Zamierzasz edytować kolumnę liczby książek!\n Jej ręczna zmiana może spowodować niespójność danych!\n Aby poprawnie wykonać tę operację należy użyć opcji pożycz/oddaj książkę.\n Czy chcesz koontynuować ?", QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            rows_affected.push_back(row);
            return;
        }
        else
        {
            if (books2remember.isEmpty()) return;
            table->item(row, column)->setText(books2remember);
            books2remember = "";
            return;
        }
    }
    rows_affected.push_back(row);
}*/

QString Books::getUniqueID()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    int id;
    if (query.exec("SELECT books_id FROM ids"))
    {
        if (query.next()) id = query.value("books_id").toInt();
        else return "select error";
    }
    query.clear();
    if (query.exec("UPDATE ids SET books_id="+QString::number(id+1))) return QString::number(id);
    return "error";
}

Books::~Books()
{
    delete ui;
    table_content.clear();
    delete table;
    delete commit;
    delete add;
    delete clear;
    checkboxes.clear();
    delete deletion;
}
