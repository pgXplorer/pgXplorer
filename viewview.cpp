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

#include "viewview.h"

ulong ViewView::viewViewObjectId = 0;

ViewView::ViewView(Database *database, QString const view_name, QString const name, QStringList column_list, QStringList column_types, bool read_only, Qt::WidgetAttribute f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    menuBar()->setVisible(false);

    error_message_box = new QMessageBox(this);

    this->database = database;
    this->view_name = view_name;
    this->column_list = column_list;
    this->column_types = column_types;
    this->column_lengths = column_lengths;

    filter_text = new QLineEdit;
    filter_text->setPlaceholderText(tr("custom filter"));

    createActions();

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("viewview");
    toolbar->setMovable(false);
    toolbar->addAction(default_action);
    toolbar->addAction(refresh_action);
    toolbar->addSeparator();
    toolbar->addAction(copy_action);
    toolbar->addAction(copy_with_headers_action);
    toolbar->addSeparator();
    toolbar->addAction(filter_action);
    toolbar->addAction(exclude_action);
    toolbar->addAction(group_action);
    toolbar->addAction(ascend_action);
    toolbar->addAction(descend_action);
    toolbar->addAction(window_action);
    toolbar->addAction(pivot_action);
    toolbar->addSeparator();
    toolbar->addAction(copy_query_action);
    toolbar->addSeparator();

    deselect_menu.setTitle(tr("Remove filter"));
    deselect_menu.setStatusTip(tr("Remove filter"));
    disarrange_menu.setTitle(tr("Remove order"));
    disarrange_menu.setStatusTip(tr("Remove order"));
    ungroup_menu.setTitle(tr("Remove group"));
    ungroup_menu.setStatusTip(tr("Remove group"));
    unwindow_menu.setTitle(tr("Remove window"));
    unwindow_menu.setStatusTip(tr("Remove window"));

    addToolBar(toolbar);
    //Identify this object with thisViewViewId for constructing database connection
    //specific to this object and this object alone.
    thisViewViewId = viewViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have ViewView
    //GUI artifacts to create overlapping threads.
    thread_busy = false;
    query_model = new QueryModel;

    quick_fetch = true;

    //Construct the SQL query needed to populate the view.
    //Cycle through the column list and cast PostgreSQL
    //time related data types to text. Otherwise, updates
    //don't work too well.
    main_query = QString("SELECT ");
    column_count = column_list.count();
    for(int column = 0; column < column_count; column++) {
        if(column == column_count-1) {
            if(column_types.value(column).startsWith("time"))
                main_query.append(column_list.value(column) + "::text");
            else if(column_types.value(column).compare("double precision") == 0)
                main_query.append(column_list.value(column) + "::text");
            else
                main_query.append(column_list.value(column));
        }
        else {
            if(column_types.value(column).startsWith("time"))
                main_query.append(column_list.value(column) + "::text, ");
            else if(column_types.value(column).compare("double precision") == 0)
                main_query.append(column_list.value(column) + "::text, ");
            else
                main_query.append(column_list.value(column) + ", ");
        }
    }
    main_query.append(" FROM ");
    main_query.append(view_name);

    setWindowTitle(name);
    setObjectName(name);

    view_view = new QTableView(this);
    view_view->setWordWrap(false);
    view_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view_view->viewport()->installEventFilter(this);
    view_view->installEventFilter(this);
    view_view->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);    
    view_view->setAlternatingRowColors(true);
    view_view->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    view_view->viewport()->setAttribute(Qt::WA_Hover);

    //Create key-sequences for fullscreen and restore.
    QShortcut *shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), SLOT(fullscreen()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), SLOT(restore()));

    //Tie vertical scrollbar of QTableView to fetch more data
    connect(view_view->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchDataSlot()));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), SLOT(busySlot()));

    setCentralWidget(view_view);

    connect(view_view->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint)), SLOT(customContextMenuHeader()));
    connect(this, &ViewView::updateColumnAggregate, query_model, &QueryModel::setColumnAggregate);
    connect(this, &ViewView::queryFailed, this, &ViewView::displayErrorMessage);

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, &ViewView::busySignal, this, &ViewView::busySlot);
    connect(this, &ViewView::notBusySignal, this, &ViewView::notBusySlot);

    //Initialise the status bar.
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0));
    statusBar()->addPermanentWidget(previous_set_button, 0);
    statusBar()->addPermanentWidget(next_set_button, 0);

    //Launch data retrieval as a future object (a different thread).
    //defaultView();
    view_query_thread = new TableQueryThread(0, query_model, check_query, main_query);
    view_query_thread->setConnectionParameters(database->getHost(),
                                                  database->getPort(),
                                                  database->getName(),
                                                  database->getUser(),
                                                  database->getPassword());
    connect(this, &ViewView::stopQuery, view_query_thread, &TableQueryThread::stopQuery);
    connect(view_query_thread, &TableQueryThread::workerIsDone, this, &ViewView::updRowCntSlot);
    connect(view_query_thread, &TableQueryThread::workerStarted, this, &ViewView::defaultView);
    connect(this, &ViewView::startQuery, view_query_thread, &TableQueryThread::executeQuery);
    view_query_thread->start();
}

