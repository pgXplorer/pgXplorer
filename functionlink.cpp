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

#include <QPainter>
#include "functionlink.h"
#include "schema.h"
#include "function.h"
#include "mainwin.h"
#include <math.h>

FunctionLink::FunctionLink(Schema *source_node, Function *destination_node)
{
    schema = source_node;
    function = destination_node;
    //schema->addEdge(this);
    //function->addEdge(this);
    setParentItem(source_node);
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setZValue(-100);
    adjust();
}

void FunctionLink::adjust()
{
    QLineF line(0, 0, function->x(), function->y());
    prepareGeometryChange();
    schema_point = line.p1();
    function_point = line.p2();
}

QRectF FunctionLink::boundingRect() const
{
    return QRectF(schema_point, QSizeF(function_point.x() - schema_point.x(),
                                      function_point.y() - schema_point.y()))
        .normalized().adjusted(-.5, -.5, .5, .5);
}

void FunctionLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QLineF line(schema_point, function_point);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
    painter->setPen(QPen(QColor(200,200,250), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);
}
