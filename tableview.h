#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>
#include "database.h"

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
    qint32 rowsFrom;
    qint32 rowsTo;
    qint32 colcount;
    QString host;
    int port;
    QString dbname;
    QString user;
    QString password;
    bool threadBusy;
    ulong thisTableViewId;

public:
    static ulong tableViewObjectId;
    TableView(Database*, QString const, QString const, Qt::WidgetAttribute f);
    ~TableView()
    {
    };

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

    void fetchMoreData(const QString, const qint32, const QString, const QString, const QString);
    void fetchMoreData2(const QString, const qint32, const QString, const QString, const QString);

private slots:
    void fetchData(const QString, const qint32, const QString, const QString, const QString);
    void fetchMore();
    void copyc();
    void copych();
    void busySlot();
    void updRowCntSlot();
    void fullscreen();
    void restore();

Q_SIGNALS:
    void busySignal();
    void updRowCntSignal();
};

#endif // TABLEVIEW_H
