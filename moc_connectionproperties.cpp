/****************************************************************************
** Meta object code from reading C++ file 'connectionproperties.h'
**
** Created: Fri Dec 30 20:08:06 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "connectionproperties.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'connectionproperties.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ConnectionProperties[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   22,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ConnectionProperties[] = {
    "ConnectionProperties\0\0,,,,\0"
    "oksignal(QString,qint32,QString,QString,QString)\0"
    "okslot()\0"
};

void ConnectionProperties::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ConnectionProperties *_t = static_cast<ConnectionProperties *>(_o);
        switch (_id) {
        case 0: _t->oksignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< qint32(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5]))); break;
        case 1: _t->okslot(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ConnectionProperties::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ConnectionProperties::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ConnectionProperties,
      qt_meta_data_ConnectionProperties, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ConnectionProperties::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ConnectionProperties::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ConnectionProperties::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConnectionProperties))
        return static_cast<void*>(const_cast< ConnectionProperties*>(this));
    return QDialog::qt_metacast(_clname);
}

int ConnectionProperties::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ConnectionProperties::oksignal(QString _t1, qint32 _t2, QString _t3, QString _t4, QString _t5)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