void ViewView::buildQuery(int offset)
{
    //Construct the SQL query needed to populate the view.
    //Cycle through the column list and cast PostgreSQL
    //time related data types to text. Otherwise, updates
    //don't work too well.

    check_query = "SELECT 1 FROM " + view_name;

    main_query = QLatin1String("SELECT ");
    if(column_list.isEmpty()) {
        main_query.append(QLatin1String("*"));
    }
    else {
        //if(primary_key_with_oid)
        //    sql.append("oid, ");
        column_count = column_list.count();
        for(int column = 0; column < column_count; column++) {
            if(column == column_count-1) {
                if(column_types.value(column).startsWith("time") || column_types.value(column).compare("double precision") == 0) {
                    if(window_partition_clause.isEmpty() || window_partition_clause.contains(column_list.value(column))) {
                        main_query.append(current_column_aggregates.value(column)).append("(")
                           .append(column_list.value(column) + ")::text ");
                    }
                    else {
                        main_query.append(current_column_aggregates.value(column)).append("() OVER (PARTITION BY " + window_partition_clause.join(',')).append(" ORDER BY (")
                           .append(column_list.value(column) + ")::text) ");
                    }
                }
                /*else if(column_types.value(column).compare("double precision") == 0) {
                    if(window_partition_clause.isEmpty() || window_partition_clause.contains(column_list.value(column))) {
                        sql.append(current_column_aggregates.value(column)).append("(")
                           .append(column_list.value(column) + ")::text ");
                    }
                    else {

                    }
                }*/
                else {
                    if(window_partition_clause.isEmpty() || window_partition_clause.contains(column_list.value(column))) {
                        main_query.append(current_column_aggregates.value(column)).append("(")
                           .append(column_list.value(column)).append(") ");
                    }
                    else {
                        main_query.append(current_column_aggregates.value(column)).append("() OVER (PARTITION BY " + window_partition_clause.join(',')).append(" ORDER BY (")
                           .append(column_list.value(column)).append(")) ");
                    }
                }

                if(!current_column_aggregates.value(column).isEmpty()) {
                    main_query.append("\"" + current_column_aggregates.value(column) + "(")
                       .append(column_list.value(column).remove("\"") + ")\"");
                }
            }
            else {
                if(column_types.value(column).startsWith("time") || column_types.value(column).compare("double precision") == 0) {
                    if(window_partition_clause.isEmpty() || window_partition_clause.contains(column_list.value(column))) {
                        main_query.append(current_column_aggregates.value(column)).append("(")
                           .append(column_list.value(column) + ")::text");
                    }
                    else {
                        main_query.append(current_column_aggregates.value(column)).append("() OVER (PARTITION BY " + window_partition_clause.join(',')).append(" ORDER BY (")
                           .append(column_list.value(column) + ")::text)");
                    }
                }
                /*else if(column_types.value(column).compare("double precision") == 0) {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column) + ")::text");
                }*/
                else {
                    if(window_partition_clause.isEmpty() || window_partition_clause.contains(column_list.value(column))) {
                        main_query.append(current_column_aggregates.value(column)).append("(")
                           .append(column_list.value(column) + ")");
                    }
                    else {
                        main_query.append(current_column_aggregates.value(column)).append("() OVER (PARTITION BY " + window_partition_clause.join(',')).append(" ORDER BY (")
                           .append(column_list.value(column)).append("))");
                    }
                }

                if(!current_column_aggregates.value(column).isEmpty()) {
                    main_query.append(" \"" + current_column_aggregates.value(column) + "(")
                       .append(column_list.value(column).remove("\"") + ")\", ");
                }
                else {
                    main_query.append(", ");
                }
            }
            QStringList funcs;
            funcs << "";// << "max" << "sum" << "avg";
            column_aggregates.append(funcs);
        }
    }
    main_query.append(QLatin1String(" FROM "));
    main_query.append(view_name);

    check_query.append((where_clause.size() > 0 ? QString(" WHERE " + where_clause.join(" AND ")) : QString("")));
    check_query.append((group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString("")));
    check_query.append((having_clause.size() > 0 ? QString(" HAVING " + having_clause.join(" AND ")) : QString("")));

    main_query.append((where_clause.size() > 0 ? QString(" WHERE " + where_clause.join(" AND ")) : QString("")));
    main_query.append((group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString("")));
    main_query.append((having_clause.size() > 0 ? QString(" HAVING " + having_clause.join(" AND ")) : QString("")));
    main_query.append((order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : QString("")));

    if(offset > 0) {
        QString offset = " OFFSET " + QString::number((offset_list.size()+1)*FETCHSIZ);

        check_query.append(limit + offset);

        rows_from = offset_list.size()*FETCHSIZ + 1;
        offset_list.append(" OFFSET " + QString::number(rows_from - 1));
    }
    else if(offset < 0) {
        offset_list.removeLast();
        rows_from = (offset_list.size()-1)*FETCHSIZ + 1;

        QString offset = " OFFSET " + QString::number(offset_list.size()*FETCHSIZ);

        check_query.append(limit + offset);
    }
    else {
        QString offset = " OFFSET " + QString::number(FETCHSIZ);
        limit = " LIMIT " + QString::number(FETCHSIZ);

        check_query.append(limit + offset);

        rows_from = 1;

        offset_list.clear();
        offset_list.append(" OFFSET 0");
    }

    main_query.append(limit + offset_list.last());
}

void ViewView::buildPivotQuery()
{
    main_query = QLatin1String("SELECT * FROM crosstab('SELECT ");
    main_query.append(column_list.at(query_model->getPivotCol()));
    main_query.append(",");
    main_query.append(column_list.at(query_model->getPivotCat()));
    main_query.append(",");
    main_query.append(column_list.at(query_model->getPivotVal()));
    main_query.append(" FROM " + view_name);
    main_query.append(" ORDER BY 1','SELECT ");
    main_query.append(column_list.at(query_model->getPivotCat()));
    main_query.append(" FROM " + view_name);
    main_query.append(" GROUP BY 1 ORDER BY 1') AS CT(");
    main_query.append(column_list.at(query_model->getPivotCol()) + " ");
    main_query.append(column_types.at(query_model->getPivotCol()) + ", ");

    QString val_type = column_types.at(query_model->getPivotVal());

    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT " + column_list.at(query_model->getPivotCat())
            + " FROM " + view_name + " GROUP BY 1 ORDER BY 1";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve table information.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    column_query.next();
    main_query.append("\"" + (column_query.value(0).toString().isEmpty() ? " " : column_query.value(0).toString()) + "\" " + val_type);
    while (column_query.next()) {
        main_query.append(", \"" + (column_query.value(0).toString().isEmpty() ? " " : column_query.value(0).toString()) + "\" " + val_type);
    }
    main_query.append(") ");
}

