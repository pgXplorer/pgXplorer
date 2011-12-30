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

#include "mainWin.h"
#include "database.h"
#include "view.h"
#include "schema.h"

View::View(Database *database, Schema *schema, QString view_name, int view_index, QColor color)
{
    this->database = database;
    this->setParent(schema);
    this->setParentItem(schema);
    this->view_index = view_index;
    this->setName(view_name);
    this->setStatus(false);
    this->setCollapsed(true);
    //setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-10);
    setAcceptHoverEvents(true);
}

void View::defaultPosition()
{
    float xs = parent_schema->x();
    float ys = parent_schema->y();
    float i;
    int view_count = parent_schema->getViewCount();
    if(view_count%2 == 0) {
        if (xs < 0)
            i = -view_index+(view_count/2)-0.5;
        else
            i = view_index-(view_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -view_index+(view_count/2);
        else
            i = view_index-(view_count/2);
    }
    int radius = 8*view_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/view_count;
    if(xs < 0)
    {
        this->setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        this->setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void View::verticalPosition()
{
    float xs = parent_schema->x();
    float ys = 0;
    float i;
    int view_count = parent_schema->getViewCount();
    if(view_count%2 == 0) {
        if (xs < 0)
            i = -view_index+(view_count/2)-0.5;
        else
            i = view_index-(view_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -view_index+(view_count/2);
        else
            i = view_index-(view_count/2);
    }
    int radius = 8*view_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/view_count;
    if(xs < 0)
    {
        this->setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        this->setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void View::verticalPosition2()
{
    setPos(0,50*(view_index+1));
}

void View::setColumnList()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT column_name, data_type FROM information_schema.columns WHERE table_schema='" + parent_schema->getName() + "' AND table_name='" + view_name + "'";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid())
    {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema tables.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    while (column_query.next())
        column_list.append("\"" + column_query.value(0).toString() + "\"");
}

void View::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    //menu.setStyleSheet("QMenu { font-size:12px; width: 100px; color:white; left: 20px; background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}");
    menu.addAction(QIcon("icons/view2.svgz"), tr("View contents"));
    if(!is_view) {
        menu.addSeparator();
        menu.addAction("Drop view");
    }
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),tr("View contents")) == 0)
    {
        emit expandView(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Drop view")) == 0)
    {
        emit dropView(database, parent_schema, this);
    }
}

void View::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    setColumnList();
    emit expandView(database, parent_schema, this);
    update();
}

void View::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if(searched)
        setToolTip(parent_schema->getName() + "." + getName());
    else
        toolTip().clear();
}

void View::getSearchTerm(QString search_term)
{
    if(view_name.contains(search_term))
        setSearched(true);
    else
        setSearched(false);
    update();
}

bool View::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}
