/****************************************************************************
** Meta object code from reading C++ file 'licensedialog.h'
**
** Created: Fri Dec 30 20:08:13 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "licensedialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'licensedialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LicenseDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      24,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_LicenseDialog[] = {
    "LicenseDialog\0\0okslot()\0cancel()\0"
};

void LicenseDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LicenseDialog *_t = static_cast<LicenseDialog *>(_o);
        switch (_id) {
        case 0: _t->okslot(); break;
        case 1: _t->cancel(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData LicenseDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LicenseDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_LicenseDialog,
      qt_meta_data_LicenseDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LicenseDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LicenseDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LicenseDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LicenseDialog))
        return static_cast<void*>(const_cast< LicenseDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int LicenseDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