//Mouse release event should enable/disable actions.
bool ViewView::eventFilter(QObject *obj, QEvent *event)
{
    if(view_view->model() == NULL)
        return QMainWindow::eventFilter(obj, event);

    if (obj == view_view->viewport()) {
        if(event->type() == QEvent::HoverMove && pivoting) {
            QHoverEvent *hover_event = static_cast<QHoverEvent*>(event);
            int hovered_col = view_view->columnAt(hover_event->pos().x());
            view_view->selectColumn(hovered_col);
            if(query_model->getPivotCol() == hovered_col && hovered_col != -1)
                setStyleSheet(pivot_col_css);
            else if(query_model->getPivotCat() == hovered_col && hovered_col != -1)
                setStyleSheet(pivot_cat_css);
            else
                setStyleSheet(pivot_hightlight_css);
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            toggleActions();
            QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
            if(mouse_event->button() == Qt::RightButton)
                customContextMenuViewport();
            if(mouse_event->button() == Qt::LeftButton && pivoting) {
                int clicked_col = view_view->columnAt(mouse_event->pos().x());
                if(clicked_col != -1) {
                    if(query_model->getPivotCol() == -1) {
                        query_model->setPivotCol(clicked_col);
                        setStyleSheet(pivot_col_css);
                        statusBar()->showMessage(tr("Choose categories to pivot."));
                    }
                    else {
                        if(clicked_col == query_model->getPivotCol()) {
                            query_model->setPivotCol(-1);
                            setStyleSheet(pivot_hightlight_css);
                            view_view->selectColumn(clicked_col);
                            statusBar()->showMessage(tr("Choose rows to pivot."));
                        }
                        else {
                            if(query_model->getPivotCat() == -1) {
                                query_model->setPivotCat(clicked_col);
                                setStyleSheet(pivot_cat_css);
                                statusBar()->showMessage(tr("Choose values to pivot."));
                            }
                            else {
                                if(clicked_col == query_model->getPivotCat()) {
                                    query_model->setPivotCat(-1);
                                    setStyleSheet(pivot_hightlight_css);
                                    view_view->selectColumn(clicked_col);
                                    statusBar()->showMessage(tr("Choose categories to pivot."));
                                }
                                else {
                                    query_model->setPivotVal(clicked_col);
                                    buildPivotQuery();
                                    unpivot();
                                    emit showQueryView(database, main_query);
                                }
                            }
                        }
                    }
                    return false;
                }
            }
        }
        else if(event->type() == QEvent::Enter) {
            statusBar()->showMessage(status_message);
        }
        else if(event->type() == QEvent::KeyPress) {
            QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
            if(key_event->key() == Qt::Key_Return ||
                    key_event->key() == Qt::Key_Enter) {
                QKeyEvent tab_event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, "\t", true, 1);
                *key_event = tab_event;
            }
            return QMainWindow::eventFilter(obj, event);
        }
        return false;
    }
    else if(obj == view_view) {
        if (event->type() == QEvent::MouseButtonRelease) {
            toggleActions();
        }
        else if(event->type() == QEvent::Enter) {
            statusBar()->showMessage(status_message);
        }
        else if(event->type() == QEvent::KeyPress) {
            QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
            if(key_event->key() == Qt::Key_Return ||
                    key_event->key() == Qt::Key_Enter) {
                QKeyEvent tab_event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, "\t", true, 1);
                *key_event = tab_event;
            }
            return QMainWindow::eventFilter(obj, event);
        }
        return false;
    }
    else {
        return QMainWindow::eventFilter(obj, event);
    }
}

//Key release event should enable/disable actions.
void ViewView::keyReleaseEvent(QKeyEvent*)
{
    toggleActions();
}

void ViewView::busySlot()
{
    thread_busy = true;
    t.start();
    setCursor(Qt::WaitCursor);
}

void ViewView::updRowCntSlot(QString dataset, QString error, bool can_fetch_more)
{
    time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0);
    rows_string = QApplication::translate("QueryView", "Rows:", 0);
    rows_string_2 = QApplication::translate("QueryView", " of whole set", 0);
    colums_string = QApplication::translate("QueryView", "Columns:", 0);
    seconds_string = QApplication::translate("QueryView", "s", 0);

    this->can_fetch_more = can_fetch_more;

    if(!error.isEmpty()) {
        queryFailed(error);
        statusBar()->showMessage(QLatin1String(""));
    }
    else {
        query_model->setRowsFrom(rows_from);
        view_view->setModel(query_model);
        rows_to = rows_from + query_model->rowCount() - 1;

        /*if(view_view->horizontalHeader() != combo_header) {
            combo_header = new ComboHeader(this);
            connect(view_view->horizontalScrollBar(), &QScrollBar::valueChanged, combo_header, &ComboHeader::fixComboPositions);
            connect(this, &ViewView::stopQuery, combo_header, &ComboHeader::deleteLater);

            connect(this, &ViewView::functionsUpdated, combo_header, &ComboHeader::refreshCombos);

            view_view->horizontalHeader()->setVisible(false);
            view_view->setHorizontalHeader(combo_header);
            view_view->horizontalHeader()->setVisible(true);
        }*/

        int column_count = query_model->columnCount();
        for(int column = 0; column < column_count; column++) {
            view_view->showColumn(column);
        }
        if(dataset.compare("previous") == 0)
            view_view->scrollToBottom();
        else if(dataset.compare("next") == 0)
            view_view->scrollToTop();
        time_elapsed = (double)t.elapsed()/1000;
        if(rows_to == 0) {
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " +  rows_string + "0" +
                                     " \t " + colums_string + QString::number(column_count));
            previous_set_button->setEnabled(false);
            next_set_button->setEnabled(false);
        }
        else {
            if(can_fetch_more) {
                statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                     " - " + QString::number(rows_to) + rows_string_2 +
                                     " \t " + colums_string + QString::number(column_count));
                next_set_button->setEnabled(true);
            }
            else {
                statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                     " - " + QString::number(rows_to) +
                                     " \t " + colums_string + QString::number(column_count));
                next_set_button->setEnabled(false);
            }
        }

        if(rows_from > 1)
            previous_set_button->setEnabled(true);
        else
            previous_set_button->setEnabled(false);

    }
    thread_busy = false;
    status_message = statusBar()->currentMessage();
    setCursor(Qt::ArrowCursor);
    toolbar->setEnabled(true);
    toggleActions();
}

void ViewView::notBusySlot()
{
    thread_busy = false;

    time_elapsed_string = tr("Time elapsed:");
    rows_string = tr("Rows:");
    rows_string_2 = tr(" of whole set");
    colums_string = tr("Columns:");
    seconds_string = tr("s");

    time_elapsed = (double)t.elapsed()/1000;
    if(can_fetch_more) {
        statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                             " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                             " - " + QString::number(rows_to) + rows_string_2 +
                             " \t " + colums_string + QString::number(column_count));
        next_set_button->setEnabled(true);
    }
    else {
        statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                             " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                             " - " + QString::number(rows_to) +
                             " \t " + colums_string + QString::number(column_count));
        next_set_button->setEnabled(false);
    }
    status_message = statusBar()->currentMessage();
    setCursor(Qt::ArrowCursor);
    toolbar->setEnabled(true);
    combo_header->setEnabled(true);
}

