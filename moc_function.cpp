/****************************************************************************
** Meta object code from reading C++ file 'function.h'
**
** Created: Fri Dec 30 20:08:09 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "function.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'function.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Function[] = {

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
      12,   10,    9,    9, 0x05,
      49,   46,    9,    9, 0x05,
      95,   46,    9,    9, 0x05,
     136,   46,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     178,    9,    9,    9, 0x0a,
     201,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Function[] = {
    "Function\0\0,\0expandFunction(Schema*,Function*)\0"
    ",,\0collapseFunction(Database*,Schema*,Function*)\0"
    "runFunction(Database*,Schema*,Function*)\0"
    "dropFunction(Database*,Schema*,Function*)\0"
    "getSearchTerm(QString)\0verticalPosition2()\0"
};

void Function::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Function *_t = static_cast<Function *>(_o);
        switch (_id) {
        case 0: _t->expandFunction((*reinterpret_cast< Schema*(*)>(_a[1])),(*reinterpret_cast< Function*(*)>(_a[2]))); break;
        case 1: _t->collapseFunction((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Function*(*)>(_a[3]))); break;
        case 2: _t->runFunction((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Function*(*)>(_a[3]))); break;
        case 3: _t->dropFunction((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Function*(*)>(_a[3]))); break;
        case 4: _t->getSearchTerm((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->verticalPosition2(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Function::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Function::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Function,
      qt_meta_data_Function, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Function::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Function::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Function::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Function))
        return static_cast<void*>(const_cast< Function*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Function*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Function*>(this));
    return QObject::qt_metacast(_clname);
}

int Function::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Function::expandFunction(Schema * _t1, Function * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Function::collapseFunction(Database * _t1, Schema * _t2, Function * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Function::runFunction(Database * _t1, Schema * _t2, Function * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Function::dropFunction(Database * _t1, Schema * _t2, Function * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
