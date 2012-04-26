
#include "graphwindow.h"

GraphView::GraphView(
        QGraphicsScene &s, QWidget *parent,
        const char *name, Qt::WindowFlags f) :
    QGraphicsView(&s, parent)
{
    setMouseTracking(false);
    setAcceptDrops(true);
    zoom = false;
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

GraphWindow::GraphWindow(QModelIndexList list, PlotType pt, QWidget *parent, const QString arg1, Qt::WindowFlags f)
{
    graph_view = new GraphView(scene, this);
    graph_view->setSceneRect(QRectF(0.0, 00.0, XRANGE + 2*MARGIN, YRANGE + 2*MARGIN));
    graph_view->setDragMode(QGraphicsView::RubberBandDrag);
    setCentralWidget(graph_view);
    QVector<double> x;
    QVector<double> y;
    double xmin=list.at(0).data().toDouble(), xmax=list.at(0).data().toDouble();
    double ymin=list.at(1).data().toDouble(), ymax=list.at(1).data().toDouble();
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
    switch(pt) {
    case GraphWindow::Scatter :
        for(int i = 0; i < x.size(); i++) {
            QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(MARGIN + x.at(i)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i)*YRANGE/ymax-5, 10, 10, 0);
            circle->setBrush(QColor(150,150,187));
            circle->setPen(QPen(Qt::white));
            circle->setToolTip(QString::number(x.at(i)).append(",").append(QString::number(y.at(i))));
            scene.addItem(circle);
        }
        break;
    case GraphWindow::Line :
        for(int i = 1; i < x.size(); i++) {
            QGraphicsLineItem *line = new QGraphicsLineItem(MARGIN + x.at(i-1)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i-1)*YRANGE/ymax-5, MARGIN + x.at(i)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i)*YRANGE/ymax-5, 0);
            line->setPen(QPen(QColor(150,150,187)));
            scene.addItem(line);
        }
        break;
    case GraphWindow::Bar :
        for(int i = 0; i < x.size(); i++) {
            QGraphicsRectItem *rect = new QGraphicsRectItem(MARGIN + x.at(i)*XRANGE/xmax-5, YRANGE + MARGIN - y.at(i)*YRANGE/ymax, 10, y.at(i)*YRANGE/ymax, 0);
            rect->setBrush(QColor(150,150,187));
            rect->setPen(QPen(Qt::white));
            scene.addItem(rect);
        }
        break;
    default:
        break;
    }

    scene.addLine(MARGIN, YRANGE + MARGIN, XRANGE + MARGIN, YRANGE + MARGIN);
    scene.addLine(MARGIN, MARGIN, MARGIN, YRANGE + MARGIN);
    scene.addLine(XRANGE + MARGIN, YRANGE + MARGIN, XRANGE + MARGIN, MARGIN);
    scene.addLine(MARGIN, MARGIN, XRANGE + MARGIN, MARGIN);

    QGraphicsTextItem *xxx = scene.addText(QString::number(xmax), QFont("Helvetica"));
    xxx->setPos(XRANGE - MARGIN, YRANGE + MARGIN);

    QGraphicsTextItem *yyy = scene.addText(QString::number(ymax), QFont("Helvetica"));
    yyy->setPos(0, 0);

    QGraphicsTextItem *origin = scene.addText("0, 0", QFont("Helvetica"));
    origin->setPos(0, YRANGE + MARGIN);
}
