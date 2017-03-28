#include "users.h"
#include "ui_users.h"
#include <QDebug>
#include <QDesktopWidget>

static QSize myGetQTableWidgetSize(QTableWidget *t) {
   int w = t->verticalHeader()->width() + 4; // +4 seems to be needed
   for (int i = 0; i < t->columnCount(); i++)
      w += t->columnWidth(i); // seems to include gridline (on my machine)
   int h = t->horizontalHeader()->height() + 4;
   for (int i = 0; i < t->rowCount(); i++)
      h += t->rowHeight(i);
   return QSize(w, h);
}

Users::Users(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Users)
{
    ui->setupUi(this);
    extern class QSqlDatabase db;
    while (!db.isOpen()) // upewnienie się że połączenie jest aktywne
    {
        int ret = QMessageBox::critical(this,"Błąd!", "Połączenie z bazą zostało zerwane! Czy odświeżyć połączenie ?", QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::Yes) db.open();
        else
        {
            this->close();
            return;
        }
    }


    table = nullptr;
    add = nullptr;
    commit = nullptr;
    deletion = nullptr;
    clear = nullptr;
    this->setGeometry(0,0,QApplication::desktop()->width(), QApplication::desktop()->height());
    this->buildTable();
}

void Users::buildTable()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec("SELECT * FROM accounts ORDER BY id ASC")) // pobierz wszystkie rekordy
    {
        if (!table_content.isEmpty()) table_content.clear();
        if (table) delete table;
        table = new QTableWidget(this); // utwórz tabele
        table->setParent(this);
        table->setColumnCount(9);
        table->setRowCount(0);
        table->move(0,0);
        QStringList labels;
        labels<<"Login"<<"Hasło"<<"Odczyt"<<"Dodawanie"<<"Edycja"<<"Usuwanie"<<"Konta"<<"ROOT"<<"Opis"; // nazwij kolumny
        table->setHorizontalHeaderLabels(labels);

        if (!ids.isEmpty()) ids.clear();
        int i = 0;
        while(query.next())
        {
            table->insertRow(i); // kolejny wiersz
            //table_content.push_back(QVector <std::shared_ptr<QTableWidgetItem>>()); // utwórz wewnętrzny vector inteligentnych wskaźników na itemy w wierszu (reprezentant wiersza)

            ids.push_back(query.value(0).toInt());
            for (int j=0; j<=1; j++) // <0;1> - login,hasło
            {
                table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(query.value(j+1).toString()))); // wrzuć do vectora
                table->setItem(i,j,table_content.constLast().get()); // użyj "gołego" wskaźnika
                //if (j == 0) table->item(i,j)->setFlags(Qt::ItemIsSelectable);
            }

            QString str_permissions = query.value("PRIVILIGES").toString();
            QStringList permissions = str_permissions.split("|",QString::SkipEmptyParts);
            for (int j=2; j<=7; j++) // rozkoduj uprawnienia
            {
                table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
                table->setItem(i,j,table_content.constLast().get());
                if (permissions[j-2] == "1") table->item(i,j)->setCheckState(Qt::Checked); else table->item(i,j)->setCheckState(Qt::Unchecked); // zaznacz odpowiednie checkboxy
            }
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem(query.value("DESCRIPTION").toString()))); // opis konta
            table->setItem(i,8,table_content.constLast().get());
            i++;
            //table->item(i,9)->setText(query.value("DESCRIPTION").toString());
        }

        table->insertRow(i); // upewnij się, że istnieje przynajmniej jeden wiersz (do dodawania)

        //table_content.push_back(QVector<std::shared_ptr<QTableWidgetItem>>());
        for (int x=0; x<9; x++)
        {
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
            if (x>=2 && x<=7) table_content.constLast()->setCheckState(Qt::Unchecked); else table_content.constLast()->setText("");
            table->setItem(i,x,table_content.constLast().get());
        }
    }
    else QMessageBox::critical(this,"Błąd!",query.lastError().text(), QMessageBox::Ok); // problem z zapytaniem
    for (int i=0; i<9; i++) table->item(0,i)->setFlags(Qt::ItemIsSelectable); // zablokowanie edycji root'a

    checkboxes.clear(); // dobudowanie checkboxów do usuwania
    for (int i=0; i<table->rowCount()-1; i++)
    {
        checkboxes.push_back(std::shared_ptr<QCheckBox>(new QCheckBox(this)));
        checkboxes[i]->move(myGetQTableWidgetSize(table).width()+25, i*table->rowHeight(i)+table->horizontalHeader()->height()); // rozmiesc checkboxy - x = szerokość tabeli+25, y = id wiersza * jego wysokość + wysokość headera
        checkboxes[i]->show();
    }
    if (table->rowCount() >= 1) checkboxes[0]->setEnabled(false); // zablokuj checkbox dla root'a

    QSize real_size = myGetQTableWidgetSize(table); // oblicz rzeczywisty rozmiar tabeli
    table->setGeometry(0,0,real_size.width()+13, real_size.height()+4); // przekształć tabele
    connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(cellChanged(int, int))); // zdarzenie edycji - zapamiętanie edytowanej komórki
    //connect(table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemChanged(QTableWidgetItem*));
    table->setVisible(true);
    table->show();

    if (add) delete add;
    add = new QPushButton("Dodaj",this); // umieść kolejne przyciski względem tabeli: dodawanie, zatwierdzanie, czyszczenie
    add->move(0,real_size.height()+4+10);
    connect(add, SIGNAL(clicked()), this, SLOT(addClicked()));
    add->show();

    if (commit) delete commit;
    commit = new QPushButton("Zatwierdź zmiany",this);
    commit->move(add->x()+add->width()+10,real_size.height()+4+10);
    commit->setEnabled(true);
    connect(commit, SIGNAL(clicked()), this, SLOT(commitClicked()));
    commit->show();

    if (clear) delete clear;
    clear = new QPushButton("Cofnij edycje", this);
    clear->move(commit->x()+commit->width()+10,real_size.height()+4+10);
    clear->setEnabled(true);
    connect(clear, SIGNAL(clicked()), this, SLOT(clearClicked()));
    clear->show();

    if (deletion) delete deletion;
    deletion = new QPushButton("Usuń zaznaczone",this);
    deletion->move(clear->x()+clear->width()+10,real_size.height()+4+10);
    connect(deletion, SIGNAL(clicked()), this, SLOT(deletionClicked()));
    deletion->show();
}

