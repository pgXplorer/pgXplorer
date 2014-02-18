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

#include "mainwin.h"
#include "database.h"
#include "function.h"
#include "schema.h"

Function::Function(Database *database, Schema *schema, QString function_name, QString function_args, QString function_arg_types, int function_index, QColor color)
{
    this->database = database;
    setParent(schema);
    setParentItem(schema);
    this->function_index = function_index;
    setName(function_name);
    ascii_length = function_name.toLatin1().length();
    utf8_length = function_name.toUtf8().length();
    setArgs(function_args);
    setArgTypes(function_arg_types);
    setStatus(false);
    setCollapsed(true);
    //setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-10);
    setAcceptHoverEvents(true);
}

void Function::defaultPosition()
{
    float xs = parent_schema->x();
    float ys = parent_schema->y();
    float i;
    int function_count = parent_schema->getFunctionCount();
    if(function_count%2 == 0) {
        if (xs < 0)
            i = -function_index+(function_count/2)-0.5;
        else
            i = function_index-(function_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -function_index+(function_count/2);
        else
            i = function_index-(function_count/2);
    }
    int radius = 8*function_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/function_count;
    if(xs < 0)
    {
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
               (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
               radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Function::verticalPosition()
{
    float xs = parent_schema->x();
    float ys = 0;
    float i;
    int function_count = parent_schema->getFunctionCount();
    if(function_count%2 == 0) {
        if (xs < 0)
            i = -function_index+(function_count/2)-0.5;
        else
            i = function_index-(function_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -function_index+(function_count/2);
        else
            i = function_index-(function_count/2);
    }
    int radius = 8*function_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/function_count;
    if(xs < 0)
    {
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
               (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
               radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Function::verticalPosition2()
{
    setPos(0,50*(function_index+1));
}

void Function::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(QIcon(":/icons/function.svg"), tr("Function definition"));
    menu.addSeparator();
    menu.addAction(tr("Drop function"));

    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),tr("Function definition")) == 0)
    {
        emit expandFunction(parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Drop function")) == 0)
    {
        emit dropFunction(database, parent_schema, this);
    }
}

void Function::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit expandFunction(parent_schema, this);
}

void Function::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if(searched)
        setToolTip(parent_schema->getName() + "." + getName() + "\n"
                   + getArgTypesToText());
    else
        toolTip().clear();
}

void Function::getSearchTerm(QString search_term)
{
    if(function_name.contains(search_term))
        setSearched(true);
    else
        setSearched(false);
    update();
}

bool Function::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

QString Function::getArgsToText()
{
    return function_args.replace(" ", ", ");
}

QString Function::getArgTypesToText()
{
    QStringList args = function_arg_types.split(" ");
    QStringList args_text;
    foreach(QString oid, args)
        args_text.append(database->getTypeText(oid.toInt()));
    return args_text.join(", ").prepend("(").append(")");
}

/*
QVariant Function::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (FunctionLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    };
    return QGraphicsItem::itemChange(change, value);
}

void Function::addEdge(FunctionLink *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<FunctionLink *> Function::edges() const
{
    return edgeList;
}
*/
