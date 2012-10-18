/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <dj@pgxplorer.com>

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

#ifndef TABLE_H
#define TABLE_H

#include <QMouseEvent>
#include <QSqlTableModel>
#include <QSqlIndex>
#include <QtGui>
#include "database.h"
#include "tablelink.h"

#define TABLE_WIDTH 80
#define TABLE_HEIGHT 20

class Table : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    bool hidden;
    Database *database;
    QSqlQueryModel *query_model;
    Schema *parent_schema;
    QString table_name;
    QStringList column_list;
    QStringList column_types;
    QStringList column_lengths;
    QStringList column_nulls;
    QStringList column_defaults;
    QStringList primary_key;
    QStringList table_constraint_names;
    QStringList table_constraints;
    bool oid;
    bool primary_key_has_oid;
    QString primary_key_name;
    QBrush pink_brush;
    bool status;
    bool collapsed;
    bool searched;
    bool is_view;
    int table_index;
    //QList<TableLink *> edgeList;
    QPointF newPos;
    int ascii_length;
    int utf8_length;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    //void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    //void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    //void dropEvent(QGraphicsSceneDragDropEvent *event);

public:
    enum { Typet = UserType + 3000 };
    int type() const
    {
        return Typet;
    }
    Table(Database *database, Schema *, QString, int, QColor);
    ~Table(){};
    void createBrush();
    void defaultPosition();
    void verticalPosition();
    bool advance();
    //void addEdge(TableLink *edge);
    //QList<TableLink *> edges() const;
    QRectF boundingRect() const
    {
        return QRectF(-40,-20,85,30);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *)
    {
        const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
        if(isSelected())
            if(is_view)
                painter->setPen(QColor(100,50,0));
            else
                painter->setPen(QColor(100,50,50));
        else
            painter->setPen(QColor(0,0,0,0));
        if(!searched) {
            painter->setBrush(QColor(220,220,220));
            painter->drawRect(-40,-10,TABLE_WIDTH,TABLE_HEIGHT);
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
            QPointF textPos(-6*table_name.left(6).length(),+5);
            if(lod > 0.5)
                painter->drawText(textPos, table_name.length()>8?table_name.left(8)+
                              "..":table_name);
        }
        else if(is_view) {
            painter->setBrush(QColor(200,150,0));
            painter->drawRect(-40,-10,TABLE_WIDTH,TABLE_HEIGHT);
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
            QPointF textPos(-6*table_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, table_name.length()>6?table_name.left(6)+
                              "..":table_name);
                else
                    painter->drawText(textPos, table_name.length()>10?table_name.left(10)+
                              "..":table_name);
            }
        }
        else {
            painter->setBrush(QColor(200,150,150));
            //painter->setBrush(pink_brush);
            painter->drawRect(-40,-10,TABLE_WIDTH,TABLE_HEIGHT);
            painter->setBrush(QColor(230,200,200));
            QPointF right[4] = {
                QPointF(+40,-10),
                QPointF(+40,+10),
                QPointF(+45,0),
                QPointF(+45,-20)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(250,220,220));
            QPointF top[4] = {
                QPointF(-30,-20),
                QPointF(-40,-10),
                QPointF(+40,-10),
                QPointF(+45,-20)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(100,50,50));
            QPointF textPos(-6*table_name.left(6).length(),+5);
            if(lod > 0.5) {
                if(ascii_length != utf8_length)
                    painter->drawText(textPos, table_name.length()>6?table_name.left(6)+
                              "..":table_name);
                else
                    painter->drawText(textPos, table_name.length()>10?table_name.left(10)+
                              "..":table_name);
            }
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QString getName()
    {
        return table_name;
    }

    QString getFullName();

    void setName(QString name)
    {
        this->table_name = name;
        update();
    }
    bool getCollapsed()
    {
        return collapsed;
    }
    bool getSearched()
    {
        return searched;
    }
    bool isView()
    {
        return is_view;
    }

    bool hasOid()
    {
        return oid;
    }

    void setView(bool view)
    {
        this->is_view = view;
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
        return status;
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
        return parent_schema;
    }
    QSqlQueryModel *getModel()
    {
        return query_model;
    }
    void setModel(QSqlQueryModel *query_model)
    {
        this->query_model = query_model;
    }
    void setColumnData();
    void setColumnData(QStringList column_list, QStringList column_types, QStringList column_lengths, QStringList column_nulls)
    {
        this->column_list = column_list;
        this->column_types = column_types;
        this->column_lengths = column_lengths;
        this->column_nulls = column_nulls;
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
    QStringList getColumnNulls()
    {
        return column_nulls;
    }
    QStringList getColumnDefaults()
    {
        return column_defaults;
    }
    void copyPrimaryKey();
    void copyConstraints();
    void setPrimaryKey(QStringList primary_key)
    {
        this->primary_key = primary_key;
    }
    QStringList getPrimaryKey()
    {
        return primary_key;
    }
    QString getPrimaryKeyName()
    {
        return primary_key_name;
    }
    QStringList getConstraintNames()
    {
        return table_constraint_names;
    }
    QStringList getConstraints()
    {
        return table_constraints;
    }

    //bool isOidInPrimaryKey()
    //{
    //    return oid_in_primary_key;
    //}

    void showView2(Table*);

public slots:
    void getSearchTerm(QString);
    void verticalPosition2();

signals:
    void expandTable(Database *, Schema *, Table*);
    void designTable(Database *, Schema *, Table*);
    void expandTableDefinition(Schema *, Table*);
    void rename(Database *, Schema *, Table*);
    void clearTable(Database *, Schema *, Table*);
    void dropTable(Database *, Schema *, Table*);
    void collapseTable(Database *, Schema *, Table*);
};

#endif // TABLE_H
