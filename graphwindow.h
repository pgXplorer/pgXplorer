#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QtGui>

#define XRANGE 640.0
#define YRANGE 480.0
#define MARGIN 20.0

class GraphView;
class GraphWindow;

class GraphView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphView(QGraphicsScene&, QWidget *parent=0,
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
    void mouseReleaseEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::RightButton) {
            fitInView(scene()->selectionArea().boundingRect(), Qt::KeepAspectRatio);
            scene()->setSelectionArea(QPainterPath());
        }
        QGraphicsView::mouseReleaseEvent(event);
    }

signals:
    void status(const QString&);
    void clicked();

private:
    bool zoom;
    GraphWindow *graph_window;
    QPoint origin;
};

class GraphWindow : public QMainWindow
{
    Q_OBJECT

private:
    QGraphicsScene scene;
    GraphView *graph_view;

public:
    enum PlotType {None, Scatter, Line, Bar};
    GraphWindow(QModelIndexList list, PlotType pt=None, QWidget *parent=0, const QString arg1=QLatin1String(""), Qt::WindowFlags f=0);
    ~GraphWindow(){}
};

#endif // GRAPHWINDOW_H
