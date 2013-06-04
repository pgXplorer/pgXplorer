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

#ifndef REPORTWINDOW_H
#define REPORTWINDOW_H

#include <QtGui>
#include <QtConcurrent/QtConcurrent>

#include "database.h"
#include "/home/nimbus/libharu/include/hpdf.h"

class ReportView;
class ReportWindow;
/*class TopMargin;
class BottomMargin;
class LeftMargin;
class RightMargin;

class TopMargin : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

private:
    BottomMargin *bottom_margin;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        setCursor(Qt::SizeVerCursor);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        unsetCursor();
    }

public:
    TopMargin(QGraphicsScene* scene)
    {
        scene->addItem(this);
        setPen(QPen(QBrush(QColor(Qt::gray)), 0.5, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        setFlags(flags() | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
        setAcceptsHoverEvents(true);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void setBottomMargin(BottomMargin *bottom_margin)
    {
        this->bottom_margin = bottom_margin;
    }
};

class BottomMargin : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

private:
    TopMargin *top_margin;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        setCursor(Qt::SizeVerCursor);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        unsetCursor();
    }

public:
    BottomMargin(QGraphicsScene* scene)
    {
        scene->addItem(this);
        setPen(QPen(QBrush(QColor(Qt::gray)), 0.5, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        setFlags(flags() | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
        setAcceptsHoverEvents(true);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == ItemPositionChange && scene()) {
            QRectF rect = scene()->sceneRect();
            QPointF new_pos = value.toPointF();
            new_pos.setX(0);

            qreal min_y = 0.0;
            if(top_margin)
                min_y = top_margin->pos().y() + 100.0;

            new_pos.setY(qMin(rect.bottom(), qMax(new_pos.y(), min_y)));

            return new_pos;
        }
        return QGraphicsItem::itemChange(change, value);
    }

    void setTopMargin(TopMargin *top_margin)
    {
        this->top_margin = top_margin;
    }
};

class LeftMargin : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

private:
    bool right_margin_set;
    RightMargin *right_margin;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        setCursor(Qt::SizeHorCursor);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        unsetCursor();
    }

public:
    LeftMargin(QGraphicsScene* scene)
    {
        right_margin_set = false;
        scene->addItem(this);
        setPen(QPen(QBrush(QColor(Qt::gray)), 0.5, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        setFlags(flags() | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
        setAcceptsHoverEvents(true);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void setRightMargin(RightMargin *right_margin)
    {
        this->right_margin = right_margin;
        right_margin_set = true;
    }
};

class RightMargin : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

private:
    LeftMargin *left_margin;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        setCursor(Qt::SizeHorCursor);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        unsetCursor();
    }

public:
    RightMargin(QGraphicsScene* scene)
    {
        scene->addItem(this);
        setPen(QPen(QBrush(QColor(Qt::gray)), 0.5, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        setFlags(flags() | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
        setAcceptsHoverEvents(true);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == ItemPositionChange && scene()) {
            QRectF rect = scene()->sceneRect();
            QPointF new_pos = value.toPointF();
            new_pos.setY(0);

            qreal min_x = 0.0;
            if(left_margin)
                min_x = left_margin->pos().x() + 100;

            new_pos.setX(qMin(rect.right(), qMax(new_pos.x(), min_x)));

            return new_pos;
        }
        return QGraphicsItem::itemChange(change, value);
    }

    void setLeftMargin(LeftMargin *left_margin)
    {
        this->left_margin = left_margin;
    }
};*/

class ReportView : public QGraphicsView
{
    Q_OBJECT

private:
    ReportWindow *report_window;
    QDrag *drag;
    QMimeData *mime_data;

protected:
    void drawBackground(QPainter *painter, const QRectF &rect)
    {
        painter->save();

        painter->setPen(Qt::lightGray);
        painter->setBrush(Qt::lightGray);
        painter->drawRect(rect);

        QRectF scene_rect = sceneRect();

        painter->setPen(Qt::white);
        painter->setBrush(Qt::white);
        painter->drawRect(scene_rect);

        painter->restore();
    }

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void wheelEvent(QWheelEvent *event);

public:
    ReportView(QGraphicsScene&, ReportWindow *parent=0,
              const char *name=0, Qt::WindowFlags f=0);
    void setReportWindow(ReportWindow *report_window)
    {
        this->report_window = report_window;
    }

public slots:
    void changeCursor(int);

signals:
    void droppedLabel(QPointF);
    void droppedDatabox(QPointF);
    void droppedTable(QPointF);
    void droppedHLine(QPointF);
};

