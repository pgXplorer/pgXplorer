/****************************************************************************
** Meta object code from reading C++ file 'pgxconsole.h'
**
** Created: Fri Dec 30 20:08:01 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pgxconsole.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pgxconsole.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PgxConsole[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      35,   11,   11,   11, 0x05,
      47,   11,   11,   11, 0x05,
      61,   11,   11,   11, 0x05,
      75,   73,   11,   11, 0x05,
     108,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
     139,   73,   11,   11, 0x08,
     163,   11,   11,   11, 0x08,
     192,   11,   11,   11, 0x08,
     210,   11,   11,   11, 0x08,
     229,   11,   11,   11, 0x08,
     250,   11,   11,   11, 0x08,
     271,   11,   11,   11, 0x08,
     300,   11,   11,   11, 0x08,
     316,   11,   11,   11, 0x08,
     327,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PgxConsole[] = {
    "PgxConsole\0\0commandSignal(QString)\0"
    "historyUp()\0historyDown()\0getDbPros()\0"
    ",\0showQueryView(Database*,QString)\0"
    "pgxconsoleClosing(PgxConsole*)\0"
    "updatePrompt(QRect,int)\0"
    "makePreviousBlocksReadonly()\0"
    "showView(QString)\0historyUpCommand()\0"
    "historyDownCommand()\0pasteFromClipboard()\0"
    "pasteAsSingleFromClipboard()\0"
    "toggleFindBar()\0findText()\0"
    "pgxconsoleClosing()\0"
};

void PgxConsole::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PgxConsole *_t = static_cast<PgxConsole *>(_o);
        switch (_id) {
        case 0: _t->commandSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->historyUp(); break;
        case 2: _t->historyDown(); break;
        case 3: _t->getDbPros(); break;
        case 4: _t->showQueryView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->pgxconsoleClosing((*reinterpret_cast< PgxConsole*(*)>(_a[1]))); break;
        case 6: _t->updatePrompt((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->makePreviousBlocksReadonly(); break;
        case 8: _t->showView((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->historyUpCommand(); break;
        case 10: _t->historyDownCommand(); break;
        case 11: _t->pasteFromClipboard(); break;
        case 12: _t->pasteAsSingleFromClipboard(); break;
        case 13: _t->toggleFindBar(); break;
        case 14: _t->findText(); break;
        case 15: _t->pgxconsoleClosing(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PgxConsole::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PgxConsole::staticMetaObject = {
    { &QPlainTextEdit::staticMetaObject, qt_meta_stringdata_PgxConsole,
      qt_meta_data_PgxConsole, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PgxConsole::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PgxConsole::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PgxConsole::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PgxConsole))
        return static_cast<void*>(const_cast< PgxConsole*>(this));
    return QPlainTextEdit::qt_metacast(_clname);
}

int PgxConsole::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void PgxConsole::commandSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PgxConsole::historyUp()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void PgxConsole::historyDown()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void PgxConsole::getDbPros()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void PgxConsole::showQueryView(Database * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PgxConsole::pgxconsoleClosing(PgxConsole * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
static const uint qt_meta_data_PgxConsoleMainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_PgxConsoleMainWindow[] = {
    "PgxConsoleMainWindow\0\0pgxconsoleClosing()\0"
};

void PgxConsoleMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PgxConsoleMainWindow *_t = static_cast<PgxConsoleMainWindow *>(_o);
        switch (_id) {
        case 0: _t->pgxconsoleClosing(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PgxConsoleMainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PgxConsoleMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_PgxConsoleMainWindow,
      qt_meta_data_PgxConsoleMainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PgxConsoleMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PgxConsoleMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PgxConsoleMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PgxConsoleMainWindow))
        return static_cast<void*>(const_cast< PgxConsoleMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int PgxConsoleMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void PgxConsoleMainWindow::pgxconsoleClosing()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
