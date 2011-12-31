/****************************************************************************
** Meta object code from reading C++ file 'database.h'
**
** Created: Fri Dec 30 20:07:54 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "database.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'database.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Database[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,
      36,    9,    9,    9, 0x05,
      57,    9,    9,    9, 0x05,
      88,    9,    9,    9, 0x05,
     116,    9,    9,    9, 0x05,
     141,    9,    9,    9, 0x05,
     171,    9,    9,    9, 0x05,
     200,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     243,  238,  233,    9, 0x0a,
     307,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Database[] = {
    "Database\0\0expandDatabase(Database*)\0"
    "expandAll(Database*)\0"
    "expandAllVertically(Database*)\0"
    "collapseDatabase(Database*)\0"
    "setMainWinTitle(QString)\0"
    "collapseSchemaTables(Schema*)\0"
    "collapseSchemaViews(Schema*)\0"
    "collapseSchemaFunctions(Schema*)\0bool\0"
    ",,,,\0"
    "setConnectionProperties(QString,qint32,QString,QString,QString)\0"
    "showPropertyDialog()\0"
};

void Database::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Database *_t = static_cast<Database *>(_o);
        switch (_id) {
        case 0: _t->expandDatabase((*reinterpret_cast< Database*(*)>(_a[1]))); break;
        case 1: _t->expandAll((*reinterpret_cast< Database*(*)>(_a[1]))); break;
        case 2: _t->expandAllVertically((*reinterpret_cast< Database*(*)>(_a[1]))); break;
        case 3: _t->collapseDatabase((*reinterpret_cast< Database*(*)>(_a[1]))); break;
        case 4: _t->setMainWinTitle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->collapseSchemaTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 6: _t->collapseSchemaViews((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 7: _t->collapseSchemaFunctions((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 8: { bool _r = _t->setConnectionProperties((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const qint32(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 9: _t->showPropertyDialog(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Database::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Database::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Database,
      qt_meta_data_Database, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Database::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Database::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Database::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Database))
        return static_cast<void*>(const_cast< Database*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Database*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Database*>(this));
    return QObject::qt_metacast(_clname);
}

int Database::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Database::expandDatabase(Database * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Database::expandAll(Database * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Database::expandAllVertically(Database * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Database::collapseDatabase(Database * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Database::setMainWinTitle(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Database::collapseSchemaTables(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Database::collapseSchemaViews(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Database::collapseSchemaFunctions(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_END_MOC_NAMESPACE
