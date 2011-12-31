/****************************************************************************
** Meta object code from reading C++ file 'table.h'
**
** Created: Fri Dec 30 20:07:58 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "table.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'table.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Table[] = {

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
      10,    7,    6,    6, 0x05,
      48,    7,    6,    6, 0x05,
      85,    7,    6,    6, 0x05,
     121,    7,    6,    6, 0x05,

 // slots: signature, parameters, type, tag, flags
     161,    6,    6,    6, 0x0a,
     184,    6,    6,    6, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Table[] = {
    "Table\0\0,,\0expandTable(Database*,Schema*,Table*)\0"
    "clearTable(Database*,Schema*,Table*)\0"
    "dropTable(Database*,Schema*,Table*)\0"
    "collapseTable(Database*,Schema*,Table*)\0"
    "getSearchTerm(QString)\0verticalPosition2()\0"
};

void Table::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Table *_t = static_cast<Table *>(_o);
        switch (_id) {
        case 0: _t->expandTable((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 1: _t->clearTable((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 2: _t->dropTable((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 3: _t->collapseTable((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< Schema*(*)>(_a[2])),(*reinterpret_cast< Table*(*)>(_a[3]))); break;
        case 4: _t->getSearchTerm((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->verticalPosition2(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Table::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Table::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Table,
      qt_meta_data_Table, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Table::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Table::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Table::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Table))
        return static_cast<void*>(const_cast< Table*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Table*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Table*>(this));
    return QObject::qt_metacast(_clname);
}

int Table::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Table::expandTable(Database * _t1, Schema * _t2, Table * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Table::clearTable(Database * _t1, Schema * _t2, Table * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Table::dropTable(Database * _t1, Schema * _t2, Table * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Table::collapseTable(Database * _t1, Schema * _t2, Table * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
