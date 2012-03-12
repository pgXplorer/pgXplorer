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
    QStringList schema_name_list;
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

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    bool setConnectionProperties(const QString, const qint32, const QString, const QString, const QString);
    void showPropertyDialog();
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
    void delDatabase(Database*);
    bool populateDatabase();
    bool advance();
    void addEdge(SchemaLink *edge);
    QRectF boundingRect() const
    {
        return QRectF(-50, -37.5, 100, 75);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget)
    {
        painter->setPen(QColor(0,0,0,0));

        if(this->getDatabaseCollapsed()) {
            painter->setBrush(QColor(200, 200, 200));
            painter->drawRect(-50, -25, 100, 50);
            painter->drawEllipse(-50, 12.5, 100, 25);
            painter->setBrush(QColor(220,220,220));
            painter->drawEllipse(-50, -37.5, 100, 25);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-3*this->name.length(), 10);
            painter->drawText(textPos, this->name);
        }
        else {
            painter->setBrush(QColor(150,150,200));
            painter->drawRect(-50,-25,100,50);
            painter->drawEllipse(-50,+12.5,100,25);
            painter->setBrush(QColor(200,200,250));
            painter->drawEllipse(-50,-37.5,100,25);
            painter->setPen(QColor(50,50,100));
            QPointF textPos(-3*this->name.length(),+10);
            painter->drawText(textPos, this->name);
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QSqlDatabase getDatabaseConnection()
    {
        return this->database_connection;
    }
    QString getName()
    {
        return this->name;
    }
    void setName(QString name)
    {
        this->name = name;
    }
    QString getHost()
    {
        return this->host;
    }
    void setHost(QString host)
    {
        this->host = host;
    }
    QString getPort()
    {
        return this->port;
    }
    void setPort(QString port)
    {
        this->port = port;
    }
    QString getUser()
    {
        return this->user;
    }
    void setUser(QString user)
    {
        this->user = user;
    }
    QString getPassword()
    {
        return this->user;
    }
    void setPassword(QString password)
    {
        this->password = password;
    }
    bool getDatabaseCollapsed()
    {
        return this->collapsed;
    }
    void setDatabaseCollapsed(bool collapsed)
    {
        this->collapsed = collapsed;
    }
    bool getDatabaseStatus()
    {
        return this->status;
    }
    void setDatabaseStatus(bool status)
    {
        this->status = status;
    }
    QList<QString> getSchemaStringList()
    {
        return this->schema_name_list;
    }
    void setSchemaList(QStringList schema_name_list)
    {
        this->schema_name_list = schema_name_list;
    }
    QList<Schema*> getSchemaList()
    {
        return this->schema_list;
    }
    void setSchemaList(QList<Schema*> schema_list)
    {
        this->schema_list = schema_list;
    }
    int getSchemaCount()
    {
        return number_of_schemas;
    }
    void setSchemaCount(int number_of_schemas)
    {
        this->number_of_schemas = number_of_schemas;
    }
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*);

Q_SIGNALS:
    void expandDatabase(Database*);
    void expandAll(Database*);
    void expandAllVertically(Database*);
    void collapseDatabase(Database*);
    void setMainWinTitle(QString);
    void collapseSchemaTables(Schema*);
    void collapseSchemaViews(Schema*);
    void collapseSchemaFunctions(Schema*);
};

#endif // DATABASE_H
