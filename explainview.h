#ifndef EXPLAINVIEW_H
#define EXPLAINVIEW_H

#include <QSqlTableModel>
#include <QSqlError>
#include <QtGui>

class ExplainView : public QTableView
{
private:
    QSqlQueryModel* mod;

public:
    ExplainView(QSqlQueryModel*, QString const, Qt::WidgetAttribute f);
    ~ExplainView(){};
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    virtual void closeEvent(QCloseEvent *);
    //virtual void resizeEvent(QResizeEvent *);
    QSqlQueryModel* getMod()
    {
        return this->mod;
    }
    void setMod(QSqlQueryModel* mod)
    {
       this->setModel(mod);
       this->mod = mod;
    }
};

#endif // EXPLAINVIEW_H