void ViewView::displayErrorMessage(QString error_text)
{
    error_status = true;
    error_message_box->setText(error_text);
    error_message_box->setStandardButtons(QMessageBox::Close);
    error_message_box->setIcon(QMessageBox::Critical);
    error_message_box->show();
}

void ViewView::bringOnTop()
{
    activateWindow();
    raise();
}

void ViewView::fetchDataSlot()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(query_model->rowCount() >= FETCHSIZ &&
        view_view->verticalScrollBar()->value() == view_view->verticalScrollBar()->maximum()) {
        if(can_fetch_more) {
            fetchNextSlot();
        }
    }
    //Check if vertical scrollbar is at the top-most position to trigger
    //fetching of previous dataset from database. Data retrieval launched as a
    //future object (separate thread).
    else if(rows_from > 1 &&
            view_view->verticalScrollBar()->value() == view_view->verticalScrollBar()->minimum()) {
        fetchPrevSlot();
    }
}

void ViewView::fetchNextSlot()
{
    statusBar()->showMessage(tr("Fetching data..."));
    busySlot();
    buildQuery(1);
    emit startQuery(check_query, main_query);
}

void ViewView::fetchPrevSlot()
{
    statusBar()->showMessage(tr("Fetching data..."));
    busySlot();
    buildQuery(-1);
    emit startQuery(check_query, main_query);
}

void ViewView::closeEvent(QCloseEvent *event)
{
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //ViewView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    emit viewViewClosing(this);

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("viewview_maximized", true);
        showNormal();
    }
    else
        settings.setValue("viewview_maximized", false);

    settings.setValue("viewview_pos", pos());
    settings.setValue("viewview_size", size());
    settings.setValue("icon_size", toolbar->iconSize());

    emit stopQuery();

    view_query_thread->quit();
    view_query_thread->wait();

    QSqlDatabase::removeDatabase("viewview" + main_query + QString::number(thisViewViewId));
    QMainWindow::closeEvent(event);
}

ViewView::~ViewView()
{
    delete view_query_thread;
    delete toolbar;
    delete view_view;
    delete query_model;
}

void ViewView::copyToClipboard()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage(tr("Nothing copied"));
        return;
    }

    //Need to sort the retrieved indices first.
    qSort(indices);
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    QModelIndex current;
    QString selectedText;
    foreach(current, indices) {
        QVariant data = view_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(view_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void ViewView::copyToClipboardWithHeaders()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage("Nothing copied");
        return;
    }
    qSort(indices);
    QString headerText;
    QModelIndex current;
    int prevRow = indices.at(0).row();
    foreach(current, indices) {
        if(current.row() == prevRow) {
            QVariant data = view_view->model()->headerData(current.column(), Qt::Horizontal);
            headerText.append(data.toString());
            headerText.append(QLatin1Char('\t'));
        }
        else {
            headerText.append(QLatin1Char('\n'));
            break;
        }
        prevRow = current.row();
    }
    if(!headerText.endsWith("\n"))
        headerText.append(QLatin1Char('\n'));
    QString selectedText;
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    foreach(current, indices) {
        QVariant data = view_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(view_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void ViewView::removeColumns()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage("No column(s) removed");
        return;
    }
    qSort(indices);
    int current_row = indices.at(0).row();
    int i = 0;
    while(indices.at(i).isValid()) {
        int column = indices.at(i).column();
        view_view->hideColumn(column);
        i++;
        if(i >= indices.size())
            break;
        if(indices.at(i).row() != current_row)
            break;
    }
}

void ViewView::customFilterReturnPressed()
{
    customFilter(filter_text->text());
    filter_text->clear();
    context_menu.hide();
}

void ViewView::defaultView()
{
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    where_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");
    group_clause.clear();
    having_clause.clear();
    order_clause.clear();
    error_status = false;
    grouping = true;
    windowing = true;

    unpivot();

    current_column_aggregates.clear();
    column_aggregates.clear();
    window_partition_clause.clear();
    window_order_clause.clear();
    for(int i=0; i<column_list.length(); i++) {
        current_column_aggregates << QString("");
        column_aggregates.append(QStringList() << "");
        //window_partition_clause << QString("");
        //window_order_clause.append(QStringList() << "");
    }

    busySlot();
    rows_from = 1;
    buildQuery(0);
    emit startQuery(check_query, main_query);

    emit functionsUpdated();

    disableActions();
}

void ViewView::refreshView()
{
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    error_status = false;

    busySlot();
    emit startQuery(check_query, main_query);
    disableActions();
}

void ViewView::filter()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    QString header_string;
    if(current_column_aggregates.at(index.column()).isEmpty()) {
        header_string = column_list.at(index.column());
        QVariant data = view_view->model()->data(index);
        if(data.isNull()) {
            if(!where_clause.contains(header_string + " IS NULL",
                                Qt::CaseInsensitive))
                where_clause.append(header_string + " IS NULL");
        }
        else
            if(data.type() == QMetaType::Int || data.type() == QMetaType::Long)
                where_clause.append(header_string + "=" + data.toString());
            else
                where_clause.append(header_string + "='" + data.toString().replace("'","\\'") + "'");
    }
    else {
        header_string = current_column_aggregates.at(index.column()) + "(" + column_list.at(index.column()) + ")";
        QVariant data = view_view->model()->data(index);

        if(data.isNull()) {
            if(!having_clause.contains(header_string + " IS NULL",
                                Qt::CaseInsensitive))
                having_clause.append(header_string + " IS NULL");
        }
        else {
            if(data.type() == QMetaType::Int || data.type() == QMetaType::Long ||
               data.type() == QMetaType::UInt || data.type() == QMetaType::ULong ||
               data.type() == QMetaType::LongLong || data.type() == QMetaType::ULongLong)
                having_clause.append(header_string + "=" + data.toString());
            else
                having_clause.append(header_string + "='" + data.toString().replace("'","\\'") + "'");
        }
    }

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
    context_menu.hide();
}

void ViewView::customFilter(QString filter)
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    if(current_column_aggregates.at(index.column()).isEmpty()) {
        QString item = column_list.at(index.column());
        where_clause.append(item + " " + filter);
    }
    else {
        QString item = current_column_aggregates.at(index.column()) + "(" + column_list.at(index.column()) + ")";
        having_clause.append(item + " " + filter);
    }

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
    context_menu.hide();
}

