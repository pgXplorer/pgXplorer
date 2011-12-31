/****************************************************************************
** Meta object code from reading C++ file 'querymodel.h'
**
** Created: Fri Dec 30 20:08:14 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "querymodel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'querymodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QueryModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   12,   11,   11, 0x05,
      63,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QueryModel[] = {
    "QueryModel\0\0,,,\0"
    "fetchDataSignal(QueryModel*,int,qint32,qint32)\0"
    "busySignal()\0destroyQueryModel()\0"
};

void QueryModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QueryModel *_t = static_cast<QueryModel *>(_o);
        switch (_id) {
        case 0: _t->fetchDataSignal((*reinterpret_cast< QueryModel*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< qint32(*)>(_a[3])),(*reinterpret_cast< qint32(*)>(_a[4]))); break;
        case 1: _t->busySignal(); break;
        case 2: _t->destroyQueryModel(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QueryModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QueryModel::staticMetaObject = {
    { &QSqlQueryModel::staticMetaObject, qt_meta_stringdata_QueryModel,
      qt_meta_data_QueryModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QueryModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QueryModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QueryModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QueryModel))
        return static_cast<void*>(const_cast< QueryModel*>(this));
    return QSqlQueryModel::qt_metacast(_clname);
}

int QueryModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSqlQueryModel::qt_metacall(_c, _id, _a);
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
void QueryModel::fetchDataSignal(QueryModel * _t1, int _t2, qint32 _t3, qint32 _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QueryModel::busySignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
