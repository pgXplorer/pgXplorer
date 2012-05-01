/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef DESIGNVIEW_H
#define DESIGNVIEW_H

#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlError>
#include <QtGui>
#include "database.h"

class DesignView;

class DesignView : public QMainWindow
{
    Q_OBJECT

private:
    Database *database;
    Table *table;
    QString table_name;
    QStringList primary_key;
    QStringList new_primary_key;
    QStringList column_list;
    QStringList new_column_list;
    QStringList column_types;
    QStringList new_column_types;
    QStringList column_lengths;
    QStringList column_nulls;
    QStringList new_column_nulls;
    QMenuBar *menubar;
    QString status_message;
    QMenu context_menu;
    QMenu deselect_menu;
    QMenu disarrange_menu;
    QTime t;
    QToolBar *toolbar;
    QTableView *design_view;
    QStandardItemModel *design_model;
    QString sql;
    QString properties;
    bool thread_busy;
    ulong thisDesignViewId;
    QAction *default_action;
    QAction *copy_action;
    QAction *copy_with_headers_action;
    QAction *remove_columns_action;
    QAction *copy_query_action;
    QBrush red_brush;
    QBrush green_brush;
    bool error_status;
    QMessageBox *error_message_box;
    QAction *save_action;
    QAction *properties_action;
    QAction *delete_column_action;

    struct Properties2 {
        bool oid;
        QString inherits;
        QString tablespace;
        int fill_factor;

        Properties2() : oid(false), inherits(QString()), tablespace(QString()), fill_factor(100){}
    };

    Properties2 properties2;

public:
    static ulong designViewObjectId;
    DesignView(Database *, Table *table, QString const, QString const, QStringList const, QStringList const, QStringList const, QStringList const, QStringList const, bool, Qt::WidgetAttribute f);
    ~DesignView(){}

    void createBrushes();
    void createActions();
    QString tableName()
    {
        return table_name;
    }
    void initialiseModel();

protected:
    void closeEvent(QCloseEvent*);

public slots:
    void bringOnTop();
    void languageChanged(QEvent*);
    void showTableProperties();
    void setProperties2(bool, QString, QString, int);

private slots:
    void updateDesigner(QModelIndex, QModelIndex);
    void updateSelectionChanged();
    void saveTable();
    void deleteColumns();

signals:
    void busySignal();
    void notBusySignal();
    void queryFailed(QString);
    void changeLanguage(QEvent*);
    void designViewClosing(DesignView*);
};

#endif // DESIGNVIEW_H
