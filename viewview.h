/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <dj@pgxplorer.com>

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

#ifndef VIEWVIEW_H
#define VIEWVIEW_H

#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlError>
#include <QtGui>
#include "database.h"
#include "querymodel.h"

#define FETCHSIZ 10000

class ViewView : public QMainWindow
{
    Q_OBJECT

private:
    QString time_elapsed_string;
    QString rows_string;
    QString rows_string_2;
    QString colums_string;
    QString seconds_string;

    double time_elapsed;

    Database *database;
    QString view_name;
    QStringList column_list;
    QStringList column_types;
    QStringList column_lengths;
    QMenuBar *menubar;
    QString status_message;
    QMenu context_menu;
    QMenu deselect_menu;
    QMenu disarrange_menu;
    QTime t;
    ToolBar *toolbar;
    QueryModel *query_model;
    QTableView *view_view;
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
    ulong thisViewViewId;
    QIcon key_icon;
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
    QAction *previous_set_action;
    QAction *next_set_action;
    QWidgetAction *custom_filter_action;
    QAction *copy_query_action;
    QLineEdit *filter_text;
    QToolButton *previous_set_button;
    QToolButton *next_set_button;
    QMessageBox *error_message_box;

public:
    static ulong viewViewObjectId;
    ViewView(Database *, QString const, QString const, QStringList const, QStringList const, bool, Qt::WidgetAttribute f);
    ~ViewView()
    {
    };

    bool eventFilter(QObject *, QEvent *);
    void keyReleaseEvent(QKeyEvent *);
    //void contextMenuEvent(QContextMenuEvent*);
    void closeEvent(QCloseEvent*);
    void createIcons();
    void createActions();
    void fetchConditionDataInitial();
    QString viewName()
    {
        return view_name;
    }

    ToolBar* getToolbar()
    {
        return toolbar;
    }

public slots:
    void languageChanged(QEvent *);
    void customContextMenuViewport();
    void customContextMenuHeader();
    void updateFailedSlot(QString);
    void bringOnTop();

private slots:
    void fetchDefaultData();
    void fetchRefreshData(QString);
    void fetchDataSlot();
    void fetchNextData();
    void fetchPreviousData();
    void copyToClipboard();
    void copyToClipboardWithHeaders();
    void defaultView();
    void refreshView();
    void addRowRefreshView();
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

    void busySlot();
    void updRowCntSlot(QString);
    void fullscreen();
    void restore();
    void toggleActions();
    void enableActions();
    void disableActions();

signals:
    void busySignal();
    void updRowCntSignal(QString);
    void viewViewClosing(ViewView *);
};

#endif // VIEWVIEW_H
