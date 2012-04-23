/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

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

#ifndef VIEW_H
#define VIEW_H

#include <QMouseEvent>
#include <QSqlTableModel>
#include <QtGui>
#include "database.h"
//#include "viewlink.h"

#define VIEW_WIDTH 80
#define VIEW_HEIGHT 20

class View : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    Database *database;
    QSqlQueryModel *model;
    Schema *parent_schema;
    QString view_name;
    QStringList column_list;
    QStringList column_types;
    QStringList column_lengths;
    bool status;
    bool collapsed;
    bool searched;
    bool is_view;
    int view_index;
    //QList<ViewLink *> edgeList;
    QPointF newPos;
    int ascii_length;
    int utf8_length;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

public:
    enum { Typet = UserType + 3000 };
    int type() const
    {
        return Typet;
    }
    View(Database *database, Schema *parent, QString view_name, int index, QColor qcolor);
    ~View(){};
    void defaultPosition();
    void verticalPosition();
    bool advance();
    QRectF boundingRect() const
    {
        return QRectF(-40,-20,85,30);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *)
    {
        const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
        if(isSelected())
            painter->setPen(QColor(100,50,0));
        else
            painter->setPen(QColor(0,0,0,0));
        if(!searched) {
            painter->setBrush(QColor(220,220,220));
            painter->drawRect(-40,-10,VIEW_WIDTH,VIEW_HEIGHT);
            painter->setBrush(QColor(200,200,200));
            QPointF right[4] = {
                 QPointF(+40,-10),
                 QPointF(+40,+10),
                 QPointF(+45,0),
                 QPointF(+45,-20)
            };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(210,210,210));
            QPointF top[4] = {
                 QPointF(-30,-20),
                 QPointF(-40,-10),
                 QPointF(+40,-10),
                 QPointF(+45,-20)
            };
            painter->drawPolygon(top, 4);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-6*this->view_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, view_name.length()>6?view_name.left(6)+
                              "..":view_name);
                else
                    painter->drawText(textPos, view_name.length()>10?view_name.left(10)+
                              "..":view_name);
            }
        }
        else {
            painter->setBrush(QColor(200,150,0));
            painter->drawRect(-40,-10,VIEW_WIDTH,VIEW_HEIGHT);
            painter->setBrush(QColor(230,200,0));
            QPointF right[4] = {
                QPointF(+40,-10),
                QPointF(+40,+10),
                QPointF(+45,0),
                QPointF(+45,-20)
            };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(250,220,0));
            QPointF top[4] = {
                QPointF(-30,-20),
                QPointF(-40,-10),
                QPointF(+40,-10),
                QPointF(+45,-20)
            };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(100,50,50));
            QPointF textPos(-6*this->view_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, view_name.length()>6?view_name.left(6)+
                              "..":view_name);
                else
                    painter->drawText(textPos, view_name.length()>10?view_name.left(10)+
                              "..":view_name);
            }
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }
    QString getName()
    {
        return this->view_name;
    }
    void setName(QString name)
    {
        this->view_name = name;
    }
    bool getCollapsed()
    {
        return this->collapsed;
    }
    bool getSearched()
    {
        return this->searched;
    }
    void setCollapsed(bool collapsed)
    {
        this->collapsed = collapsed;
    }
    void setSearched(bool searched)
    {
        this->searched = searched;
    }
    bool getStatus()
    {
        return this->status;
    }
    void setStatus(bool status)
    {
        this->status = status;
    }
    void setParent(Schema *sch)
    {
        this->parent_schema = sch;
    }
    Schema *getParent()
    {
        return this->parent_schema;
    }
    QSqlQueryModel *getModel()
    {
        return this->model;
    }
    void setModel(QSqlQueryModel *model)
    {
        this->model = model;
    }
    void setColumnData();
    void setColumnData(QStringList column_list, QStringList column_types, QStringList column_lengths)
    {
        this->column_list = column_list;
        this->column_types = column_types;
        this->column_lengths = column_lengths;
    }
    QStringList getColumnList()
    {
        return column_list;
    }
    QStringList getColumnTypes()
    {
        return column_types;
    }
    QStringList getColumnLengths()
    {
        return column_lengths;
    }
    void showView2(View *);

public slots:
    void getSearchTerm(QString);
    void verticalPosition2();

signals:
    void expandView(Database *, Schema *, View *);
    void expandViewDefinition(Schema *, View *);
    void clearView(Database *, Schema *, View *);
    void dropView(Database *, Schema *, View *);
    void collapseView(Database *, Schema *, View *);
};

#endif // VIEW_H
