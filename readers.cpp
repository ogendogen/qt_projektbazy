#include "readers.h"
#include "ui_readers.h"

Readers::Readers(QWidget *parent, bool p_read, bool p_add, bool p_edit, bool p_deleting) :
    QWidget(parent),
    ui(new Ui::Readers)
{
    ui->setupUi(this);
    table = nullptr;
    add = nullptr;
    commit = nullptr;
    clear = nullptr;
    deletion = nullptr;

    this->reading = p_read;
    this->adding = p_add;
    this->editing = p_edit;
    this->deleting = p_deleting;
    this->buildTable();
}

QSize Readers::getQTableSize(QTableWidget *t) {
   int w = t->verticalHeader()->width() + 20; // +4 seems to be needed
   for (int i = 0; i < t->columnCount(); i++)
      w += t->columnWidth(i); // seems to include gridline (on my machine)
   int h = t->horizontalHeader()->height() + 8;
   for (int i = 0; i < t->rowCount(); i++)
      h += t->rowHeight(i);
   return QSize(w, h);
}

QString Readers::getUniqueID()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    int id;
    if (query.exec("SELECT readers_id FROM ids"))
    {
        if (query.next()) id = query.value("readers_id").toInt();
        else return "select error";
    }
    query.clear();
    if (query.exec("UPDATE ids SET readers_id=readers_id+1")) return QString::number(id);
    return "error";
}

void Readers::buildTable()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT id, name, surname, borrowed_books FROM readers ORDER BY surname ASC")) // alfabetycznie, po nazwiskach
    {
        if (!table_content.isEmpty()) table_content.clear();
        if (table) delete table;
        table = new QTableWidget(this); // utwórz tabele
        table->setParent(this);
        table->setColumnCount(3);
        table->setRowCount(1);
        table->move(0,0);
        QStringList labels;
        labels<<"Imię"<<"Nazwisko"<<"Ilość książek";
        table->setHorizontalHeaderLabels(labels);
        int row = 0;

        if (!checkboxes.isEmpty()) checkboxes.clear();
        if (!ids.isEmpty()) ids.clear();
        while(query.next())
        {
            table->insertRow(table->rowCount());

            int id = query.value("id").toInt();
            ids.push_back(id);
            for (int i=1; i<4; i++)
            {
                QString value = query.value(i).toString();
                table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(value)));
                table->setItem(row, i-1, table_content.constLast().get());
            }

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
    }
    else QMessageBox::critical(this, "Błąd zapytania !", query.lastError().text());
}

