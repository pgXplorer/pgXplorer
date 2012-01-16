/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github>

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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGui>
#include "database.h"

#define FETCHSIZ 10000

class TableView : public QMainWindow
{
    Q_OBJECT

private:
    Database *database;
    QMenuBar *menubar;
    QString status_message;
    QMenu context_menu;
    QMenu deselect_menu;
    QMenu disarrange_menu;
    QTime t;
    QToolBar *toolbar;
    QSqlQueryModel *query_model;
    QTableView *tview;
    QString sql;
    QStringList where_clause;
    QStringList order_clause;
    QStringList group_clause;
    QString limit;
    QStringList offset_list;
    bool quick_fetch;
    bool can_fetch_more;
    qint32 rows_from;
    qint32 rows_to;
    qint32 column_count;
    bool thread_busy;
    ulong thisTableViewId;
    QIcon filter_icon;
    QIcon ascend_icon;
    QIcon descend_icon;
    QAction *default_action;
    QAction *refresh_action;
    QAction *copy_action;
    QAction *copy_with_headers_action;
    QAction *remove_columns_action;
    QAction *filter_action;
    QAction *exclude_action;
    QAction *ascend_action;
    QAction *descend_action;
    QAction *remove_all_filters_action;
    QAction *remove_all_ordering_action;
    QAction *truncate_action;
    QAction *delete_rows_action;
    QWidgetAction *custom_filter_action;
    QAction *copy_query_action;
    QLineEdit *filter_text;

public:
    static ulong tableViewObjectId;
    TableView(Database *, QString const, QString const, QStringList const, bool, Qt::WidgetAttribute f);
    ~TableView()
    {
    };

    bool eventFilter(QObject *, QEvent*);
    void keyReleaseEvent(QKeyEvent*);
    //void contextMenuEvent(QContextMenuEvent*);
    void closeEvent(QCloseEvent*);
    void createActions();
    void fetchNextData();
    void fetchPreviousData();
    void fetchConditionDataInitial();
    void deleteData();

public slots:
    void languageChanged(QEvent*);
    void customContextMenuViewport();
    void customContextMenuHeader();

private slots:
    void fetchDefaultData();
    void fetchRefreshData();
    void fetchDataSlot();
    void copyc();
    void copych();
    void defaultView();
    void refreshView();
    void filter();
    void filter(QString);
    void exclude();
    void ascend();
    void descend();
    void removeAllFilters();
    void removeAllOrdering();
    void removeColumns();
    void customFilterReturnPressed();
    void copyQuery();
    void truncateTable();
    void deleteRows();
    void deleteRow(int);

    void busySlot();
    void updRowCntSlot(QString);
    void fullscreen();
    void restore();
    void toggleActions();
    void enableActions();
    void disableActions();

Q_SIGNALS:
    void busySignal();
    void updRowCntSignal(QString);
    void tableViewClosing(TableView*);
};

#endif // TABLEVIEW_H
