#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>

#define FETCHSIZ 10000

class TableView : public QMainWindow
{
    Q_OBJECT

private:
    QTime t;
    QToolBar* tb;
    QSqlQueryModel *qryMdl;
    QTableView *tview;
    QString sql;
    QStringList whereCl;
    QStringList orderCl;
    QStringList groupCl;
    QString limit;
    QStringList offsetList;
    bool quickFetch;
    bool canFetchMore;
    qint32 rowcount;
    qint32 colcount;

public:
    TableView(QString const, QString const, Qt::WidgetAttribute f);
    ~TableView() {
        delete qryMdl;
        delete tview;
    }
    virtual void contextMenuEvent(QContextMenuEvent *);
    virtual void closeEvent(QCloseEvent *);
    QSqlQueryModel* getMdl()
    {
        return qryMdl;
    }

    void setMdl(QSqlQueryModel* qryMdl)
    {
        this->qryMdl = qryMdl;
    }

private slots:
    void fetchData();
    void fetchMore();
    void copyc();
    void copych();
    void updRowCntSlot();

Q_SIGNALS:
    void updRowCntSignal(qint32,qint32,int);
};

#endif // TABLEVIEW_H
