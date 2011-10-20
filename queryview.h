#ifndef QUERYVIEW_H
#define QUERYVIEW_H

#include <QSqlTableModel>
#include <QSqlError>
#include <QtGui>
#include <QObject>
#include "pgxconsole.h"

class QueryView : public QMainWindow
{
    Q_OBJECT
private:
    QTableView *qview;
    SqlMdl* sqlmd;
    QString sql;
    QStringList whereCl;
    QStringList orderCl;
    ulong thisQueryViewId;
    bool threadBusy;

public:
    static ulong queryViewObjectId;
    QueryView(QWidget *parent, QSqlQueryModel*, QString const,
               int const, int const, int const, Qt::WidgetAttribute);
    ~QueryView();
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    virtual void closeEvent(QCloseEvent *);
    //virtual void keyPressEvent(QKeyEvent *e);
    void setMod(SqlMdl* mod)
    {
        sqlmd = mod;
        qview->setModel(mod);
    }

    SqlMdl* getMod()
    {
        return this->sqlmd;
    }

    ulong getId()
    {
        return this->thisQueryViewId;
    }

private slots:
    void copyc();
    void copych();
    void fetchDataSlot(SqlMdl*, int, qint32, qint32);
    void busySlot();
    void fullscreen();
    void restore();

Q_SIGNALS:
    void errMesg(QString, uint);
    void finished();
    void cleanSignal();
};

#endif // QUERYVIEW_H
