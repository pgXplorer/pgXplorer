#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>

class TableView : public QMainWindow
{
    Q_OBJECT

private:
    QToolBar* tb;
    QSqlQueryModel *qryMdl;
    QTableView *tview;
    QString sql;
    QStringList whereCl;
    QStringList orderCl;
    QStringList groupCl;
    QString limit;
    QStringList offsetList;
    int fetchSiz;
    bool quickFetch;
    bool canFetchMore;

public:
    TableView(QString const, QString const, Qt::WidgetAttribute f);
    ~TableView() {
        delete qryMdl;
        delete tview;
    }
    virtual void contextMenuEvent(QContextMenuEvent *);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void fetchData();
    void fetchMore();
    void copyc();
    void copych();
};

#endif // TABLEVIEW_H