/****************************************************************************
** Meta object code from reading C++ file 'tableview.h'
**
** Created: Fri Dec 30 20:08:04 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tableview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tableview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TableView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      24,   10,   10,   10, 0x05,
      49,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      78,   10,   10,   10, 0x08,
      97,   10,   10,   10, 0x08,
     116,   10,   10,   10, 0x08,
     132,   10,   10,   10, 0x08,
     140,   10,   10,   10, 0x08,
     149,   10,   10,   10, 0x08,
     163,   10,   10,   10, 0x08,
     177,   10,   10,   10, 0x08,
     186,   10,   10,   10, 0x08,
     202,   10,   10,   10, 0x08,
     212,   10,   10,   10, 0x08,
     221,   10,   10,   10, 0x08,
     231,   10,   10,   10, 0x08,
     250,   10,   10,   10, 0x08,
     270,   10,   10,   10, 0x08,
     286,   10,   10,   10, 0x08,
     314,   10,   10,   10, 0x08,
     326,   10,   10,   10, 0x08,
     342,   10,   10,   10, 0x08,
     353,   10,   10,   10, 0x08,
     376,   10,   10,   10, 0x08,
     389,   10,   10,   10, 0x08,
     399,   10,   10,   10, 0x08,
     415,   10,   10,   10, 0x08,
     431,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TableView[] = {
    "TableView\0\0busySignal()\0"
    "updRowCntSignal(QString)\0"
    "tableViewClosing(TableView*)\0"
    "fetchDefaultData()\0fetchRefreshData()\0"
    "fetchDataSlot()\0copyc()\0copych()\0"
    "defaultView()\0refreshView()\0filter()\0"
    "filter(QString)\0exclude()\0ascend()\0"
    "descend()\0removeAllFilters()\0"
    "removeAllOrdering()\0removeColumns()\0"
    "customFilterReturnPressed()\0copyQuery()\0"
    "truncateTable()\0busySlot()\0"
    "updRowCntSlot(QString)\0fullscreen()\0"
    "restore()\0toggleActions()\0enableActions()\0"
    "disableActions()\0"
};

void TableView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TableView *_t = static_cast<TableView *>(_o);
        switch (_id) {
        case 0: _t->busySignal(); break;
        case 1: _t->updRowCntSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->tableViewClosing((*reinterpret_cast< TableView*(*)>(_a[1]))); break;
        case 3: _t->fetchDefaultData(); break;
        case 4: _t->fetchRefreshData(); break;
        case 5: _t->fetchDataSlot(); break;
        case 6: _t->copyc(); break;
        case 7: _t->copych(); break;
        case 8: _t->defaultView(); break;
        case 9: _t->refreshView(); break;
        case 10: _t->filter(); break;
        case 11: _t->filter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 12: _t->exclude(); break;
        case 13: _t->ascend(); break;
        case 14: _t->descend(); break;
        case 15: _t->removeAllFilters(); break;
        case 16: _t->removeAllOrdering(); break;
        case 17: _t->removeColumns(); break;
        case 18: _t->customFilterReturnPressed(); break;
        case 19: _t->copyQuery(); break;
        case 20: _t->truncateTable(); break;
        case 21: _t->busySlot(); break;
        case 22: _t->updRowCntSlot((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 23: _t->fullscreen(); break;
        case 24: _t->restore(); break;
        case 25: _t->toggleActions(); break;
        case 26: _t->enableActions(); break;
        case 27: _t->disableActions(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TableView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TableView::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_TableView,
      qt_meta_data_TableView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TableView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TableView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TableView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TableView))
        return static_cast<void*>(const_cast< TableView*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int TableView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    return _id;
}

// SIGNAL 0
void TableView::busySignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void TableView::updRowCntSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TableView::tableViewClosing(TableView * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