void ViewView::exclude()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));
    QString header_string;
    if(current_column_aggregates.at(index.column()).isEmpty()) {
        header_string = column_list.at(index.column());
        QVariant data = view_view->model()->data(index);
        if(data.isNull()) {
            where_clause.append(header_string + " IS NOT NULL");
        }
        else {
            if(data.type() == QMetaType::Int || data.type() == QMetaType::Long)
                where_clause.append(header_string + "<>" + data.toString());
            else
                where_clause.append(header_string + "<>'" + data.toString() + "'");
        }
    }
    else {
        header_string = current_column_aggregates.at(index.column()) + "(" + column_list.at(index.column()) + ")";
        QVariant data = view_view->model()->data(index);
        if(data.isNull()) {
            having_clause.append(header_string + " IS NOT NULL");
        }
        else {
            if(data.type() == QMetaType::Int || data.type() == QMetaType::Long ||
               data.type() == QMetaType::UInt || data.type() == QMetaType::ULong ||
               data.type() == QMetaType::LongLong || data.type() == QMetaType::ULongLong)
                having_clause.append(header_string + "<>" + data.toString());
            else
                having_clause.append(header_string + "<>'" + data.toString() + "'");
        }
    }

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
    context_menu.hide();
}

void ViewView::ascend()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QString header_string;
    if(!current_column_aggregates.at(index.column()).isEmpty() && !windowing)
        header_string = current_column_aggregates.at(index.column()) + "(" + column_list.at(index.column()) + ")";
    else
        header_string = column_list.at(index.column());
    if(order_clause.contains(header_string + " DESC"))
        order_clause.removeOne(header_string + " DESC");
    if(!order_clause.contains(header_string + " ASC")) {
        order_clause.append(header_string + " ASC");

        busySlot();
        buildQuery(0);
        emit startQuery(check_query, main_query);
    }
}

void ViewView::descend()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QString header_string;
    if(!current_column_aggregates.at(index.column()).isEmpty() && !windowing)
        header_string = current_column_aggregates.at(index.column()) + "(" + column_list.at(index.column()) + ")";
    else
        header_string = column_list.at(index.column());
    if(order_clause.contains(header_string + " ASC"))
        order_clause.removeOne(header_string + " ASC");
    if(!order_clause.contains(header_string + " DESC")) {
        order_clause.append(header_string + " DESC");

        busySlot();
        buildQuery(0);
        emit startQuery(check_query, main_query);
    }
}

void ViewView::group()
{
    grouping = true;
    windowing = false;
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QString header_string = column_list.at(index.column());

    if(!group_clause.contains(header_string)) {
        group_clause.append(header_string);

        column_aggregates.clear();
        for(int i=0; i < column_types.length(); i++) {
            if(group_clause.indexOf(column_list.at(i)) != -1) {
                current_column_aggregates.replace(i, "");
                column_aggregates.append(QStringList() << "");
                continue;
            }

            if(column_types.at(i).startsWith("bool")) {
                current_column_aggregates.replace(i, "count");
                QStringList fs;
                fs << "count" << "every";
                column_aggregates.append(fs);
            }
            else if(column_types.at(i).startsWith("char") ||
                    column_types.at(i).startsWith("text") ||
                    column_types.at(i).startsWith("date") ||
                    column_types.at(i).contains("time")) {
                current_column_aggregates.replace(i, "count");
                QStringList fs;
                fs << "count" << "min" << "max";
                column_aggregates.append(fs);
            }
            else if(column_types.at(i).contains("int") ||
                    column_types.at(i).startsWith("real") ||
                    column_types.at(i).startsWith("double precision") ||
                    column_types.at(i).startsWith("numeric")) {
                current_column_aggregates.replace(i, "sum");
                QStringList fs;
                fs << "sum" << "count" << "min" << "max" << "avg";
                column_aggregates.append(fs);
            }
        }

        //Remove all order clause terms that
        //don't match our grouping clause terms
        QStringList pruned_order_clause;
        for(int i=0; i<order_clause.length(); i++) {
            for(int j=0; j<group_clause.length(); j++) {
                if(order_clause.at(i).startsWith(group_clause.at(j))) {
                    pruned_order_clause.append(order_clause.at(i));
                    break;
                }
            }
        }
        order_clause = pruned_order_clause;

        busySlot();
        buildQuery(0);
        emit startQuery(check_query, main_query);
    }
    emit functionsUpdated();
}

void ViewView::regroup(QStringList aggs)
{
    column_aggregates.clear();
    current_column_aggregates = aggs;

    for(int i=0; i<current_column_aggregates.length(); i++) {
        if(!current_column_aggregates.at(i).isEmpty()) {
            for(int j=0; j<order_clause.length(); j++) {
                QString rem(column_list.at(i));
                if(order_clause.at(j).contains(rem.remove("\""))) {
                    order_clause.replace(j, current_column_aggregates.at(i) + "(" + rem + ")");
                }
            }
        }
    }

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
}

void ViewView::window()
{
    grouping = false;
    windowing = true;
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QString item = column_list.at(index.column());

    if(!window_partition_clause.contains(item)) {
        window_partition_clause.append(item);

        column_aggregates.clear();
        for(int i=0; i < column_list.length(); i++) {
            if(window_partition_clause.contains(column_list.at(i))) {
                current_column_aggregates.replace(i, "");
                window_order_clause.append(QStringList());
                column_aggregates.append(QStringList() << "");
            }
            else {
                current_column_aggregates.replace(i, "rank");
                window_order_clause.append(QStringList() << column_list.at(i));
                QStringList fs;
                fs << "rank" << "dense_rank" << "percent_rank" << "cume_dist" << "row_number";
                column_aggregates.append(fs);
            }
        }

        busySlot();
        buildQuery(0);
        emit startQuery(check_query, main_query);
    }

    emit functionsUpdated();
}

void ViewView::pivot()
{
    view_view->clearSelection();

    if(pivoting) {
        unpivot();
        statusBar()->showMessage(status_message);
    }
    else {
        pivoting = true;
        statusBar()->showMessage(tr("Choose rows to pivot."));
        setStyleSheet(pivot_hightlight_css);
    }
}

