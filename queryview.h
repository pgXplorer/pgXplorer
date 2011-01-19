#ifndef QUERYVIEW_H
#define QUERYVIEW_H

#include <QSqlTableModel>
#include <QSqlError>
#include <QtGui>
#include <QObject>

class QueryView : public QMainWindow
{
    Q_OBJECT
private:
    QTableView *tview;

public:
    /*! QueryView2 launch the query.
     */
    QueryView(QWidget *parent, QSqlQueryModel*, QString const,
               int const, int const, int const, Qt::WidgetAttribute);
    ~QueryView();
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    virtual void closeEvent(QCloseEvent *);
    //virtual void keyPressEvent(QKeyEvent *e);
    void setMod(QSqlQueryModel* mod)
    {
       tview->setModel(mod);
    }

private slots:
    void copyc();
    void copych();

Q_SIGNALS:

};

#endif // QUERYVIEW_H