void Users::addClicked() // dodanie nowego usera
{
    int new_row = table->rowCount()-1; // id nowego wiersza

    QString login = table->item(new_row,0)->text();
    QString password = table->item(new_row,1)->text();
    if (login.isEmpty() || password.isEmpty()) // jesli pusto ...
    {
        QMessageBox::warning(this,"Uwaga!","Jedno z pól jest puste !");
        return;
    }

    for (int i=0; i<table->rowCount(); i++)
    {
        if (table->item(i,0)->text() == login && i != new_row)
        {
            QMessageBox::warning(this,"Uwaga!","Login musi być unikalny !",QMessageBox::Ok);
            return;
        }
    }
    QString permissions = "";
    for (int i=2; i<=7; i++) // kodowanie uprawnień
    {
        if (table->item(new_row,i)->checkState() == Qt::Checked) permissions.append("1|");
        else if (table->item(new_row,i)->checkState() == Qt::Unchecked) permissions.append("0|");
    }
    permissions.remove(permissions.size()-1, 1); // ucinanie ostatniego niepotrzebnego znaku '|'

    if (permissions == "0|0|0|0|0|0")
    {
        int ret = QMessageBox::warning(this,"Uwaga!","Nie nadałeś żadnych uprawnień! Czy chcesz koontynuować ?",QMessageBox::Yes, QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }
    QString desc = table->item(new_row,8)->text();
    if (desc.isEmpty())
    {
        QMessageBox::warning(this,"Uwaga!","Opis nie może być pusty !",QMessageBox::Ok);
        return;
    }

    // hashowanie hasła
    const char *tab = password.toStdString().c_str();
    int len = strlen(tab);
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(tab,len);
    password = QString(hash.result().toHex());
    if (tab) delete tab;

    extern class QSqlDatabase db;
    QSqlQuery query(db);
    QString id = this->getUniqueID();
    if (query.exec("INSERT INTO accounts (id, login, password, priviliges, description) VALUES('"+id+"','"+login+"','"+password+"','"+permissions+"','"+desc+"')")) // wpisz nowego usera
    {
        QMessageBox::information(this,"Powodzenie","Nowy użytkownik dodany poprawnie!", QMessageBox::Ok); // powodzenie - przekształć tabele
        new_row++;
        table->insertRow(new_row); // utwórz kolejny wiersz do dodawania
        //table_content.push_back(QVector <std::shared_ptr<QTableWidgetItem>>());
        for (int i=0; i<=8; i++)
        {
            table_content.push_back(std::shared_ptr<QTableWidgetItem>(new QTableWidgetItem()));
            if (i>=2 && i<=7)
            {
                table_content.constLast()->setCheckState(Qt::Unchecked);
                table->setItem(new_row, i, table_content.constLast().get());
            }
            else
            {
                table_content.constLast()->setText("");
                table->setItem(new_row, i, table_content.constLast().get());
            }
        }

        QSize table_size = myGetQTableWidgetSize(table);
        table->setGeometry(0,0,table_size.width(),table_size.height()); // przekształć tabele
        table->item(table->rowCount()-2, 1)->setText(password);
        emit cellChanged(-2, -2); // wywołanie slotu, aby edycja linijke wyżej nie spowodowała włączenia przycisku 'zatwierdz edycje'
        commit->move(commit->x(), table->height()+25); // przesuń przyciski w dół
        add->move(add->x(), table->height()+25);
        clear->move(clear->x(), table->height()+25);
        deletion->move(deletion->x(), table->height()+25);

        //dodaj nowego checkbox'a
        checkboxes.push_back(std::shared_ptr<QCheckBox>(new QCheckBox(this)));
        checkboxes[checkboxes.size()-1]->setChecked(false);
        checkboxes[checkboxes.size()-1]->show();

        for (int i=0; i<checkboxes.size(); i++) checkboxes[i]->move(myGetQTableWidgetSize(table).width()+25, i*table->rowHeight(i)+table->horizontalHeader()->height()); // dostosuj checkboxy na nowo
    }
    else
    {
        QMessageBox::critical(this,"Błąd zapytania!", query.lastError().text(), QMessageBox::Ok);
        return;
    }
}

void Users::commitClicked() // zatwierdz zmiany i wyślij do bazy
{
    if (rows_affected.isEmpty()) // stos pusty - brak zmian
    {
        QMessageBox::information(this, "Uwaga!", "Nie dokonałeś żadnej zmiany!", QMessageBox::Ok);
        return;
    }

    int size = rows_affected.size();
    for (int i=0; i<size; i++) // pobierz dane każdego zmienionego wiersza i wysyłaj
    {
        int row = rows_affected.pop();
        if (table->item(row,0) == 0) continue; // jesli dany item nie istnieje, oznacza to że cały rząd nie istnieje

        QString login = table->item(row,0)->text();
        QString password = table->item(row,1)->text();

        if (login.isEmpty() || password.isEmpty())
        {
            QMessageBox::warning(this,"Uwaga!","Hasło lub login jest puste!");
            return;
        }

        for (int i=0; i<table->rowCount(); i++) // unikalność loginu
        {
            if (table->item(i,1)->text() == login && i != row)
            {
                QMessageBox::warning(this,"Uwaga!","Login musi być unikalny !",QMessageBox::Ok);
                return;
            }
        }
        // hashowanie hasła
        const char *tab = password.toStdString().c_str();
        int len = strlen(tab);
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(tab,len);
        password = QString(hash.result().toHex());
        if (tab) delete tab;

        QString permissions = "";
        for (int j=2; j<=7; j++) // konwersja checkbox'ów na zakodowane uprawnienia
        {
            if (table->item(row,j)->checkState() == Qt::Checked) permissions.append("1|");
            else if (table->item(row,j)->checkState() == Qt::Unchecked) permissions.append("0|");
        }
        permissions.remove(permissions.size()-1, 1); // ucinanie ostatniego niepotrzebnego znaku '|'
        if (permissions == "0|0|0|0|0|0")
        {
            int ret = QMessageBox::warning(this,"Uwaga!","Nie nadałeś żadnych uprawnień! Czy chcesz koontynuować ?",QMessageBox::Yes, QMessageBox::No);
            if (ret == QMessageBox::No) return;
        }

        QString desc = table->item(row,8)->text();
        if (desc.isEmpty())
        {
            QMessageBox::warning(this,"Uwaga!","Opis nie może być pusty !",QMessageBox::Ok);
            return;
        }

        extern class QSqlDatabase db;
        QSqlQuery query(db); // aktualizuj wiersz w którym występuje "stare" id
        QString id = getUniqueID();
        QString s_query = "UPDATE accounts SET login='"+login+"', password='"+password+"', priviliges='"+permissions+"', description='"+desc+"' WHERE id="+id;

        if (query.exec(s_query))
        {
            if (table->item(row, 1)->text().length() != 32) table->item(row, 1)->setText(password);
        }
        else
        {
            QMessageBox::critical(this, "Błąd zapytania!", query.lastError().text(), QMessageBox::Ok);
            return;
        }
    }
    QMessageBox::information(this, "Powodzenie", "Zmiany dokonane prawidłowo!", QMessageBox::Ok);
}

void Users::clearClicked() // cofnij zmiany
{
    rows_affected.clear();
    ids.clear();
    this->buildTable();
}

void Users::deletionClicked()
{
    bool is_checked = false;// czy w ogóle jakiś został wciśnięty ?
    QString s_query = "DELETE FROM accounts WHERE ";
    QStack <int>rows_to_remove;
    for (int i=0; i<checkboxes.size(); i++)
    {
        if (checkboxes[i]->checkState() == Qt::Checked)
        {
            if (is_checked) s_query.append(" OR "); // nie pierwszy rekord do usunięcia
            is_checked = true;
            s_query.append("id="+QString::number(ids[i]));
            rows_to_remove.push(i);
        }
    }

    if (!is_checked) // brak zaznaczonego checkboxa - przerwij
    {
        QMessageBox::warning(this,"Uwaga!","Nie zaznaczyłeś żadnego wiersza!",QMessageBox::Ok);
        return;
    }

    extern class QSqlDatabase db;
    QSqlQuery query(db);
    if (query.exec(s_query))
    {
        this->buildTable();
        QMessageBox::information(this,"Powodzenie!","Pomyślnie usunięto zaznaczone wiersze!",QMessageBox::Ok);
    }
    else QMessageBox::critical(this,"Błąd zapytania!",query.lastError().text(),QMessageBox::Ok);
}

void Users::cellChanged(int row, int column)
{
    if (row == -2 && row == -2) // zablokowanie włączenia przycisku przy hashowaniu hasła po dodaniu nowego konta
    {
        rows_affected.pop(); // zdejmij ostatni element ze stosu - hashowanie hasła nowego użytkownika
        return;
    }
    if (row == -1 || column == -1 || rows_affected.contains(row) || table->item(row,column) == 0 || row == table->rowCount()-1) return; // rząd nie istnieje lub ostatni rząd lub już jest na stosie

    // ogólny przypadek
    rows_affected.push(row);
}

QString Users::getUniqueID()
{
    extern class QSqlDatabase db;
    QSqlQuery query(db);
    int id;
    if (query.exec("SELECT users_id FROM ids"))
    {
        if (query.next()) id = query.value("users_id").toInt();
        else return "select error";
    }
    query.clear();
    if (query.exec("UPDATE ids SET users_id="+QString::number(id+1))) return QString::number(id);
    return "error";
}

Users::~Users()
{
    delete ui;
    if (!table_content.isEmpty()) table_content.clear();
    delete table;
    delete commit;
    delete add;
    delete clear;
    if (!checkboxes.isEmpty()) checkboxes.clear();
    delete deletion;
}