void ViewView::unpivot()
{
    pivoting = false;
    pivot_action->setChecked(false);

    query_model->setPivotCol(-1);
    query_model->setPivotCat(-1);
    query_model->setPivotVal(-1);

    setStyleSheet(default_css);
}

void ViewView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0);
        rows_string = QApplication::translate("QueryView", "Rows:", 0);
        rows_string_2 = QApplication::translate("QueryView", " of whole set", 0);
        colums_string = QApplication::translate("QueryView", "Columns:", 0);
        seconds_string = QApplication::translate("QueryView", "s", 0);

        if(can_fetch_more) {
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                 " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                 " - " + QString::number(rows_to) + rows_string_2 +
                                 " \t " + colums_string + QString::number(column_count));
        }
        else {
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                 " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                 " - " + QString::number(rows_to) +
                                 " \t " + colums_string + QString::number(column_count));
        }
        status_message = statusBar()->currentMessage();
        deselect_menu.setTitle(tr("Remove filter"));
        deselect_menu.setStatusTip(tr("Remove filter"));
        disarrange_menu.setTitle(tr("Remove order"));
        disarrange_menu.setStatusTip(tr("Remove order"));

        default_action->setText(tr("Default"));
        default_action->setStatusTip(tr("Default"));
        refresh_action->setText(tr("Refresh"));
        refresh_action->setStatusTip(tr("Refresh"));
        copy_action->setText(tr("Copy"));
        copy_action->setStatusTip(tr("Copy selected"));
        copy_with_headers_action->setText(tr("Copy with headers"));
        copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
        remove_columns_action->setText(tr("Remove column(s)"));
        remove_columns_action->setStatusTip(tr("Removes the column from this display."));
        filter_action->setText(tr("Filter"));
        filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
        exclude_action->setText(tr("Exclude"));
        exclude_action->setStatusTip(tr("Filter table exclusive of selected cell value on column"));
        ascend_action->setText(tr("Ascending order"));
        ascend_action->setStatusTip(tr("Ascending order"));
        descend_action->setText(tr("Descending order"));
        descend_action->setStatusTip(tr("Descending order"));
        remove_all_filters_action->setText(tr("All filters"));
        remove_all_filters_action->setStatusTip(tr("Remove all filters"));
        remove_all_ordering_action->setText(tr("All ordering"));
        remove_all_ordering_action->setStatusTip(tr("Remove all ordering"));
        custom_filter_action->setText(tr("Custom filter"));
        custom_filter_action->setStatusTip(tr("Custom filter"));
        copy_query_action->setText(tr("Copy query"));
        copy_query_action->setStatusTip(tr("Copy the query to clipboard"));

        filter_text->setPlaceholderText(tr("custom filter"));
    }
}

