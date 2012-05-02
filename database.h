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

#ifndef DATABASE_H
#define DATABASE_H

#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>
#include "mainwin.h"
#include "schema.h"
#include "schemalink.h"
#include "table.h"
#include "function.h"

//! Database class
/*!
  Database class derived from QObject and QGraphicsItem.
  Displayed as a 3D cylinder. Contains necessary
  database parameters like server name, port number,
  database name, etc.
 */
class Database : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    int database_id;
    MainWin *mainwin;
    int number_of_schemas;
    QSqlDatabase database_connection;
    QList<Schema*> schema_list;
    QString name;
    QString host;
    QString port;
    QString user;
    QString password;
    bool status;
    bool collapsed;
    QList<SchemaLink*> edgeList;
    QPointF newPos;
    QHash<int, QString> types_hash;
    QStringList table_names_list;
    QStringList view_names_list;
    QStringList function_names_list;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    bool setConnectionProperties(const QString, const qint32, const QString, const QString, const QString);

public:
    static ulong dbViewObjectId;
    enum { Typed = UserType + 1000 };
    int type() const
    {
        return Typed;
    }
    Database(MainWin *, int);
    ~Database(){};
    int getId()
    {
        return database_id;
    }
    QString getTypeText(int oid)
    {
        return types_hash.value(oid);
    }

    void arrangeHorizontally();
    void delDatabase(Database *);
    bool populateDatabase();
    bool advance();
    void addEdge(SchemaLink *);
    QRectF boundingRect() const
    {
        return QRectF(-50, -37.5, 100, 75);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *)
    {
        painter->setPen(QColor(0,0,0,0));

        if(getDatabaseCollapsed()) {
            painter->setBrush(QColor(200, 200, 200));
            painter->drawRect(-50, -25, 100, 50);
            painter->drawEllipse(-50, 12.5, 100, 25);
            painter->setBrush(QColor(220,220,220));
            painter->drawEllipse(-50, -37.5, 100, 25);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-3*name.length(), 10);
            painter->drawText(textPos, name);
        }
        else {
            painter->setBrush(QColor(150,150,200));
            painter->drawRect(-50,-25,100,50);
            painter->drawEllipse(-50,+12.5,100,25);
            painter->setBrush(QColor(200,200,250));
            painter->drawEllipse(-50,-37.5,100,25);
            painter->setPen(QColor(50,50,100));
            QPointF textPos(-3*name.length(),+10);
            painter->drawText(textPos, name);
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QSqlDatabase getDatabaseConnection()
    {
        return database_connection;
    }
    QString getName()
    {
        return name;
    }
    void setName(QString name)
    {
        this->name = name;
    }
    QString getHost()
    {
        return host;
    }
    void setHost(QString host)
    {
        this->host = host;
    }
    QString getPort()
    {
        return port;
    }
    void setPort(QString port)
    {
        this->port = port;
    }
    QString getUser()
    {
        return user;
    }
    void setUser(QString user)
    {
        this->user = user;
    }
    QString getPassword()
    {
        return user;
    }
    void setPassword(QString password)
    {
        this->password = password;
    }
    bool getDatabaseCollapsed()
    {
        return collapsed;
    }
    void setDatabaseCollapsed(bool collapsed)
    {
        this->collapsed = collapsed;
    }
    bool getDatabaseStatus()
    {
        return status;
    }
    void setDatabaseStatus(bool status)
    {
        this->status = status;
    }
    QList<Schema*> getSchemaList()
    {
        return schema_list;
    }
    void setSchemaList(QList<Schema*> schema_list)
    {
        this->schema_list = schema_list;
    }
    void appendSchemaList(Schema *schema)
    {
        schema_list.append(schema);
    }
    int getSchemaCount()
    {
        return number_of_schemas;
    }
    void setSchemaCount(int number_of_schemas)
    {
        this->number_of_schemas = number_of_schemas;
    }
    void appendTableName(QString table_name)
    {
        table_names_list.append(table_name);
    }
    void appendViewName(QString view_name)
    {
        view_names_list.append(view_name);
    }
    void appendFunctionName(QString function_name)
    {
        function_names_list.append(function_name);
    }
    QStringList tableNamesList()
    {
        return table_names_list;
    }
    QStringList viewNamesList()
    {
        return view_names_list;
    }
    QStringList functionNamesList()
    {
        return function_names_list;
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*);

signals:
    void expandDatabase(Database*);
    void summonPropertyDialog(Database*);
    void expandAll(Database*);
    void expandAllVertically(Database*);
    void collapseDatabase(Database*);
    void setMainWinTitle(QString);
    void collapseSchemaTables(Schema*);
    void collapseSchemaViews(Schema*);
    void collapseSchemaFunctions(Schema*);
};

#endif // DATABASE_H
