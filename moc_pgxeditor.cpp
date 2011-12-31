/****************************************************************************
** Meta object code from reading C++ file 'pgxeditor.h'
**
** Created: Fri Dec 30 20:08:07 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pgxeditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pgxeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PgxEditor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   11,   10,   10, 0x05,
      46,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      89,   75,   10,   10, 0x08,
     120,   11,   10,   10, 0x08,
     152,   10,   10,   10, 0x08,
     175,   10,   10,   10, 0x08,
     196,   10,   10,   10, 0x08,
     211,   10,   10,   10, 0x08,
     225,   10,   10,   10, 0x08,
     243,   10,   10,   10, 0x08,
     266,   10,   10,   10, 0x08,
     284,   10,   10,   10, 0x08,
     300,   10,   10,   10, 0x08,
     311,   10,   10,   10, 0x08,
     325,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PgxEditor[] = {
    "PgxEditor\0\0,\0showQueryView(Database*,QString)\0"
    "pgxeditorClosing(PgxEditor*)\0newBlockCount\0"
    "updateLineNumberAreaWidth(int)\0"
    "updateLineNumberArea(QRect,int)\0"
    "highlightCurrentLine()\0removeHighlighting()\0"
    "saveFunction()\0executeText()\0"
    "executeFunction()\0selectionChangedSlot()\0"
    "textChangedSlot()\0toggleFindBar()\0"
    "findText()\0replaceText()\0pgxeditorClosing()\0"
};

void PgxEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PgxEditor *_t = static_cast<PgxEditor *>(_o);
        switch (_id) {
        case 0: _t->showQueryView((*reinterpret_cast< Database*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->pgxeditorClosing((*reinterpret_cast< PgxEditor*(*)>(_a[1]))); break;
        case 2: _t->updateLineNumberAreaWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->updateLineNumberArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->highlightCurrentLine(); break;
        case 5: _t->removeHighlighting(); break;
        case 6: _t->saveFunction(); break;
        case 7: _t->executeText(); break;
        case 8: _t->executeFunction(); break;
        case 9: _t->selectionChangedSlot(); break;
        case 10: _t->textChangedSlot(); break;
        case 11: _t->toggleFindBar(); break;
        case 12: _t->findText(); break;
        case 13: _t->replaceText(); break;
        case 14: _t->pgxeditorClosing(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PgxEditor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PgxEditor::staticMetaObject = {
    { &QPlainTextEdit::staticMetaObject, qt_meta_stringdata_PgxEditor,
      qt_meta_data_PgxEditor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PgxEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PgxEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PgxEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PgxEditor))
        return static_cast<void*>(const_cast< PgxEditor*>(this));
    return QPlainTextEdit::qt_metacast(_clname);
}

int PgxEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void PgxEditor::showQueryView(Database * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PgxEditor::pgxeditorClosing(PgxEditor * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_PgxEditorMainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_PgxEditorMainWindow[] = {
    "PgxEditorMainWindow\0\0pgxeditorClosing()\0"
};

void PgxEditorMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PgxEditorMainWindow *_t = static_cast<PgxEditorMainWindow *>(_o);
        switch (_id) {
        case 0: _t->pgxeditorClosing(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PgxEditorMainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PgxEditorMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_PgxEditorMainWindow,
      qt_meta_data_PgxEditorMainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PgxEditorMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PgxEditorMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PgxEditorMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PgxEditorMainWindow))
        return static_cast<void*>(const_cast< PgxEditorMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int PgxEditorMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void PgxEditorMainWindow::pgxeditorClosing()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