void ViewView::customContextMenuViewport()
{
    if(thread_busy)
        return;

    filter_text->clear();
    context_menu.clear();

    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty())
        return;

    //Need to sort the retrieved indices first.
    qSort(indices);

    QModelIndex index;
    QVariant data;
    int order_clause_size = order_clause.size();
    int group_clause_size = group_clause.size();
    int window_clause_size = window_partition_clause.size();

    if(indices.size() > 1) {
        context_menu.addAction(copy_action);
        context_menu.addAction(copy_with_headers_action);
        context_menu.addSeparator();
        context_menu.addAction(remove_columns_action);
        context_menu.popup(QCursor::pos());
        return;
    }

    context_menu.addAction(filter_action);
    context_menu.addAction(exclude_action);
    context_menu.addSeparator();
    context_menu.addAction(custom_filter_action);
    context_menu.addSeparator();

    deselect_menu.clear();
    for(int i=0; i<where_clause.size(); i++)
        deselect_menu.addAction(filter_icon, where_clause.at(i));
    if(where_clause.size() > 1) {
        deselect_menu.addSeparator();
        deselect_menu.addAction(remove_all_filters_action);
    }
    context_menu.addMenu(&deselect_menu);
    context_menu.addSeparator();

    context_menu.addAction(group_action);
    ungroup_menu.clear();
    for(int i=0; i<group_clause_size; i++) {
        QString group = group_clause.at(i);
        ungroup_menu.addAction(group_icon, group);
    }
    if(group_clause_size > 1) {
        ungroup_menu.addSeparator();
        ungroup_menu.addAction(remove_all_grouping_action);
    }
    context_menu.addMenu(&ungroup_menu);
    context_menu.addSeparator();

    context_menu.addAction(window_action);
    unwindow_menu.clear();
    for(int i=0; i<window_clause_size; i++) {
        QString window = window_partition_clause.at(i);
        unwindow_menu.addAction(window_icon, window);
    }
    if(window_clause_size > 1) {
        unwindow_menu.addSeparator();
        unwindow_menu.addAction(remove_all_windowing_action);
    }
    context_menu.addMenu(&unwindow_menu);
    context_menu.addSeparator();

    context_menu.addAction(ascend_action);
    context_menu.addAction(descend_action);

    disarrange_menu.clear();
    for(int i=0; i<order_clause_size; i++) {
        QString order = order_clause.at(i);
        if(order.endsWith(" ASC")) {
            order.remove(" ASC");
            disarrange_menu.addAction(ascend_icon, order);
        }
        else if (order.endsWith(" DESC")) {
            order.remove(" DESC");
            disarrange_menu.addAction(descend_icon, order);
        }
    }
    if(order_clause_size > 1) {
        disarrange_menu.addSeparator();
        disarrange_menu.addAction(remove_all_ordering_action);
    }
    context_menu.addMenu(&disarrange_menu);
    context_menu.addSeparator();
    context_menu.addAction(copy_action);
    context_menu.addAction(copy_with_headers_action);
    context_menu.addSeparator();
    context_menu.addAction(remove_columns_action);

    if(!indices.isEmpty()) {
        index = indices.first();
        data = view_view->model()->data(index);
        if(data.canConvert<QString>()) {
            filter_action->setEnabled(true);
            exclude_action->setEnabled(true);
            group_action->setEnabled(true);
            ascend_action->setEnabled(true);
            descend_action->setEnabled(true);
            window_action->setEnabled(true);
        }
    }

    QAction *a = context_menu.exec(QCursor::pos());
    if(!a)
        return;
    int status;

    if(a->icon().cacheKey() == filter_icon.cacheKey() ||
            a->icon().cacheKey() == exclude_icon.cacheKey()) {
        if((status = where_clause.indexOf(a->text())) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));
            where_clause.removeAt(status);
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            return;
        }
    }

    else if(a->icon().cacheKey() == having_icon.cacheKey()) {
        if((status = having_clause.indexOf(a->text())) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));
            having_clause.removeAt(status);
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            return;
        }
    }

    else if(a->icon().cacheKey() == group_icon.cacheKey()) {
        QString item_for_removal = a->text();
        if((status = group_clause.indexOf(item_for_removal)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            group_clause.removeAt(status);

            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            column_aggregates.clear();
            if(group_clause.size() == 0) {
                having_clause.clear();
                windowing = true;
                for(int i=0; i < current_column_aggregates.length(); i++) {
                   current_column_aggregates.replace(i, "");
                   column_aggregates.append(QStringList() << "");
                }
            }
            else {
                //Remove all order clause terms that
                //don't match our grouping clause terms
                QStringList pruned_order_clause;
                for(int i=0; i<order_clause.length(); i++) {
                    for(int j=0; j<group_clause.length(); j++) {
                        if(order_clause.at(i).startsWith(group_clause.at(j))) {
                            pruned_order_clause.append(order_clause.at(i));
                            break;
                        }
                    }
                }
                order_clause = pruned_order_clause;

                for(int i=0; i < column_types.length(); i++) {
                    if(group_clause.indexOf(column_list.at(i)) != -1) {
                        current_column_aggregates.replace(i, "");
                        QStringList fs;
                        fs << "";
                        column_aggregates.append(fs);
                        continue;
                    }

                    if(column_types.at(i).startsWith("bool")) {
                        current_column_aggregates.replace(i, "count");
                        QStringList fs;
                        fs << "count" << "every";
                        column_aggregates.append(fs);
                    }
                    else if(column_types.at(i).startsWith("char") ||
                            column_types.at(i).startsWith("text") ||
                            column_types.at(i).startsWith("date") ||
                            column_types.at(i).contains("time")) {
                        current_column_aggregates.replace(i, "count");
                        QStringList fs;
                        fs << "count" << "min" << "max";
                        column_aggregates.append(fs);
                    }
                    else if(column_types.at(i).contains("int") ||
                            column_types.at(i).startsWith("real") ||
                            column_types.at(i).startsWith("double precision") ||
                            column_types.at(i).startsWith("numeric")) {
                        current_column_aggregates.replace(i, "sum");
                        QStringList fs;
                        fs << "sum" << "count" << "min" << "max" << "avg";
                        column_aggregates.append(fs);
                    }
                }
            }

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            emit functionsUpdated();
            return;
        }
    }
    else if(a->icon().cacheKey() == window_icon.cacheKey()) {
        QString item_for_removal = a->text();
        if((status = window_partition_clause.indexOf(item_for_removal)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            window_partition_clause.removeAt(status);

            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            column_aggregates.clear();
            if(window_partition_clause.size() == 0) {
                grouping = true;
                for(int i=0; i < current_column_aggregates.length(); i++) {
                   current_column_aggregates.replace(i, "");
                   column_aggregates.append(QStringList() << "");
                }
            }
            else {
                for(int i=0; i < column_types.length(); i++) {
                    if(window_partition_clause.indexOf(column_list.at(i)) != -1) {
                        current_column_aggregates.replace(i, "");
                        QStringList fs;
                        fs << "";
                        column_aggregates.append(fs);
                    }
                    else {
                        current_column_aggregates.replace(i, "rank");
                        QStringList fs;
                        fs << "rank" << "dense_rank" << "percent_rank" << "cume_dist" << "row_number";
                        column_aggregates.append(fs);
                    }
                }
            }

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            emit functionsUpdated();
            return;
        }
    }
    else if(a->icon().cacheKey() == ascend_icon.cacheKey()) {
        QString order = a->text();
        order.append(" ASC");
        if((status = order_clause.indexOf(order)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            order_clause.removeAt(status);
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            return;
        }
    }
    else if(a->icon().cacheKey() == descend_icon.cacheKey()) {
        QString order = a->text();
        order.append(" DESC");
        if((status = order_clause.indexOf(order)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            order_clause.removeAt(status);
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            busySlot();
            buildQuery(0);
            emit startQuery(check_query, main_query);
            return;
        }
    }
}

void ViewView::customContextMenuHeader()
{
    return;
    if(thread_busy)
        return;
    context_menu.clear();

    QModelIndexList indices = view_view->selectionModel()->selectedRows();
    if(indices.isEmpty()) {
        return;
    }

    context_menu.exec(QCursor::pos());
}

void ViewView::removeAllFilters()
{
    statusBar()->showMessage(tr("Fetching data..."));
    where_clause.clear();
    having_clause.clear();

    offset_list.clear();
    offset_list.append(" OFFSET 0");

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
}

void ViewView::removeAllGrouping()
{
    column_aggregates.clear();
    for(int i=0; i < current_column_aggregates.length(); i++) {
        current_column_aggregates.replace(i, "");
        column_aggregates.append(QStringList() << "");
    }

    statusBar()->showMessage(tr("Fetching data..."));

    group_clause.clear();
    having_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
    emit functionsUpdated();
}

void ViewView::removeAllWindowing()
{
    column_aggregates.clear();
    for(int i=0; i < current_column_aggregates.length(); i++) {
        current_column_aggregates.replace(i, "");
        column_aggregates.append(QStringList() << "");
    }

    statusBar()->showMessage(tr("Fetching data..."));

    window_partition_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
    emit functionsUpdated();
}

void ViewView::removeAllOrdering()
{
    statusBar()->showMessage(tr("Fetching data..."));

    order_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    busySlot();
    buildQuery(0);
    emit startQuery(check_query, main_query);
}

void ViewView::copyQuery()
{
    QString copy_sql = main_query;
    copy_sql.remove("::text");
    int lt = copy_sql.indexOf(" LIMIT");
    copy_sql.truncate(lt);

    qApp->clipboard()->setText(copy_sql);
}

void ViewView::fullscreen()
{
    this->showFullScreen();
}

void ViewView::restore()
{
    this->showNormal();
}

void ViewView::toggleActions()
{
    disableActions();
    if(view_view->selectionModel()) {
        //Only enable filter, exclude and ordering actions
        //when a single cell is selected.
        if(view_view->selectionModel()->selectedIndexes().isEmpty()) {
            copy_action->setEnabled(false);
            copy_with_headers_action->setEnabled(false);
        }
        else {
            copy_action->setEnabled(true);
            copy_with_headers_action->setEnabled(true);
            if(view_view->selectionModel()->selectedIndexes().size() == 1)
                enableActions();
        }
    }
}

void ViewView::enableActions()
{
    if(!thread_busy) {
        filter_action->setEnabled(true);
        exclude_action->setEnabled(true);
        if(grouping)
            group_action->setEnabled(true);
        else
            group_action->setEnabled(false);
        if(windowing)
            window_action->setEnabled(true);
        else
            window_action->setEnabled(false);
        ascend_action->setEnabled(true);
        descend_action->setEnabled(true);
    }
}

void ViewView::disableActions()
{
    filter_action->setEnabled(false);
    exclude_action->setEnabled(false);
    group_action->setEnabled(false);
    ascend_action->setEnabled(false);
    descend_action->setEnabled(false);
    window_action->setEnabled(false);
}

void ViewView::createActions()
{
    default_action = new QAction(QIcon(":/icons/view2.svg"), tr("Default"), this);
    default_action->setShortcut(QKeySequence("Ctrl+D"));
    default_action->setStatusTip(tr("Default"));
    connect(default_action, &QAction::triggered, this, &ViewView::defaultView);

    refresh_action = new QAction(QIcon(":/icons/refresh.svg"), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    refresh_action->setStatusTip(tr("Refresh"));
    connect(refresh_action, &QAction::triggered, this, &ViewView::refreshView);

    copy_action = new QAction(QIcon(":/icons/copy_without_headers.svg"), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected"));
    //copy_action->setEnabled(false);
    connect(copy_action, &QAction::triggered, this, &ViewView::copyToClipboard);

    copy_with_headers_action = new QAction(QIcon(":/icons/copy_with_headers.svg"), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    //copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, &QAction::triggered, this, &ViewView::copyToClipboardWithHeaders);

    remove_columns_action = new QAction(QIcon(":/icons/removecolumn.svg"), tr("Remove column(s)"), this);
    remove_columns_action->setStatusTip(tr("Removes the column from this display."));
    connect(remove_columns_action, &QAction::triggered, this, &ViewView::removeColumns);

    filter_action = new QAction(filter_icon, tr("Filter"), this);
    filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
    filter_action->setEnabled(false);
    connect(filter_action, &QAction::triggered, this, &ViewView::filter);

    exclude_action = new QAction(QIcon(":/icons/exclude.svg"), tr("Exclude"), this);
    exclude_action->setStatusTip(tr("Filter table exclusive of selected cell value on column"));
    exclude_action->setEnabled(false);
    connect(exclude_action, &QAction::triggered, this, &ViewView::exclude);

    group_action = new QAction(group_icon, tr("Group"), this);
    group_action->setStatusTip(tr("Group this column by this item value"));
    group_action->setEnabled(false);
    connect(group_action, &QAction::triggered, this, &ViewView::group);

    window_action = new QAction(window_icon, tr("Window"), this);
    window_action->setStatusTip(tr("Window function"));
    window_action->setEnabled(false);
    connect(window_action, &QAction::triggered, this, &ViewView::window);

    pivot_action = new QAction(pivot_icon, tr("Pivot"), this);
    pivot_action->setCheckable(true);
    pivot_action->setStatusTip(tr("Pivot table"));
    connect(pivot_action, &QAction::triggered, this, &ViewView::pivot);

    ascend_action = new QAction(ascend_icon, tr("Ascending order"), this);
    ascend_action->setStatusTip(tr("Ascending order"));
    ascend_action->setEnabled(false);
    connect(ascend_action, &QAction::triggered, this, &ViewView::ascend);

    descend_action = new QAction(descend_icon, tr("Descending order"), this);
    descend_action->setStatusTip(tr("Descending order"));
    descend_action->setEnabled(false);
    connect(descend_action, &QAction::triggered, this, &ViewView::descend);

    remove_all_filters_action = new QAction(tr("All filters"), this);
    remove_all_filters_action->setStatusTip(tr("Remove all filters"));
    connect(remove_all_filters_action, &QAction::triggered, this, &ViewView::removeAllFilters);

    remove_all_grouping_action = new QAction(tr("All grouping"), this);
    remove_all_grouping_action->setStatusTip(tr("Remove all grouping"));
    connect(remove_all_grouping_action, &QAction::triggered, this, &ViewView::removeAllGrouping);

    remove_all_windowing_action = new QAction(tr("All windowing"), this);
    remove_all_windowing_action->setStatusTip(tr("Remove all windowing"));
    connect(remove_all_windowing_action, &QAction::triggered, this, &ViewView::removeAllWindowing);

    remove_all_ordering_action = new QAction(tr("All ordering"), this);
    remove_all_ordering_action->setStatusTip(tr("Remove all ordering"));
    connect(remove_all_ordering_action, &QAction::triggered, this, &ViewView::removeAllOrdering);

    custom_filter_action = new QWidgetAction(this);
    custom_filter_action->setStatusTip(tr("Custom filter"));
    custom_filter_action->setIcon(filter_icon);
    custom_filter_action->setDefaultWidget(filter_text);
    connect(filter_text, &QLineEdit::returnPressed, this, &ViewView::customFilterReturnPressed);

    copy_query_action = new QAction(QIcon(":/icons/copy_sql.svg"), tr("Copy query"), this);
    copy_query_action->setStatusTip(tr("Copy the query to clipboard"));
    connect(copy_query_action, &QAction::triggered, this, &ViewView::copyQuery);

    previous_set_action = new QAction(QIcon(":/icons/previous.svg"), "", this);
    previous_set_action->setToolTip(tr("Fetch previous set"));
    connect(previous_set_action, &QAction::triggered, this, &ViewView::fetchPrevSlot);
    previous_set_button = new QToolButton;
    previous_set_button->setDefaultAction(previous_set_action);
    previous_set_button->setEnabled(false);

    next_set_action = new QAction(QIcon(":/icons/next.svg"), "", this);
    next_set_action->setToolTip(tr("Fetch next set"));
    connect(next_set_action, &QAction::triggered, this, &ViewView::fetchNextSlot);
    next_set_button = new QToolButton;
    next_set_button->setDefaultAction(next_set_action);
    next_set_button->setEnabled(false);
}
