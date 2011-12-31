/****************************************************************************
** Meta object code from reading C++ file 'mainWin.h'
**
** Created: Fri Dec 30 20:07:59 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainWin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainWin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Canvas[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,
      24,    7,    7,    7, 0x05,
      34,    7,    7,    7, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Canvas[] = {
    "Canvas\0\0status(QString)\0clicked()\0"
    "search()\0"
};

void Canvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Canvas *_t = static_cast<Canvas *>(_o);
        switch (_id) {
        case 0: _t->status((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->clicked(); break;
        case 2: _t->search(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Canvas::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Canvas::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_Canvas,
      qt_meta_data_Canvas, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Canvas::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Canvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Canvas::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Canvas))
        return static_cast<void*>(const_cast< Canvas*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int Canvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Canvas::status(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Canvas::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void Canvas::search()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
static const uint qt_meta_data_MainWin[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      59,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x05,
      19,    8,    8,    8, 0x05,
      29,    8,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,    8,    8,    8, 0x0a,
      54,    8,    8,    8, 0x0a,
      73,    8,    8,    8, 0x0a,
     101,    8,    8,    8, 0x0a,
     129,    8,    8,    8, 0x0a,
     159,    8,    8,    8, 0x0a,
     187,    8,    8,    8, 0x0a,
     211,  206,  201,    8, 0x0a,
     263,    8,    8,    8, 0x08,
     273,    8,    8,    8, 0x08,
     281,    8,    8,    8, 0x08,
     292,    8,    8,    8, 0x08,
     306,    8,    8,    8, 0x08,
     317,    8,    8,    8, 0x08,
     331,    8,    8,    8, 0x08,
     344,    8,    8,    8, 0x08,
     354,    8,    8,    8, 0x08,
     364,    8,    8,    8, 0x08,
     377,    8,    8,    8, 0x08,
     402,    8,    8,    8, 0x08,
     419,    8,    8,    8, 0x08,
     437,  435,    8,    8, 0x08,
     475,    8,    8,    8, 0x08,
     494,    8,    8,    8, 0x08,
     509,    8,    8,    8, 0x08,
     518,    8,    8,    8, 0x08,
     539,    8,    8,    8, 0x08,
     561,    8,    8,    8, 0x08,
     581,    8,    8,    8, 0x08,
     591,    8,    8,    8, 0x08,
     605,    8,    8,    8, 0x08,
     629,    8,    8,    8, 0x08,
     663,    8,    8,    8, 0x08,
     677,    8,    8,    8, 0x08,
     697,    8,    8,    8, 0x08,
     713,    8,    8,    8, 0x08,
     733,    8,    8,    8, 0x08,
     749,    8,    8,    8, 0x08,
     768,    8,    8,    8, 0x08,
     783,    8,    8,    8, 0x08,
     802,    8,    8,    8, 0x08,
     817,    8,    8,    8, 0x08,
     840,    8,    8,    8, 0x08,
     859,    8,    8,    8, 0x08,
     882,    8,    8,    8, 0x08,
     901,    8,    8,    8, 0x08,
     929,  926,    8,    8, 0x08,
     969,  926,    8,    8, 0x08,
    1010,  926,    8,    8, 0x08,
    1050,  926,    8,    8, 0x08,
    1088,  926,    8,    8, 0x08,
    1126,  435,    8,    8, 0x08,
    1159,    8,    8,    8, 0x08,
    1169,    8,    8,    8, 0x08,
    1185,    8,    8,    8, 0x08,
    1202,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWin[] = {
    "MainWin\0\0clicked()\0closing()\0"
    "showColumnView()\0about()\0document_changed()\0"
    "tableViewClosed(TableView*)\0"
    "queryViewClosed(QueryView*)\0"
    "pgxconsoleClosed(PgxConsole*)\0"
    "pgxeditorClosed(PgxEditor*)\0newDatabase()\0"
    "bool\0,,,,\0newDatabase(QString,qint32,QString,QString,QString)\0"
    "newView()\0clear()\0openFile()\0open(QString)\0"
    "saveFile()\0save(QString)\0saveFileAs()\0"
    "quitApp()\0newFile()\0newProcess()\0"
    "openDatabaseProperties()\0showPgxconsole()\0"
    "showPgxeditor()\0,\0"
    "showFunctionEditor(Schema*,Function*)\0"
    "toggleFullscreen()\0showTreeview()\0"
    "search()\0setLanguageDefault()\0"
    "setLanguageJapanese()\0setLanguageFrench()\0"
    "restore()\0showSchemas()\0explodeAndShowSchemas()\0"
    "explodeAndShowSchemasVertically()\0"
    "hideSchemas()\0showTables(Schema*)\0"
    "showAllTables()\0hideTables(Schema*)\0"
    "hideAllTables()\0showViews(Schema*)\0"
    "showAllViews()\0hideViews(Schema*)\0"
    "hideAllViews()\0showFunctions(Schema*)\0"
    "showAllFunctions()\0hideFunctions(Schema*)\0"
    "hideAllFunctions()\0hideOtherTables(Schema*)\0"
    ",,\0showTableView(Database*,Schema*,Table*)\0"
    "clearTableView(Database*,Schema*,Table*)\0"
    "dropTableView(Database*,Schema*,Table*)\0"
    "showViewView(Database*,Schema*,View*)\0"
    "dropViewView(Database*,Schema*,View*)\0"
    "showQueryView(Database*,QString)\0"
    "fitView()\0zoomIn(QPointF)\0zoomOut(QPointF)\0"
    "noZoom()\0"
};

void MainWin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWin *_t = static_cast<MainWin *>(_o);
        switch (_id) {
        case 0: _t->clicked(); break;
        case 1: _t->closing(); break;
        case 2: _t->showColumnView(); break;
        case 3: _t->about(); break;
        case 4: _t->document_changed(); break;
        case 5: _t->tableViewClosed((*reinterpret_cast< TableView*(*)>(_a[1]))); break;
        case 6: _t->queryViewClosed((*reinterpret_cast< QueryView*(*)>(_a[1]))); break;
        case 7: _t->pgxconsoleClosed((*reinterpret_cast< PgxConsole*(*)>(_a[1]))); break;
        case 8: _t->pgxeditorClosed((*reinterpret_cast< PgxEditor*(*)>(_a[1]))); break;
        case 9: _t->newDatabase(); break;
        case 10: { bool _r = _t->newDatabase((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< qint32(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 11: _t->newView(); break;
        case 12: _t->clear(); break;
        case 13: _t->openFile(); break;
        case 14: _t->open((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 15: _t->saveFile(); break;
        case 16: _t->save((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 17: _t->saveFileAs(); break;
        case 18: _t->quitApp(); break;
        case 19: _t->newFile(); break;
        case 20: _t->newProcess(); break;
        case 21: _t->openDatabaseProperties(); break;
        case 22: _t->showPgxconsole(); break;
        case 23: _t->showPgxeditor(); break;
        case 24: _t->showFunctionEditor((*reinterpret_cast< Schema*(*)>(_a[1])),(*reinterpret_cast< Function*(*)>(_a[2]))); break;
        case 25: _t->toggleFullscreen(); break;
        case 26: _t->showTreeview(); break;
        case 27: _t->search(); break;
        case 28: _t->setLanguageDefault(); break;
        case 29: _t->setLanguageJapanese(); break;
        case 30: _t->setLanguageFrench(); break;
        case 31: _t->restore(); break;
        case 32: _t->showSchemas(); break;
        case 33: _t->explodeAndShowSchemas(); break;
        case 34: _t->explodeAndShowSchemasVertically(); break;
        case 35: _t->hideSchemas(); break;
        case 36: _t->showTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 37: _t->showAllTables(); break;
        case 38: _t->hideTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 39: _t->hideAllTables(); break;
        case 40: _t->showViews((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 41: _t->showAllViews(); break;
        case 42: _t->hideViews((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 43: _t->hideAllViews(); break;
        case 44: _t->showFunctions((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 45: _t->showAllFunctions(); break;
        case 46: _t->hideFunctions((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 47: _t->hideAllFunctions(); break;
        case 48: _t->hideOtherTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 49: _t->showTableView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 50: _t->clearTableView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 51: _t->dropTableView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 52: _t->showViewView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 53: _t->dropViewView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 54: _t->showQueryView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 55: _t->fitView(); break;
        case 56: _t->zoomIn((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 57: _t->zoomOut((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 58: _t->noZoom(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWin::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWin::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWin,
      qt_meta_data_MainWin, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWin))
        return static_cast<void*>(const_cast< MainWin*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 59)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 59;
    }
    return _id;
}

// SIGNAL 0
void MainWin::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MainWin::closing()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void MainWin::showColumnView()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