void Readers::addClicked()
{
    QString name = table->item(table->rowCount()-1, 0)->text();
    QString surname = table->item(table->rowCount()-1, 1)->text();
    QString books = table->item(table->rowCount()-1, 2)->text();
    if (books.isEmpty())
    {
        books = "0";
        table->item(table->rowCount()-1, 2)->setText("0");
    }
    if (books.toInt() < 0)
    {
        QMessageBox::critical(this, "Błąd!", "Ilość pożyczonych książek nie może być ujemna !");
        return;
    }
    if (books.toInt() > 3)
    {
        QMessageBox::critical(this, "Błąd!", "Czytelnik może pożyczyć maksymalnie 3 książki !");
        return;
    }
    if (books != "0")
    {
        int ret = QMessageBox::warning(this, "Uwaga!", "Zalecane jest nadanie ilości książek równej zero. Czy kontynuować ?", QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }

    // sprawdź unikalność nazwiska
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT surname FROM readers"))
    {
        while (query.next())
        {
            if (query.value("surname").toString() == surname)
            {
                QMessageBox::critical(this, "Błąd!", "Takie nazwisko już istnieje !");
                return;
            }
        }
    }

    QString id = this->getUniqueID();

    query.clear();
    if (query.exec("INSERT INTO readers (id, name, surname, borrowed_books) VALUES ("+id+", '"+name+"', '"+surname+"', '"+books+"')"))
    {
        table->insertRow(table->rowCount());
        ids.push_back(id.toInt());

        for (int i=0; i<3; i++)
        {
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
            table->setItem(table->rowCount()-1, i, table_content.constLast().get());
        }

        this->buildTable();
        QMessageBox::information(this, "Powodzenie!", "Czytelnik pomyślnie dodany !");
    }
    else QMessageBox::critical(this, "Błąd zapytania !", query.lastError().text());
}

void Readers::commitClicked()
{
    if (rows_affected.isEmpty())
    {
        QMessageBox::warning(this, "Uwaga!", "Nie dokonałeś żadnej zmiany !");
        return;
    }

    int size = rows_affected.size();
    for (int i=0; i<size; i++)
    {
        int row = rows_affected.pop();
        QString name = table->item(row,0)->text();
        QString surname = table->item(row,1)->text();
        QString books = table->item(row,2)->text();
        if (books.isEmpty()) books = "0";
        if (books.toInt() < 0)
        {
            QMessageBox::critical(this, "Błąd!", "Ilość pożyczonych książek nie może być ujemna ! (wiersz "+QString::number(row));
            rows_affected.push_back(row);
            return;
        }
        if (books.toInt() > 3)
        {
            QMessageBox::critical(this, "Błąd!", "Czytelnik może pożyczyć maksymalnie 3 książki !");
            return;
        }
        if (books != "0")
        {
            int ret = QMessageBox::warning(this, "Uwaga!", "Zalecane jest nadanie ilości książek równej zero. Czy kontynuować ?", QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::No) return;
        }

        // sprawdź unikalność nazwiska
        extern class QSqlDatabase db;
        QSqlQuery query(db);
        if (query.exec("SELECT surname FROM readers WHERE surname != '"+surname+"'"))
        {
            while (query.next())
            {
                if (query.value("surname").toString() == surname)
                {
                    QMessageBox::critical(this, "Błąd!", "Takie nazwisko już istnieje !");
                    return;
                }
            }
        }

        QString s_query = "UPDATE readers SET name='"+name+"', surname='"+surname+"', borrowed_books="+books+" WHERE id="+QString::number(ids[row]);
        QMessageBox::critical(this, "elo", s_query);
        query.clear();
        if (query.exec(s_query)) QMessageBox::information(this, "Powodzenie!", "Pomyślnie edytowano wiersz nr "+QString::number(row+1));
        else QMessageBox::critical(this, "Błąd zapytania !", query.lastError().text());
    }
    this->buildTable();
}

void Readers::clearClicked()
{
    rows_affected.clear();
    ids.clear();
    this->buildTable();
}

void Readers::deletionClicked()
{
    bool is_checked = false;
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    QString s_query = "DELETE FROM readers WHERE id=";
    for (int i=0; i<checkboxes.size(); i++)
    {
        if (checkboxes[i]->checkState() == Qt::Checked)
        {
            if (!is_checked) s_query.append(QString::number(ids[i])); // jednocześnie wykorzystuje zmienną do sprawdzenia czy to jest pierwszy wiersz ?
            else s_query.append(" OR id="+QString::number(ids[i]));
            is_checked = true;
        }
    }

    if (!is_checked)
    {
        QMessageBox::warning(this, "Uwaga!", "Nie zaznaczyłeś żadnego wiersza !");
        return;
    }

    if (query.exec(s_query))
    {
        this->buildTable();
        QMessageBox::information(this, "Powodzenie!", "Wiersze pomyślnie usunięte!");
    }
    else QMessageBox::critical(this, "Uwaga!", query.lastError().text());
}

void Readers::cellDoubleClicked(int row, int column)
{
    if (row == -1 || column == -1) return;
    if (column == 2) books2remember = table->item(row, column)->text(); // zapamiętuje poprzednią wartość w przypadku, gdy podejmowana jest próba edycji kolumny książek
}

void Readers::cellChanged(int row, int column)
{
    if (!editing || row == -1 || column == -1 || table->item(row, column) == 0 || table->rowCount()-1 == row) return;
    if (table->item(row,column)->text() == books2remember) return;
    if (column == 2 && table->item(row, column)->text() != "0")
    {
        int ret = QMessageBox::warning(this, "Uwaga!", "Zamierzasz edytować kolumnę liczby książek!\n Jej ręczna zmiana może spowodować niespójność danych!\n Aby poprawnie wykonać tę operację należy użyć opcji pożycz/oddaj książkę.\n Czy chcesz kontynuować ?", QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            if (rows_affected.contains(row)) rows_affected.push_back(row);
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
    if (!rows_affected.contains(row)) rows_affected.push_back(row);
}

Readers::~Readers()
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
