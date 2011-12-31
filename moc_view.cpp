/****************************************************************************
** Meta object code from reading C++ file 'view.h'
**
** Created: Fri Dec 30 20:08:10 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "view.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'view.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_View[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
       9,    6,    5,    5, 0x05,
      45,    6,    5,    5, 0x05,
      80,    6,    5,    5, 0x05,
     114,    6,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
     152,    5,    5,    5, 0x0a,
     175,    5,    5,    5, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_View[] = {
    "View\0\0,,\0expandView(Database*,Schema*,View*)\0"
    "clearView(Database*,Schema*,View*)\0"
    "dropView(Database*,Schema*,View*)\0"
    "collapseView(Database*,Schema*,View*)\0"
    "getSearchTerm(QString)\0verticalPosition2()\0"
};

void View::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        View *_t = static_cast<View *>(_o);
        switch (_id) {
        case 0: _t->expandView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 1: _t->clearView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 2: _t->dropView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 3: _t->collapseView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< View*(*)>(_a[3]))); break;
        case 4: _t->getSearchTerm((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->verticalPosition2(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData View::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject View::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_View,
      qt_meta_data_View, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &View::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *View::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *View::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_View))
        return static_cast<void*>(const_cast< View*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< View*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< View*>(this));
    return QObject::qt_metacast(_clname);
}

int View::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void View::expandView(Database * _t1, Schema * _t2, View * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void View::clearView(Database * _t1, Schema * _t2, View * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void View::dropView(Database * _t1, Schema * _t2, View * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void View::collapseView(Database * _t1, Schema * _t2, View * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
