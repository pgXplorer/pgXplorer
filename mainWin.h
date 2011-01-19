#ifndef PGXPLORER
#define PGXPLORER

#include <QMouseEvent>
#include <QtGui>

/*!
  @mainpage PgXplorer
  @section intro_sec Introduction
  pgXplorer is a freely downloadable, BSD licensed
  administration interface for PostgreSQL family of databases.
  Features include a table viewer and the Pgx SQL console (PgxConsole).

  @todo
        -# 'bcrypt'ed password save.
        -# SSL support.
        -# Linux support.
 */

class SchemaLink;
class TableLink;
class Database;
class Schema;
class Table;

class FigureEditor : public QGraphicsView {
    Q_OBJECT

public:
    FigureEditor(QGraphicsScene&, QWidget* parent=0,
                 const char* name=0, Qt::WindowFlags f=0);
    void clear();
    //virtual void mousePressEvent(QGraphicsSceneMouseEvent * )
    //{
    //    emit clicked();
    //}
    void wheelEvent(QWheelEvent *event);

signals:
    void status(const QString&);
    void clicked();
};

class MainWin : public QMainWindow {
    Q_OBJECT

public:
    MainWin(QGraphicsScene&, QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0);
    ~MainWin();
    QGraphicsScene& getScn()
    {
        return scn;
    }

public slots:
    void help();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void newView();
    void clear();
    void openDbFile();
    void open(QString);
    void saveDbFile();
    void save(QString);
    void newDb();
    void console();
    void addSchema(Database*);
    void delSchema(Database*);
    void addTable(Schema*);
    void delTable(Schema*);
    void enlarge();
    void shrink();
    void zoomIn();
    void zoomOut();
    void default1();
    void moveL();
    void moveR();
    void moveU();
    void moveD();

Q_SIGNALS:
    void clicked();

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * );
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * );
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * );
    void wheelEvent(QWheelEvent *);
    void readSettings();
    void writeSettings();
    QGraphicsScene& scn;
    QRubberBand* rubberBand;
    QPoint origin;
    FigureEditor *edt;
    QMenu* opt;
    QPrinter* prn;
    int dbf_id;
};

class LaunchTable : public QThread
{
public:
     void showTbl(Schema*, Table*);
     void runQry(QString);
};

#endif
