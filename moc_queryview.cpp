/****************************************************************************
** Meta object code from reading C++ file 'queryview.h'
**
** Created: Fri Dec 30 20:08:02 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "queryview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'queryview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QueryView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      24,   10,   10,   10, 0x05,
      51,   49,   10,   10, 0x05,
      73,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     102,   10,   10,   10, 0x08,
     110,   10,   10,   10, 0x08,
     119,   10,   10,   10, 0x08,
     130,   10,   10,   10, 0x08,
     153,   10,   10,   10, 0x08,
     166,   10,   10,   10, 0x08,
     176,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QueryView[] = {
    "QueryView\0\0busySignal()\0"
    "updRowCntSignal(QString)\0,\0"
    "errMesg(QString,uint)\0"
    "queryViewClosing(QueryView*)\0copyc()\0"
    "copych()\0busySlot()\0updRowCntSlot(QString)\0"
    "fullscreen()\0restore()\0fetchData(QString)\0"
};

void QueryView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QueryView *_t = static_cast<QueryView *>(_o);
        switch (_id) {
        case 0: _t->busySignal(); break;
        case 1: _t->updRowCntSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->errMesg((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 3: _t->queryViewClosing((*reinterpret_cast< QueryView*(*)>(_a[1]))); break;
        case 4: _t->copyc(); break;
        case 5: _t->copych(); break;
        case 6: _t->busySlot(); break;
        case 7: _t->updRowCntSlot((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->fullscreen(); break;
        case 9: _t->restore(); break;
        case 10: _t->fetchData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QueryView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QueryView::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_QueryView,
      qt_meta_data_QueryView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QueryView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QueryView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QueryView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QueryView))
        return static_cast<void*>(const_cast< QueryView*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int QueryView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void QueryView::busySignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QueryView::updRowCntSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QueryView::errMesg(QString _t1, uint _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QueryView::queryViewClosing(QueryView * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
