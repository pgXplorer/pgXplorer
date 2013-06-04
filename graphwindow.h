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

#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QtGui>
#include <QtWidgets>

#define XRANGE 640.0
#define YRANGE 480.0
#define MARGIN 40.0

class GraphView;
class GraphWindow;

class GraphView : public QGraphicsView
{
    Q_OBJECT

private:
    bool zoom;
    GraphWindow *graph_window;
    QPoint origin;

public:
    GraphView(QGraphicsScene&, GraphWindow *parent=0,
              const char *name=0, Qt::WindowFlags f=0);
    void clear();
    bool isZoom()
    {
        return zoom;
    }
    void setZoom(bool zoom)
    {
        this->zoom = zoom;
    }
    void setGraphWindow(GraphWindow *graph_window)
    {
        this->graph_window = graph_window;
    }

protected:
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void status(const QString&);
    void clicked();

};

class GraphWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(PlotType plot_type READ plotType WRITE setPlotType)

public:
    enum PlotType {None, Scatter, Line, Bar, Area};
    void setPlotType(PlotType plot_type);
    PlotType plotType() const;

    GraphWindow(QModelIndexList list, PlotType pt=None, QWidget *parent=0, const QString arg1=QLatin1String(""), Qt::WindowFlags f=0);
    ~GraphWindow(){}
    int extractData(QModelIndexList);
    void addPlot(PlotType);
    void addXAxis();
    void addYAxis();
    void addGrid();
    void addXLables();
    void addYLables();
    QList<QGraphicsEllipseItem*> dataPoints()
    {
        return data_points;
    }

private slots:
    void zoomIn();
    void zoomOut();
    void setupMatrix();

private:
    QFont x_lable_font;
    QFont y_lable_font;
    QGraphicsTextItem *max_x;
    QGraphicsTextItem *origin_x;
    QGraphicsTextItem *max_y;
    QGraphicsTextItem *origin_y;
    QSlider *zoom_slider;
    QGraphicsScene scene;
    PlotType plot_type;
    GraphView *graph_view;
    QList<QGraphicsEllipseItem*> data_points;

    QVector<double> x;
    QVector<double> y;

    double xmin, xmax;
    double ymin, ymax;

    int number_of_plots;
    QStringList color_names;

    QGraphicsLineItem *x_axis;
    QGraphicsLineItem *y_axis;
};

#endif // GRAPHWINDOW_H
