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

#include "graphwindow.h"

GraphView::GraphView(QGraphicsScene &s, GraphWindow *parent,
                     const char *name, Qt::WindowFlags f) :
    QGraphicsView(&s, parent)
{
    this->graph_window = parent;
    setMouseTracking(false);
    setAcceptDrops(true);
    zoom = false;
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void GraphView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        fitInView(scene()->selectionArea().boundingRect(), Qt::KeepAspectRatio);
        scene()->setSelectionArea(QPainterPath());
    }
    QGraphicsView::mouseReleaseEvent(event);
}

GraphWindow::GraphWindow(QModelIndexList list, PlotType plot_type, QWidget *parent, const QString arg1, Qt::WindowFlags f)
{
    zoom_slider = new QSlider;
    zoom_slider->setMinimum(0);
    zoom_slider->setMaximum(500);
    zoom_slider->setValue(250);
    zoom_slider->setTickPosition(QSlider::TicksRight);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/icons/zoom-in.png"));
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/icons/zoom-out.png"));

    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoom_slider);
    zoomSliderLayout->addWidget(zoomOutIcon);

    number_of_plots = 0;
    color_names = QColor::colorNames();
    color_names.removeOne("black");
    color_names.removeOne("transparent");
    foreach(QString color_name, color_names) {
        QColor c(color_name);
        if(c.red()>200 || c.green()>200 || c.blue()>200)
            color_names.removeOne(color_name);
    }

    x_lable_font = QFont("Helvetica");
    y_lable_font = QFont("Helvetica");

    graph_view = new GraphView(scene, this);
    graph_view->setSceneRect(QRectF(0.0, 00.0, XRANGE + 2*MARGIN, YRANGE + 2*MARGIN));
    graph_view->setDragMode(QGraphicsView::RubberBandDrag);

    QWidget *main = new QWidget;

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(graph_view);
    labelLayout->addLayout(zoomSliderLayout);

    main->setLayout(labelLayout);

    connect(zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

    setCentralWidget(main);

    if(extractData(list)) {
        QMessageBox::warning(this, tr(""), tr(""));
    }

    addPlot(plot_type);

    addXAxis();
    addYAxis();
    addGrid();

    addXLables();
    addYLables();
}

void GraphWindow::zoomIn()
{
    zoom_slider->setValue(zoom_slider->value() + 1);
}

void GraphWindow::zoomOut()
{
    zoom_slider->setValue(zoom_slider->value() - 1);
}

void GraphWindow::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoom_slider->value() - 250) / qreal(50));
    QMatrix matrix;
    matrix.scale(scale, scale);
    graph_view->setMatrix(matrix);
}

int GraphWindow::extractData(QModelIndexList list)
{
    if(list.isEmpty())
        return 1;

    xmin = list.at(0).data().toDouble(), xmax = list.at(0).data().toDouble();
    ymin = list.at(1).data().toDouble(), ymax = list.at(1).data().toDouble();

    bool even = true;
    foreach(QModelIndex index, list) {
        if(even) {
            x.append(index.data().toDouble());
            if(index.data().toInt() > xmax)
                xmax = index.data().toDouble();
            if(index.data().toInt() < xmin)
                xmin = index.data().toDouble();
            even = false;
            continue;
        }
        else {
            y.append(index.data().toDouble());
            if(index.data().toReal() > ymax)
                ymax = index.data().toDouble();
            if(index.data().toDouble() < ymin)
                ymin = index.data().toDouble();
            even = true;
        }
    }
    if(x.size() != y.size())
        return 2;
    return 0;
}

