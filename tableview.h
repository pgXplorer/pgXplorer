/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2010-2013, davyjones <dj@pgxplorer.com>

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

#include <QDebug>
#include <QtWidgets/QtWidgets>
#include <QtConcurrent/QtConcurrent>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlError>
#include <QtGui>
#include "database.h"
#include "tablequerythread.h"
#include "tablemodel.h"
#include "comboheader.h"

#define FETCHSIZ 10000

class TableView;
class ComboHeader;
class NewRowTableView;

class TableView : public QMainWindow
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
    QString table_name;
    QStringList primary_key;
    //bool primary_key_with_oid;
    QStringList column_list;
    QStringList column_types;
    QStringList column_lengths;
    QStringList current_column_aggregates;
    QStringList current_column_windows;
    QList<QStringList> column_aggregates;
    QList<QStringList> column_windows;
    QMenuBar *menubar;
    QDockWidget *dock_widget;
    QString status_message;
    QMenu context_menu;
    QMenu deselect_menu;
    QMenu disarrange_menu;
    QMenu ungroup_menu;
    QMenu unwindow_menu;
    QTime t;
    ToolBar *toolbar;
    TableQueryThread *table_query_thread;
    TableModel *table_model;
    QTableView *table_view;
    QStandardItemModel *new_row_model;
    NewRowTableView *new_row_view;
    QString check_query;
    QString main_query;
    QStringList where_clause;
    QStringList order_clause;
    QStringList group_clause;
    QStringList having_clause;
    QStringList window_partition_clause;
    QList<QStringList> window_order_clause;
    QString limit;
    QStringList offset_list;
    bool quick_fetch;
    bool can_fetch_more;
    qint32 rows_from;
    qint32 rows_to;
    qint32 column_count;
    bool thread_busy;
    bool grouping;
    bool windowing;
    bool pivoting;
    ulong thisTableViewId;
    const QIcon key_icon = QIcon(":/icons/key.svg");
    const QIcon filter_icon = QIcon(":/icons/filter.svg");
    const QIcon exclude_icon = QIcon(":/icons/exclude.svg");
    const QIcon group_icon = QIcon(":/icons/group.svg");
    const QIcon having_icon = QIcon(":/icons/filter.svg");
    const QIcon window_icon = QIcon(":/icons/window.svg");
    const QIcon pivot_icon = QIcon(":/icons/pivot.svg");
    const QIcon ascend_icon = QIcon(":/icons/ascending.svg");
    const QIcon descend_icon = QIcon(":/icons/descending.svg");
    const QString default_css = "QTableView {selection-background-color: \
                                    qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0.25, \
                                    stop: 0 #5F5F7F, stop: 1 #7F7F9F); \
                                    selection-color: #F0F0F0; \
                                    color: #0F0F0F; \
                                 } \
                                 QHeaderView {\
                                    border-bottom: 2px solid lightgray;\
                                    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, \
                                    stop: 0 #FFFFFF, stop: 1 #EEEEEE); \
                                 }";
    const QString pivot_hightlight_css = "QTableView {selection-background-color: rgba(128, 128, 255, 100); \
                                              selection-color: #0F0F0F; \
                                              color: #0F0F0F; \
                                          }";
    const QString pivot_cat_css = "QTableView {selection-background-color: rgba(0, 255, 0, 100); \
                                       selection-color: #0F0F0F; \
                                       color: #0F0F0F; \
                                   }";
    const QString pivot_col_css = "QTableView {selection-background-color: rgba(255, 0, 0, 100); \
                                       selection-color: #0F0F0F; \
                                       color: #0F0F0F; \
                                   }";
    QAction *default_action;
    QAction *refresh_action;
    QAction *copy_action;
    QAction *copy_with_headers_action;
    QAction *remove_columns_action;
    QAction *filter_action;
    QAction *exclude_action;
    QAction *group_action;
    QAction *window_action;
    QAction *ascend_action;
    QAction *descend_action;
    QAction *pivot_action;
    QAction *remove_all_filters_action;
    QAction *remove_all_ordering_action;
    QAction *remove_all_grouping_action;
    QAction *remove_all_windowing_action;
    QAction *truncate_action;
    QAction *delete_rows_action;
    QAction *previous_set_action;
    QAction *next_set_action;
    QWidgetAction *custom_filter_action;
    QWidgetAction *bulk_update_action;
    QAction *copy_query_action;
    QLineEdit *filter_text;
    QLineEdit *bulk_update;
    QBrush red_brush;
    QBrush green_brush;
    QToolButton *previous_set_button;
    QToolButton *next_set_button;
    bool error_status;
    QMessageBox *error_message_box;
    ComboHeader *combo_header;

