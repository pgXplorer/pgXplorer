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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QMouseEvent>
#include <QtGui>
#include "database.h"
#include "functionlink.h"

#define FUNCTION_WIDTH 80
#define FUNCTION_HEIGHT 20

class Function : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    Database *database;
    Schema *parent_schema;
    QString function_name;
    QString function_args;
    QString function_arg_types;
    bool status;
    bool collapsed;
    bool searched;
    int function_index;
    //QList<FunctionLink *> edgeList;
    QPointF newPos;
    int ascii_length;
    int utf8_length;

protected:
    //QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

public:
    enum { Typet = UserType + 9000 };
    int type() const
    {
        return Typet;
    }
    Function(Database *database, Schema *parent, QString function_name, QString function_args, QString function_arg_types, int index, QColor qcolor);
    ~Function(){};
    void defaultPosition();
    void verticalPosition();
    bool advance();
    //void addEdge(FunctionLink *edge);
    //QList<FunctionLink *> edges() const;
    QRectF boundingRect() const
    {
        return QRectF(-40,-20,85,30);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget)
    {
        const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
        if(isSelected())
            painter->setPen(QColor(50,100,100));
        else
            painter->setPen(QColor(0,0,0,0));
        if(!searched) {
            painter->setBrush(QColor(220,220,220));
            painter->drawRect(-40,-10,FUNCTION_WIDTH,FUNCTION_HEIGHT);
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
            QPointF textPos(-6*function_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, function_name.length()>6?function_name.left(6)+
                              "..":function_name);
                else
                    painter->drawText(textPos, function_name.length()>10?function_name.left(10)+
                              "..":function_name);
            }
        }
        else {
            painter->setBrush(QColor(150,200,200));
            painter->drawRect(-40,-10,FUNCTION_WIDTH,FUNCTION_HEIGHT);
            painter->setBrush(QColor(200,230,230));
            QPointF right[4] = {
                QPointF(+40,-10),
                QPointF(+40,+10),
                QPointF(+45,0),
                QPointF(+45,-20)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(220,250,250));
            QPointF top[4] = {
                QPointF(-30,-20),
                QPointF(-40,-10),
                QPointF(+40,-10),
                QPointF(+45,-20)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(50,100,100));
            QPointF textPos(-6*this->function_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, function_name.length()>6?function_name.left(6)+
                              "..":function_name);
                else
                    painter->drawText(textPos, function_name.length()>10?function_name.left(10)+
                              "..":function_name);
            }
        }
        painter->setRenderHint(QPainter::Antialiasing, false);
    }
    QString getName()
    {
        return this->function_name;
    }
    QString getArgs()
    {
        return this->function_args;
    }
    QString getArgTypes()
    {
        return this->function_arg_types;
    }
    QString getArgTypesToText();
    void setName(QString name)
    {
        this->function_name = name;
    }
    void setArgs(QString args)
    {
        this->function_args = args;
    }
    void setArgTypes(QString arg_types)
    {
        this->function_arg_types = arg_types;
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

public slots:
    void getSearchTerm(QString);
    void verticalPosition2();

Q_SIGNALS:
    void expandFunction(Schema*, Function*);
    void collapseFunction(Database *, Schema *, Function*);
    void runFunction(Database *, Schema *, Function*);
    void dropFunction(Database *, Schema *, Function*);
};

#endif // FUNCTION_H
