/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef EXPLAINVIEW_H
#define EXPLAINVIEW_H

#include <QSqlTableModel>
#include <QSqlError>
#include <QtGui>

class ExplainView : public QTableView
{
private:
    QSqlQueryModel *mod;

public:
    ExplainView(QSqlQueryModel *, QString const, Qt::WidgetAttribute f);
    ~ExplainView(){};
    //virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    virtual void closeEvent(QCloseEvent*);
    //virtual void resizeEvent(QResizeEvent *);
    QSqlQueryModel *getMod()
    {
        return this->mod;
    }
    void setMod(QSqlQueryModel *mod)
    {
       this->setModel(mod);
       this->mod = mod;
    }
};

#endif // EXPLAINVIEW_H