class ReportLabel : public QGraphicsTextItem
{
    Q_OBJECT

private:
    bool hovered;
    QPointF hover_spot;
    bool repeat_every_page;

protected:
    //void hoverMoveEvent(QGraphicsSceneHoverEvent *);
    //void hoverLeaveEvent(QGraphicsSceneHoverEvent *);

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        if(textInteractionFlags() == Qt::NoTextInteraction) {
            setFocus(Qt::TabFocusReason);
            setSelected(true);
            setTextInteractionFlags(Qt::TextEditorInteraction);
        }// else {
            QGraphicsTextItem::mouseDoubleClickEvent(event);
        //}
    }

    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        QFont f = font();
        if(event->delta() > 0)
            f.setPointSize(font().pointSize()+1);
        else
            f.setPointSize(font().pointSize()-1);
        setFont(f);
    }

    void focusOutEvent(QFocusEvent *event)
    {
        setSelected(false);
        QTextCursor tc = textCursor();
        tc.clearSelection();
        setTextCursor(tc);
        setTextInteractionFlags(Qt::NoTextInteraction);
        setEnabled(true);
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
        QGraphicsTextItem::focusOutEvent(event);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        QMenu context_menu;
        if(repeat_every_page)
            context_menu.addAction(QIcon(":/icons/ok.png"), tr("Repeat on every page"));
        else
            context_menu.addAction(tr("Repeat on every page"));
        context_menu.addSeparator();
        context_menu.addAction(tr("Change font..."));
        context_menu.addSeparator();
        context_menu.addAction(tr("Set colour..."));
        context_menu.addSeparator();
        context_menu.addAction(tr("Delete this"));

        QAction *a = context_menu.exec(event->screenPos());

        if(a && QString::compare(a->text(), tr("Repeat on every page")) == 0) {
            repeat_every_page = !repeat_every_page;
        }
        else if(a && QString::compare(a->text(), tr("Change font...")) == 0) {
            bool ok;
            QFont f = QFontDialog::getFont(&ok, font(), 0);
            setFont(f);
        }
        else if(a && QString::compare(a->text(), tr("Set colour...")) == 0) {
            QColor c = QColorDialog::getColor(defaultTextColor(), (QWidget*) this->parentWidget());
            if(c.isValid())
                setDefaultTextColor(c);
        }
        else if(a && QString::compare(a->text(), tr("Delete this")) == 0) {
            emit deletingLabel(this);
            deleteLater();
        }
    }

public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->setPen(Qt::darkGray);
        painter->drawRect(boundingRect());
        QGraphicsTextItem::paint(painter, option, widget);
    }

    ReportLabel()
    {
        repeat_every_page = true;
        setPlainText(tr("Label"));
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFocus(Qt::ActiveWindowFocusReason);
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::Document);
        setTextCursor(tc);
        setPos(70, 70);
        prepareGeometryChange();
    }

    ReportLabel(QPointF pos)
    {
        repeat_every_page = true;
        setPlainText(tr("Label"));
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFocus(Qt::ActiveWindowFocusReason);
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::Document);
        setTextCursor(tc);
        setPos(pos);
        prepareGeometryChange();
    }

    bool repeatOnEveryPage()
    {
        return repeat_every_page;
    }

    ~ReportLabel(){}

signals:
    void deletingLabel(ReportLabel *);
};

class ReportTable : public QGraphicsObject
{
    Q_OBJECT

private:
    int rows;
    QStringList column_list;
    QBitArray enabled_cols;
    QList<QColor> column_color_list;
    bool repeat_header_every_page;

    QFont f;

    QPointF top_left;
    QVector<int> width;
    int height;
    qreal x_mouse_pressed;
    int x_width;
    int col_to_be_moved;
    qreal y_mouse_pressed;
    int y_height;

    bool hovered;
    QPointF hover_spot;
    QPointF drag_spot;

    bool x_isResizing;
    bool y_isResizing;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);

    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        prepareGeometryChange();
        //Capture wheel rolls and increase/decrease
        //font size and table height appropriately.
        if(event->delta() > 0)
            f.setPointSize(font().pointSize()+1);
        else
            f.setPointSize(font().pointSize()-1);

        height = f.pointSize()*2;

        //Repaint the scene to avoid ugly artifacts
        //especially when the table height is reduced.
        //scene()->update();
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