public:
    static ulong tableViewObjectId;
    explicit TableView(Database *, QString const, QStringList const, QStringList const, QStringList const, QStringList const, bool, Qt::WidgetAttribute f);
    ~TableView();

    bool eventFilter(QObject *, QEvent*);
    void keyReleaseEvent(QKeyEvent*);
    //void contextMenuEvent(QContextMenuEvent*);
    void closeEvent(QCloseEvent*);

    void createBrushes();
    void createActions();
    void deleteData();
    QTableView *getTableView()
    {
        return table_view;
    }

    NewRowTableView *getNewRowView()
    {
        return new_row_view;
    }

    QString tableName()
    {
        return table_name;
    }

    ToolBar* getToolbar()
    {
        return toolbar;
    }

    QStringList columnNames()
    {
        return column_list;
    }

    QStringList primaryKeys()
    {
        return primary_key;
    }

    QStringList curColumnAggs()
    {
        return current_column_aggregates;
    }

    QList<QStringList> columnAggsList()
    {
        return column_aggregates;
    }

public slots:
    void languageChanged(QEvent*);
    void customContextMenuViewport();
    void customContextMenuHeader();
    void displayErrorMessage(QString);
    void bringOnTop();
    void regroup(QStringList);
    void selectColumn(int col) {
        table_view->selectColumn(col);
    }

private slots:
    void buildQuery(int);
    void buildPivotQuery();
    void fetchDataSlot();
    void fetchNextSlot();
    void fetchPrevSlot();
    void bulkUpdateData(QModelIndexList, QVariant);
    void copyToClipboard();
    void copyToClipboardWithHeaders();
    void defaultView();
    void refreshView();
    void addRowRefreshView();
    void filter();
    void customFilter(QString);
    void exclude();
    void group();
    void window();
    void pivot();
    void unpivot();
    void ascend();
    void descend();
    void removeAllFilters();
    void removeAllGrouping();
    void removeAllWindowing();
    void removeAllOrdering();
    void removeColumns();
    void customFilterReturnPressed();
    void copyQuery();
    bool insertRow();
    void truncateTable();
    void deleteRows();
    void deleteRow(int);
    void updatePrimaryKeyInfo();
    void bulkUpdate();
    void busySlot();
    void notBusySlot();
    void updRowCntSlot(QString, QString, bool);
    void fullscreen();
    void restore();
    void toggleActions();
    void enableActions();
    void disableActions();

signals:
    void busySignal();
    void notBusySignal();
    void showQueryView(Database *, QString);
    void updRowCntSignal(QString, QString);
    void updateColumnAggregate(QStringList);
    void canUpdate(bool);
    void queryFailed(QString);
    void startQuery(QString, QString);
    void stopQuery();
    void functionsUpdated();
    void tableViewClosing(TableView*);
};

class NewRowTableView : public QTableView
{
    Q_OBJECT

private:
    int column_count;

public:
    NewRowTableView(QWidget *parent = 0);
    void setColumnCount(quint32);
    bool eventFilter(QObject *, QEvent *);

public slots:
    void resizeCells(int, int, int);
    void setFocuz() {
        setFocus();
    }

signals:
    void insertRow();
};

#endif // TABLEVIEW_H
