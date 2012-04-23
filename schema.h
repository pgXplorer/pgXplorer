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

#ifndef SCHEMA_H
#define SCHEMA_H

#include <QMouseEvent>
#include <QSqlDatabase>
#include <QtGui>
#include "mainwin.h"
#include "schemalink.h"
#include "tablelink.h"
#include "view.h"
#include "functionlink.h"

#define SCHEMA_WIDTH 70
#define SCHEMA_HEIGHT 40
#define A_RADIUS (SCHEMA_WIDTH*5)
#define B_RADIUS (SCHEMA_HEIGHT*6)

class View;

class Schema : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    MainWin *mainwin;
    int number_of_tables;
    int number_of_views;
    int number_of_functions;
    QList<Table*> table_list;
    QList<View*> view_list;
    QList<Function*> function_list;
    Database *parent_database;
    QString name;
    bool status;
    bool collapsed;
    int schema_index;
    int number_of_schemas;
    SchemaLink *schema_link;
    //QList<TableLink *> table_link_list;
    //QList<FunctionLink *> function_link_list;
    QPointF newPos;

protected:
    //QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

public:
    enum { Types = UserType + 2000 };
    int type() const
    {
        return Types;
    }
    Schema(MainWin *, Database *, QString, int, uint);
    ~Schema()
    {
    };
    bool advance();
    void addEdge(SchemaLink *schema_link);
    //void addEdge(TableLink *table_link);
    //void addEdge(FunctionLink *table_link);
    //SchemaLink *dblink() const;
    //QList<TableLink*> tablelinks() const;
    QRectF boundingRect() const
    {
        return QRectF(-35, -30, 80, 50);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *)
    {
        if(this->isSelected())
            painter->setPen(QColor(100,100,100));
        else
            painter->setPen(QColor(0,0,0,0));
        if(this->getSchemaCollapsed()) {
            painter->setBrush(QColor(200,200,200));
            painter->drawRect(-35,-20,SCHEMA_WIDTH,SCHEMA_HEIGHT);
            painter->setBrush(QColor(190,190,190));
            QPointF right[4] = {
                 QPointF(+35,-20),
                 QPointF(+35,+20),
                 QPointF(+40,+10),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(220,220,220));
            QPointF top[4] = {
                 QPointF(-25,-30),
                 QPointF(-35,-20),
                 QPointF(+35,-20),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-3*name.left(8).length(),+5);
            painter->drawText(textPos, name.length()>8?name.left(8)+
                              "..":name);
        }
        else {
            painter->setBrush(QColor(150,150,200));
            painter->drawRect(-35,-20,SCHEMA_WIDTH,SCHEMA_HEIGHT);
            painter->setBrush(QColor(120,120,150));
            QPointF right[4] = {
                 QPointF(+35,-20),
                 QPointF(+35,+20),
                 QPointF(+40,+10),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(200,200,250));
            QPointF top[4] = {
                 QPointF(-25,-30),
                 QPointF(-35,-20),
                 QPointF(+35,-20),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(50,50,100));
            QPointF textPos(-3*name.left(8).length(),+5);
            painter->drawText(textPos, name.length()>8?name.left(8)+
                              "..":name);
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }
    QString getName()
    {
        return this->name;
    }
    void setName(QString name)
    {
        this->name = name;
    }
    bool getSchemaCollapsed()
    {
        return this->collapsed;
    }
    void setSchemaCollapsed(bool collapsed)
    {
        this->collapsed = collapsed;
    }
    bool getStatus()
    {
        return this->status;
    }
    void setStatus(bool status)
    {
        this->status = status;
    }
    void setRt(int x, int y, qreal radius, qreal dtheta)
    {
        setX(x + radius*sin(dtheta));
        setY(y + radius*cos(dtheta));
    }
    Database *getParent()
    {
        return this->parent_database;
    }
    void setParent(Database *database)
    {
        this->parent_database = database;
    }
    int getTableCount()
    {
        return number_of_tables;
    }
    void setTableCount(int number_of_tables)
    {
        this->number_of_tables = number_of_tables;
    }
    int getViewCount()
    {
        return number_of_views;
    }
    void setViewCount(int number_of_views)
    {
        this->number_of_views = number_of_views;
    }
    int getFunctionCount()
    {
        return number_of_functions;
    }
    void setFunctionCount(int number_of_functions)
    {
        this->number_of_functions = number_of_functions;
    }
    QList<Table*> getTableList()
    {
        return this->table_list;
    }
    void setTableList(QList<Table*> table_list)
    {
        this->table_list = table_list;
    }
    void appendTableList(Table *table)
    {
        table_list.append(table);
    }
    QList<View*> getViewList()
    {
        return this->view_list;
    }
    void setViewList(QList<View*> view_list)
    {
        this->view_list = view_list;
    }
    void appendViewList(View *view)
    {
        view_list.append(view);
    }
    QList<Function*> getFunctionList()
    {
        return this->function_list;
    }
    void setFunctionList(QList<Function*> function_list)
    {
        this->function_list = function_list;
    }
    void appendFunctionList(Function *function)
    {
        function_list.append(function);
    }
//    QList<TableLink*> getTableLinkList()
//    {
//        return this->table_link_list;
//    }
//    QList<FunctionLink*> getFunctionLinkList()
//    {
//        return this->function_link_list;
//    }
    void resetPos();
    void resetTables();
    void resetTablesVertically();
    void resetTablesVertically2();
    void resetViews();
    void resetViewsVertically2();
    void resetFunctions();
    void resetFunctionsVertically2();
    void horizontalPosition();

public slots:
    void populateSchemaTables();
    void populateSchemaViews();
    void populateSchemaFunctions();

signals:
    void expandSchemaTables(Schema*);
    void collapseSchemaTables(Schema*);
    void expandSchemaViews(Schema*);
    void collapseSchemaViews(Schema*);
    void collapseOtherSchemas(Schema*);
    void expandSchemaFunctions(Schema*);
    void collapseSchemaFunctions(Schema*);
    void newTable(Schema*);
    void newView(Schema*);
    void newFunction(Schema*);
};

#endif // SCHEMA_H
