#ifndef DATABASE_H
#define DATABASE_H

#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>
#include "schemaLink.h"

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
    QList<QString> schList;
    QString name;
    QString host;
    QString port;
    QString user;
    QString password;
    bool status;
    bool collapsed;
    QList<SchemaLink *> edgeList;
    QPointF newPos;
    QStringList history;
    qint32 hit;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    bool setConnProps(const QString, const qint32, const QString, const QString, const QString);

public:
    Database();
    ~Database(){};
    void delDatabase(Database*);
    bool populate();
    bool advance();
    void addEdge(SchemaLink *edge);
    QRectF boundingRect() const
    {
        return QRectF(-50, -37.5, 100, 75);
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget)
    {
        painter->setPen(QColor(0,0,0,0));

        if(this->getCollapsed()) {
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
    bool getCollapsed()
    {
        return this->collapsed;
    }
    void setCollapsed(bool collapsed)
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
    QList<QString> getSchList()
    {
        return this->schList;
    }
    void setSchList(QList<QString> schList)
    {
        this->schList = schList;
    }
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    virtual void contextMenuEvent ( QGraphicsSceneContextMenuEvent *);

Q_SIGNALS:
    void expand(Database*);
    void collapse(Database*);
};

#endif // DATABASE_H
