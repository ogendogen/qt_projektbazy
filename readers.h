#ifndef READERS_H
#define READERS_H

#include <QWidget>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QStack>
#include <memory>

namespace Ui {
class Readers;
}

class Readers : public QWidget
{
    Q_OBJECT

public:
    explicit Readers(QWidget *parent = 0, bool p_read=false, bool p_add=false, bool p_edit=false, bool p_deleting=false);
    void buildTable();
    ~Readers();

private slots:
    void addClicked();
    void commitClicked();
    void clearClicked();
    void deletionClicked();
    void cellChanged(int, int);
    void cellDoubleClicked(int, int);

private:
    Ui::Readers *ui;
    bool reading;
    bool adding;
    bool editing;
    bool deleting;

    QTableWidget *table;
    QVector<std::shared_ptr<QTableWidgetItem>> table_content;
    QVector<std::shared_ptr<QCheckBox>> checkboxes; // id = wierszowi
    QPushButton *add;
    QPushButton *commit;
    QPushButton *clear;
    QPushButton *deletion;

    QString getUniqueID(); // pobierz unikalne id
    QStack <int> rows_affected; // wiersze, które zostały zmienione (do edycji)
    QVector <int> ids; // id wierszy w bazie
    QSize getQTableSize(QTableWidget *t);
    QString books2remember; // gdy użytkownik zaczyna edytować kolumnę książek to zapamiętywana jest stara wartość - jeżeli po ostrzeżeniu zdecyduje się cofnąć decyzje to wczytywana jest ta wartość
};

#endif // READERS_H