public:
    ReportTable()
    {
        setAcceptDrops(true);
        setAcceptHoverEvents(true);
        top_left.setX(70);
        top_left.setY(70);
        height = f.pointSize()*2;
        rows = 1;
        column_list = QStringList();
        enabled_cols = QBitArray();
        setCacheMode(DeviceCoordinateCache);
        prepareGeometryChange();
    }

    ReportTable(QPointF pos, QStringList column_list)
    {
        repeat_header_every_page = true;
        setAcceptDrops(true);
        setAcceptHoverEvents(true);
        top_left = mapToScene(pos.toPoint());
        width.resize(column_list.size());
        width.fill(90);
        height = f.pointSize()*2;
        rows = 1;
        this->column_list = column_list;
        enabled_cols = QBitArray(column_list.size(), true);
        for(int col = 0; col < enabled_cols.size(); col++)
            column_color_list.append(Qt::black);
        x_isResizing = false;
        x_mouse_pressed = 0.0;
        col_to_be_moved = 0;
        y_isResizing = false;
        y_mouse_pressed = 0.0;
        setCacheMode(DeviceCoordinateCache);
        prepareGeometryChange();
    }

    ~ReportTable(){}

    QFont font()
    {
        return f;
    }

    void setFont(QFont f)
    {
        this->f = f;
    }

    int getHeight()
    {
        return height;
    }

    int getWidth(int i)
    {
        return width.at(i);
    }

    int totalWidth() const
    {
        int columns = enabled_cols.count();
        int w = 0;
        for(int col=0; col<columns; col++) {
            if(enabled_cols.at(col))
                 w += width.at(col);
        }
        return w;
    }

    int widthTill(int column) const
    {
        int w = 0;
        for(int col=0; col<column; col++) {
            if(enabled_cols.at(col))
                 w += width.at(col);
        }
        return w;
    }

    void setHeight(int height)
    {
        this->height = height;
    }

    void setWidth(int i, int width)
    {
        this->width.replace(i, width);
    }

    QPointF getTopLeft()
    {
        return top_left;
    }

    void disableColumnAt(QPointF pos)
    {
        QPointF relpos = mapFromScene(pos);
        int columns = enabled_cols.count();
        for(int col=0; col<columns; col++) {
            if(enabled_cols.at(col)) {
                QRectF rect = QRectF(top_left.x() + widthTill(col), top_left.y(), width.at(col), height);
                if(rect.contains(relpos)) {
                    enabled_cols.clearBit(col);
                    break;
                }
            }
        }
    }

    void setTextColor(QPointF pos, QColor color)
    {
        QPointF relpos = mapFromScene(pos);
        int columns = enabled_cols.count();
        for(int col=0; col<columns; col++) {
            if(enabled_cols.at(col)) {
                QRectF rect = QRectF(top_left.x() + widthTill(col), top_left.y(), width.at(col), height);
                if(rect.contains(relpos)) {
                    column_color_list.replace(col, color);
                    break;
                }
            }
        }
    }

    QColor textColor(int col) const
    {
        return column_color_list.at(col);
    }

    bool repeatOnEveryPage()
    {
        return repeat_header_every_page;
    }

    bool getColumnEnabled(int i)
    {
        if(i < enabled_cols.size())
            return enabled_cols.at(i);
        else
            return false;
    }

    bool onCol(qreal);

    QRectF boundingRect() const
    {
        return QRectF(top_left.x(), top_left.y(), totalWidth(), height).adjusted(-3, -3, 3, 3);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        int columns = enabled_cols.count();

        if(isSelected()) {
            painter->setPen(Qt::DashLine);
            painter->drawRect(boundingRect());
            update();
        }

        for(int col=0; col<columns; col++) {
            if(enabled_cols.at(col)) {
                QRectF rect = QRectF(top_left.x() + widthTill(col), top_left.y(), width.at(col), height);
                if(hovered && rect.contains(hover_spot))
                    painter->setBrush(Qt::lightGray);
                else
                    painter->setBrush(QBrush());
                painter->setPen(QPen());
                painter->drawRect(rect);

                painter->setFont(f);
                painter->setPen(column_color_list.at(col));
                painter->drawText(rect.adjusted(1,0,0,0), Qt::AlignCenter | Qt::AlignVCenter, column_list.at(col));
            }
        }
        painter->setPen(QPen());
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

signals:
    void deletingTable(ReportTable*);
};

class ReportWindow : public QMainWindow
{
    Q_OBJECT

private:
    Database *database;
    QString sql;
    QStringList column_list;
    QButtonGroup *button_group;
    QGraphicsScene scene;
    ReportView *report_view;
    ToolBar *toolbar;

    QProgressDialog *progress_dialog;

    //LeftMargin *left_margin;
    //RightMargin *right_margin;
    //TopMargin *top_margin;
    //BottomMargin *bottom_margin;

    QAction *pdf_print_action;
    QAction *html_print_action;
    QAction *odf_print_action;
    bool print_cancelled;

    QList<ReportTable*> table_list;
    QList<QGraphicsLineItem*> hline_list;
    QList<ReportLabel*> label_list;

    //void initMargins();

    void drawPDFLine(HPDF_Page page, float x, float y);

protected:
    void closeEvent(QCloseEvent *);

signals:
    void done();

public slots:
    void restore();
    void drawLabel(QPointF);
    void drawDatabox(QPointF);
    void drawTable(QPointF);
    void drawHLine(QPointF);
    void pdfPrintThread();
    void htmlPrintThread();
    void odfPrintThread();
    void pdfPrint(QString file_name);
    void htmlPrint(QString file_name);
    void odfPrint(QString file_name);
    void printingCancelled();
    void deleteSeletectedItems();
    void labelDeleted(ReportLabel *);
    void tableDeleted(ReportTable *);
    void selectAll();
    void noZoom();

public:
    ReportWindow(Database *, QString);
    void createActions();

    ToolBar* getToolbar()
    {
        return toolbar;
    }
};

#endif // REPORTWINDOW_H