void GraphWindow::addPlot(PlotType plot_type)
{
    switch(plot_type) {
    case GraphWindow::Scatter : {
        setPlotType(plot_type);
        for(int i = 0; i < x.size(); i++) {
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(MARGIN + x.at(i)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i)*YRANGE/ymax-5, 10, 10, 0);
            circle->setBrush(QColor(150,150,187));
            //circle->setBrush(QColor(color_names.at(number_of_plots%color_names.size())));
            circle->setPen(QPen(Qt::white));
            circle->setToolTip(QString::number(x.at(i)).append(",").append(QString::number(y.at(i))));
            scene.addItem(circle);
            data_points.append(circle);
        }
    } break;
    case GraphWindow::Line : {
        setPlotType(plot_type);
        for(int i = 1; i < x.size(); i++) {
            QGraphicsLineItem *line = new QGraphicsLineItem(MARGIN + x.at(i-1)*XRANGE/xmax, YRANGE + MARGIN - y.at(i-1)*YRANGE/ymax, MARGIN + x.at(i)*XRANGE/xmax, YRANGE + MARGIN - y.at(i)*YRANGE/ymax, 0);
            line->setPen(QPen(QColor(150,150,187)));
            //line->setPen(QPen(QColor(color_names.at(number_of_plots%color_names.size()))));
            scene.addItem(line);

            QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(MARGIN + x.at(i)*XRANGE/xmax-2, YRANGE + MARGIN - y.at(i)*YRANGE/ymax-2, 4, 4, 0);
            dot->setBrush(QColor(150,150,187));
            //dot->setBrush(QColor(color_names.at(number_of_plots%color_names.size())));
            dot->setPen(QPen(Qt::white));
            dot->setToolTip(QString::number(x.at(i)).append(", ").append(QString::number(y.at(i))));
            scene.addItem(dot);
        }
        QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(MARGIN + x.at(0)*XRANGE/xmax-2, YRANGE + MARGIN - y.at(0)*YRANGE/ymax-2, 4, 4, 0);
        dot->setBrush(QColor(150,150,187));
        //dot->setBrush(QColor(color_names.at(number_of_plots%color_names.size())));
        dot->setPen(QPen(Qt::white));
        dot->setToolTip(QString::number(x.at(0)).append(", ").append(QString::number(y.at(0))));
        scene.addItem(dot);
    } break;
    case GraphWindow::Bar : {
        setPlotType(plot_type);
        for(int i = 0; i < x.size(); i++) {
            QGraphicsRectItem *rect = new QGraphicsRectItem(MARGIN + x.at(i)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i)*YRANGE/ymax, 10, y.at(i)*YRANGE/ymax, 0);
            rect->setBrush(QColor(150,150,187));
            //rect->setBrush(QColor(color_names.at(number_of_plots%color_names.size())));
            rect->setPen(QPen(Qt::white));
            rect->setToolTip(QString::number(x.at(i)).append(", ").append(QString::number(y.at(i))));
            scene.addItem(rect);
        }
    } break;
    case GraphWindow::Area : {
        setPlotType(plot_type);
        for(int i = 0; i < x.size(); i++) {
            QPolygonF polygon_points;
            polygon_points << QPointF(MARGIN + x.at(i-1)*XRANGE/xmax, YRANGE + MARGIN)
                           << QPointF(MARGIN + x.at(i-1)*XRANGE/xmax, YRANGE + MARGIN - y.at(i-1)*YRANGE/ymax)
                           << QPointF(MARGIN + x.at(i)*XRANGE/xmax, YRANGE + MARGIN - y.at(i)*YRANGE/ymax)
                           << QPointF(MARGIN + x.at(i)*XRANGE/xmax, YRANGE + MARGIN);
            QGraphicsPolygonItem *polygon = new QGraphicsPolygonItem(polygon_points);
            polygon->setBrush(QColor(150,150,187));
            //polygon->setBrush(QColor(color_names.at(number_of_plots%color_names.size())));
            polygon->setPen(QPen(QColor(200,200,237)));
            //polygon->setPen(QPen(QColor(color_names.at(number_of_plots%color_names.size())).lighter()));
            polygon->setToolTip(QString::number(x.at(i)).append(", ").append(QString::number(y.at(i))));
            scene.addItem(polygon);
        }
    } break;
    default:
        break;
    }
    number_of_plots++;
}

void GraphWindow::addXAxis()
{
    x_axis = scene.addLine(MARGIN, YRANGE + MARGIN, XRANGE + MARGIN, YRANGE + MARGIN);
    x_axis->setPen(QPen(Qt::darkGray));
    scene.addLine(MARGIN, MARGIN, MARGIN, YRANGE + MARGIN)->setPen(QPen(Qt::darkGray));
}

void GraphWindow::addYAxis()
{
    y_axis = scene.addLine(MARGIN, MARGIN, XRANGE + MARGIN, MARGIN);
    y_axis->setPen(QPen(Qt::darkGray));
    scene.addLine(XRANGE + MARGIN, YRANGE + MARGIN, XRANGE + MARGIN, MARGIN)->setPen(QPen(Qt::darkGray));
}

void GraphWindow::addGrid()
{
    for(int i=1; i < XRANGE/MARGIN; i++)
        scene.addLine(MARGIN+i*MARGIN, MARGIN, MARGIN+i*MARGIN, YRANGE + MARGIN)->setPen(QPen(Qt::lightGray));
    for(int i=2; i <= YRANGE/MARGIN; i++)
        scene.addLine(MARGIN, i*MARGIN, XRANGE + MARGIN, i*MARGIN)->setPen(QPen(Qt::lightGray));
}

void GraphWindow::addXLables()
{
    origin_x = scene.addText("0", x_lable_font);
    origin_x->setPos(MARGIN - origin_x->boundingRect().width()/2, YRANGE + MARGIN);

    max_x = scene.addText(QString::number(xmax), x_lable_font);
    max_x->setPos(XRANGE + MARGIN -max_x->boundingRect().width()/2, YRANGE + MARGIN);
}

void GraphWindow::addYLables()
{
    origin_y = scene.addText("0", y_lable_font);
    origin_y->setPos(-origin_y->boundingRect().width()+MARGIN, YRANGE+MARGIN-origin_y->boundingRect().height()/2);

    max_y = scene.addText(QString::number(ymax), y_lable_font);
    max_y->setPos(-max_y->boundingRect().width()+MARGIN, -max_y->boundingRect().height()/2+MARGIN);
}

GraphWindow::PlotType GraphWindow::plotType() const
{
    return plot_type;
}

void GraphWindow::setPlotType(PlotType plot_type)
{
    this->plot_type = plot_type;
}
