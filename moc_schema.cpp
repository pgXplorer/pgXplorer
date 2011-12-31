/****************************************************************************
** Meta object code from reading C++ file 'schema.h'
**
** Created: Fri Dec 30 20:07:56 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "schema.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'schema.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Schema[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,
      36,    7,    7,    7, 0x05,
      66,    7,    7,    7, 0x05,
      93,    7,    7,    7, 0x05,
     122,    7,    7,    7, 0x05,
     152,    7,    7,    7, 0x05,
     183,    7,    7,    7, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Schema[] = {
    "Schema\0\0expandSchemaTables(Schema*)\0"
    "collapseSchemaTables(Schema*)\0"
    "expandSchemaViews(Schema*)\0"
    "collapseSchemaViews(Schema*)\0"
    "collapseOtherSchemas(Schema*)\0"
    "expandSchemaFunctions(Schema*)\0"
    "collapseSchemaFunctions(Schema*)\0"
};

void Schema::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Schema *_t = static_cast<Schema *>(_o);
        switch (_id) {
        case 0: _t->expandSchemaTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 1: _t->collapseSchemaTables((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 2: _t->expandSchemaViews((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 3: _t->collapseSchemaViews((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 4: _t->collapseOtherSchemas((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 5: _t->expandSchemaFunctions((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        case 6: _t->collapseSchemaFunctions((*reinterpret_cast< Schema*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Schema::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Schema::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Schema,
      qt_meta_data_Schema, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Schema::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Schema::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Schema::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Schema))
        return static_cast<void*>(const_cast< Schema*>(this));
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Schema*>(this));
    if (!strcmp(_clname, "com.trolltech.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(const_cast< Schema*>(this));
    return QObject::qt_metacast(_clname);
}

int Schema::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void Schema::expandSchemaTables(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Schema::collapseSchemaTables(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Schema::expandSchemaViews(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Schema::collapseSchemaViews(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Schema::collapseOtherSchemas(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Schema::expandSchemaFunctions(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Schema::collapseSchemaFunctions(Schema * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_END_MOC_NAMESPACE
