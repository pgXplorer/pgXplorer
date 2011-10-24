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
    static QString hostS;
    static int portS;
    static QString dbnameS;
    static QString userS;
    static QString passwordS;

public slots:
    void help();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void newView();
    void clear();
    void openFile();
    void open(QString);
    void saveFile();
    void saveFileAs();
    void save(QString);
    int saveState(QString);
    void newFile();
    void console();
    void fullscreen();
    void restore();
    void addSchema(Database*);
    void delSchema(Database*);
    void addTable(Database*, Schema*);
    void delTable(Database*, Schema*);
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
    Database* db;
    QMenuBar* menubar;
    QMenu* fileMenu;
    QMenu* toolMenu;
    QMenu* viewMenu;
    QMenu* helpMenu;
    /*QAction *newDb;
    QAction *openDb;
    QAction *savDb;
    QAction *exit;
    QAction *console;
    QAction *tools;*/
    void createActions();
    void createMenus();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * );
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * );
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * );
    void wheelEvent(QWheelEvent *);
    void readSettings();
    void writeSettings();
    QGraphicsScene& scn;
    QRubberBand* rubberBand;
    QPoint origin;
    FigureEditor* edt;
    QMenu* opt;
    QPrinter* prn;
    int dbf_id;
};

class LaunchTable : public QThread
{
public:
     void showTbl(Database*, Schema*, Table*);
     void runQry(QString);
};

#endif
