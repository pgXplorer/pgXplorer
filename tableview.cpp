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

#include "tableview.h"

ulong TableView::tableViewObjectId = 0;

NewRowTableView::NewRowTableView(QWidget *parent)
{
    setParent(parent);
    installEventFilter(this);
}

void NewRowTableView::setColumnCount(quint32 count)
{
    column_count = count;
}

bool NewRowTableView::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        //Map ENTER/RETURNs to TAB
        if(key_event->key() == Qt::Key_Return ||
                key_event->key() == Qt::Key_Enter) {
            if(currentIndex().column() == column_count-1) {
                setCurrentIndex(QModelIndex());
                emit insertRow();
                return true;
            }
            QKeyEvent tab_event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, "\t", true, 1);
            *key_event = tab_event;
        }
        //Consume TABs (and ENTER/RETURNs) on the last column
        else if(key_event->key() == Qt::Key_Tab) {
            if(currentIndex().column() == column_count-1)
                return true;
        }
    }
    return QTableView::eventFilter(obj, event);
}

void NewRowTableView::resizeCells(int logicalIndex, int oldSize, int newSize)
{
    horizontalHeader()->resizeSection(logicalIndex, newSize);
}

TableView::TableView(Database *database, QString const table_name, QString const name, QStringList column_list, QStringList primary_key, QStringList column_types, QStringList column_lengths, bool read_only, Qt::WidgetAttribute f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    menuBar()->setVisible(false);

    error_status = false;
    error_message_box = new QMessageBox(this);
    error_message_box->setWindowModality(Qt::WindowModal);

    this->database = database;
    this->table_name = table_name;
    this->primary_key = primary_key;
    //this->primary_key_with_oid = primary_key_with_oid;
    this->column_list = column_list;
    this->column_types = column_types;
    this->column_lengths = column_lengths;
    foreach(const QString &str, column_list)
        this->current_column_aggregates << QString("");

    filter_text = new QLineEdit;
    filter_text->setPlaceholderText(tr("custom filter"));

    bulk_update = new QLineEdit;
    bulk_update->setPlaceholderText(tr("bulk update"));

    table_model = new TableModel(database, this->primary_key, this->table_name);

    createBrushes();
    createActions();
    setWindowTitle(name);
    setObjectName(name);
    buildTableQuery();

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("tableview");
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
    toolbar->addSeparator();
    toolbar->addAction(copy_query_action);
    toolbar->addSeparator();
    if(!read_only) {
        toolbar->addAction(delete_rows_action);
        toolbar->addAction(truncate_action);
    }

    deselect_menu.setTitle(tr("Remove filter"));
    deselect_menu.setStatusTip(tr("Remove filter"));
    disarrange_menu.setTitle(tr("Remove order"));
    disarrange_menu.setStatusTip(tr("Remove order"));
    ungroup_menu.setTitle(tr("Remove group"));
    ungroup_menu.setStatusTip(tr("Remove group"));

    addToolBar(toolbar);
    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisTableViewId = tableViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have TableView
    //GUI artifacts to create overlapping threads yet.
    thread_busy = false;

    //Quick fetch feature that is supposed to fetch only a fixed
    //number of rows of a large table to populate the view.
    //For now, we will always enable Quick fetch.
    quick_fetch = true;

    setContextMenuPolicy(Qt::NoContextMenu);

    table_view = new QTableView(this);
    table_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table_view->viewport()->installEventFilter(this);
    table_view->installEventFilter(this);
    table_view->setAlternatingRowColors(true);
    table_view->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //QShortcut *shortcut_copy_with_headers = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    //connect(shortcut_copy_with_headers, SIGNAL(activated()), SLOT(copych()));

    //Create key-sequences for fullscreen and restore.
    QShortcut *shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), SLOT(fullscreen()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), SLOT(restore()));

    //Tie vertical scrollbar of TableView to fetch more data
    connect(table_view->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchDataSlot()));

    //Tie thread finish to an update slot that refreshes meta-information.
    connect(this, SIGNAL(updRowCntSignal(QString)), SLOT(updRowCntSlot(QString)));
    connect(this, SIGNAL(updRowCntSignal(QString)), table_model, SLOT(clearCache()));
    connect(table_model, SIGNAL(updateFailed(QString)), SLOT(displayErrorMessage(QString)));
    connect(this, SIGNAL(queryFailed(QString)), SLOT(displayErrorMessage(QString)));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), SLOT(busySlot()));
    connect(this, SIGNAL(notBusySignal()), SLOT(notBusySlot()));

    connect(table_view->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint)), SLOT(customContextMenuHeader()));

    setCentralWidget(table_view);

    //New data input row.
    new_row_model = new QStandardItemModel(1, column_list.size());
    for (int column = 0; column < column_list.size(); ++column) {
             QStandardItem *item = new QStandardItem(QString(""));
             new_row_model->setItem(0, column, item);
    }
    new_row_model->setHeaderData(0, Qt::Vertical, QString());
    //if(primary_key_with_oid)
    //    new_row_model->insertColumn(0);

    dock_widget = new QDockWidget(tr("Insert new data"), this);
    dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock_widget);

    new_row_view = new NewRowTableView;
    new_row_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    new_row_view->setColumnCount(column_list.size());
    new_row_view->setModel(new_row_model);
    dock_widget->setWidget(new_row_view);
    new_row_view->horizontalHeader()->setVisible(false);
    new_row_view->setFixedHeight(new_row_view->rowHeight(0));
    new_row_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(table_view->horizontalScrollBar(), SIGNAL(valueChanged(int)), new_row_view->horizontalScrollBar(), SLOT(setValue(int)));
    connect(new_row_view->horizontalScrollBar(), SIGNAL(valueChanged(int)), table_view->horizontalScrollBar(), SLOT(setValue(int)));
    QShortcut *insert_row_search = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
    connect(insert_row_search, SIGNAL(activated()), SLOT(insertRow()));
    connect(new_row_view, SIGNAL(insertRow()), SLOT(insertRow()));
    QShortcut *shortcut_focus_add_row = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I), this);
    connect(shortcut_focus_add_row, SIGNAL(activated()), new_row_view, SLOT(setFocus()));
    connect(new_row_model, SIGNAL(itemChanged(QStandardItem*)), SLOT(updatePrimaryKeyInfo()));
    connect(table_view->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), new_row_view, SLOT(resizeCells(int,int,int)));
    new_row_view->setEnabled(false);
    new_row_view->show();

    //Initialise the status bar.
    statusBar()->showMessage(tr("Fetching data..."));
    statusBar()->addPermanentWidget(previous_set_button, 0);
    statusBar()->addPermanentWidget(next_set_button, 0);

    //Launch data retrieval as a future object (a different thread).
    defaultView();
}

void TableView::buildTableQuery()
{
    //Construct the SQL query needed to populate the view.
    //Cycle through the column list and cast PostgreSQL
    //time related data types to text. Otherwise, updates
    //don't work too well.
    sql = QLatin1String("SELECT ");
    if(column_list.isEmpty())
        sql.append(QLatin1String("*"));
    else {
        //if(primary_key_with_oid)
        //    sql.append("oid, ");
        column_count = column_list.count();
        for(int column = 0; column < column_count; column++) {
            if(column == column_count-1) {
                if(column_types.value(column).startsWith("time")) {
                    sql.append(current_column_aggregates.value(column)).append("(")
                        .append(column_list.value(column) + ")::text ");
                }
                else if(column_types.value(column).compare("double precision") == 0) {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column) + ")::text ");
                }
                else {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column)).append(") ");
                }

                if(!current_column_aggregates.value(column).isEmpty()) {
                    sql.append("\"" + current_column_aggregates.value(column) + "("
                       + column_list.value(column).remove("\"") + ")\"");
                }
            }
            else {
                if(column_types.value(column).startsWith("time")) {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column) + ")::text");
                }
                else if(column_types.value(column).compare("double precision") == 0) {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column) + ")::text");
                }
                else {
                    sql.append(current_column_aggregates.value(column)).append("(")
                       .append(column_list.value(column) + ")");
                }

                if(!current_column_aggregates.value(column).isEmpty()) {
                    sql.append(" \"" + current_column_aggregates.value(column) + "("
                       + column_list.value(column).remove("\"") + ")\", ");
                }
                else {
                    sql.append(", ");
                }
            }
            QStringList funcs;
            funcs << "";// << "max" << "sum" << "avg";
            column_aggregates.append(funcs);
        }
    }
    sql.append(QLatin1String(" FROM "));
    sql.append(table_name);
}

//Mouse release event should enable/disable actions.
bool TableView::eventFilter(QObject *obj, QEvent *event)
{
    if(table_view->model() == NULL)
        return QMainWindow::eventFilter(obj, event);

    if (obj == table_view->viewport()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            toggleActions();
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::RightButton)
                customContextMenuViewport();
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
        }
        return QMainWindow::eventFilter(obj, event);
    }
    else if(obj == table_view) {
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
        }
        return QMainWindow::eventFilter(obj, event);
    }
    else {
        return QMainWindow::eventFilter(obj, event);
    }
}

//Key release event should enable/disable actions.
void TableView::keyReleaseEvent(QKeyEvent*)
{
    toggleActions();
}

void TableView::fetchDefaultData()
{
    {
        //We want to ensure that we have only one thread acting
        //at a given point in time.
        //If the previous thread is not done with, abort spawning
        //a new thread to avoid the possibility of a crash.
        if(thread_busy)
            return;

        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        buildTableQuery();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        // Test the next batch of offset to set 'can_fetch_more'.
        // This boolean variable determines if we need to fetch more data
        // after current batch.
        if(quick_fetch) {
            QString offset = " OFFSET " + QString::number(FETCHSIZ);
            limit = " LIMIT " + QString::number(FETCHSIZ);

            // Need a temporary query to check if there is more data to be
            // retrieved. Flagged by 'can_fetch_more'.
            QSqlQueryModel temp_query_model;
            temp_query_model.setQuery(sql + limit + offset, database_connection);
            if(temp_query_model.rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.append(" OFFSET 0");
            if(table_model)
                table_model->setQuery(sql + limit + offset_list.last(), database_connection);
            else
                close();
        }
        else {
            table_model->setQuery(sql, database_connection);
        }

        rows_from = 1;
        rows_to = table_model->rowCount();
        column_count = table_model->columnCount();
        emit updRowCntSignal("next");
    }
}

void TableView::fetchRefreshData(QString mode)
{
    {
        //We want to ensure that we have only one thread acting
        //at a given point in time.
        //If the previous thread is not done with, abort spawning
        //a new thread to avoid the possibility of a crash.
        if(thread_busy)
            return;

        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        buildTableQuery();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        table_model->setQuery(table_model->query().lastQuery(), database_connection);
        rows_from = FETCHSIZ * (offset_list.size()-1) + 1;
        rows_to = rows_from + table_model->rowCount() - 1;
        column_count = table_model->columnCount();
        emit updRowCntSignal(mode);
    }
}

void TableView::busySlot()
{
    toolbar->setEnabled(false);
    thread_busy = true;
    new_row_view->setEnabled(false);
    t.start();
    setCursor(Qt::WaitCursor);
}

void TableView::updRowCntSlot(QString dataset)
{
    time_elapsed_string = tr("Time elapsed:");
    rows_string = tr("Rows:");
    rows_string_2 = tr(" of whole set");
    colums_string = tr("Columns:");
    seconds_string = tr("s");

    if(table_model->lastError().isValid()) {
        queryFailed(table_model->lastError().databaseText());
        statusBar()->showMessage(QLatin1String(""));
    }
    else {
        table_model->setRowsFrom(rows_from);
        table_view->setModel(table_model);

        if(table_view->horizontalHeader() != combo_header) {
            combo_header = new ComboHeader(this);

            connect(table_view->horizontalScrollBar(), SIGNAL(valueChanged(int)), combo_header, SLOT(fixComboPositions()));
            connect(this, SIGNAL(functionsUpdated()), combo_header, SLOT(refreshCombos()));

            table_view->horizontalHeader()->setVisible(false);
            table_view->setHorizontalHeader(combo_header);
            table_view->horizontalHeader()->setVisible(true);
        }

        int column_count = table_model->columnCount();
        for(int column = 0; column < column_count; column++) {
            table_view->showColumn(column);
            new_row_view->showColumn(column);
        }
        if(dataset.compare("previous") == 0)
            table_view->scrollToBottom();
        else if(dataset.compare("next") == 0)
            table_view->scrollToTop();
        time_elapsed = (double)t.elapsed()/1000;

        if(rows_to == 0) {
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " +  rows_string + "0" +
                                     " \t " + colums_string + QString::number(column_count));
            new_row_view->verticalHeader()->setVisible(false);
            previous_set_button->setEnabled(false);
            next_set_button->setEnabled(false);
        }
        else {
            new_row_view->verticalHeader()->setVisible(true);
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

            //Align insert tableview to main tableview by
            //setting the correct header width (ugly hack).
            int number_of_digits = 0;
            int number_of_rows = rows_to;
            while(number_of_rows != 0) {
                number_of_rows /= 10;
                number_of_digits++;
            }
            new_row_model->setHeaderData(0, Qt::Vertical, QString(2*number_of_digits, ' '));
        }

        if(rows_from > 1)
            previous_set_button->setEnabled(true);
        else
            previous_set_button->setEnabled(false);

        updatePrimaryKeyInfo();

        if(table_view->verticalScrollBar()->isVisible())
            new_row_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        else
            new_row_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        if(!where_clause.isEmpty())
            delete_rows_action->setEnabled(true);

        new_row_view->setEnabled(true);
    }
    thread_busy = false;
    status_message = statusBar()->currentMessage();
    setCursor(Qt::ArrowCursor);
    toolbar->setEnabled(true);
}

void TableView::notBusySlot()
{
    thread_busy = false;
    new_row_view->setEnabled(true);

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
}

void TableView::displayErrorMessage(QString error_text)
{
    error_status = true;
    error_message_box->setText(error_text);
    error_message_box->setStandardButtons(QMessageBox::Close);
    error_message_box->setIcon(QMessageBox::Critical);
    error_message_box->show();
}

void TableView::bringOnTop()
{
    activateWindow();
    raise();
}

//Paint primary key columns with a key icon for easy identification
//Paint the background of primary key columns of insert row as red if it is empty
//Paint the background of primary key columns of insert row as green if it is not empty
void TableView::updatePrimaryKeyInfo()
{
    foreach(QString key_element, primary_key) {
        int actual_index = table_model->record().indexOf(key_element.remove("\""));
        if(actual_index > -1) {
            table_model->setHeaderData(actual_index, Qt::Horizontal, key_icon, Qt::DecorationRole);
            QModelIndex idx = new_row_model->index(0,actual_index,QModelIndex());
            if(new_row_model->data(idx).toString().isEmpty() ||
                    new_row_model->data(idx).toString().length() > column_lengths.at(actual_index).toInt())
                new_row_model->setData(idx, red_brush, Qt::BackgroundRole);
            else
                new_row_model->setData(idx, green_brush, Qt::BackgroundRole);
        }
    }
    /*
    for(int column = 0; column < column_count; column++) {
        if(column_lengths.at(column).compare("0") == 0)
            continue;
        QModelIndex idx = new_row_model->index(0,column,QModelIndex());
        if(new_row_model->data(idx).toString().isEmpty()) {
            continue;
        }
        if(new_row_model->data(idx).toString().length() > column_lengths.at(column).toInt()) {
            new_row_model->setData(idx, red_brush, Qt::BackgroundRole);
            new_row_model->setData(idx, tr("Length more than %1").arg(column_lengths.at(column)), Qt::ToolTipRole);
        }
        else
            new_row_model->setData(idx, QBrush(Qt::white, Qt::SolidPattern), Qt::BackgroundRole);
    }*/
}

void TableView::fetchNextData()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        buildTableQuery();

        QSqlDatabase::removeDatabase("tableview " + sql + QString::number(thisTableViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "tableview " + sql + QString::number(thisTableViewId));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        // Test the next batch of offset to set 'can_fetch_more'.
        // This boolean variable determines if we need to fetch more data
        // after current batch.
        QSqlQueryModel temp_query_model;
        QString offset = " OFFSET " + QString::number((offset_list.size()+1)*FETCHSIZ);
        if(where_clause.isEmpty())
            temp_query_model.setQuery(sql
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : QString("")) + limit + offset, database_connection);
        else
            temp_query_model.setQuery(sql + " WHERE " + where_clause.join(" AND ")
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : QString("")) + limit + offset, database_connection);
        if(temp_query_model.rowCount() == 0)
            can_fetch_more = false;
        else
            can_fetch_more = true;
        rows_from = offset_list.size()*FETCHSIZ + 1;
        offset_list.append(" OFFSET " + QString::number(rows_from - 1));
        if(where_clause.isEmpty())
            table_model->setQuery(sql
                                  + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                  + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : QString("")) + limit + offset_list.last(), database_connection);
        else
            table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ")
                                  + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                  + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : QString("")) + limit + offset_list.last(), database_connection);

        rows_to = rows_from + table_model->rowCount() - 1;
        column_count = table_model->columnCount();

        emit updRowCntSignal("next");
    }
}

void TableView::fetchPreviousData()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        buildTableQuery();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        offset_list.removeLast();
        rows_from = (offset_list.size()-1)*FETCHSIZ + 1;
        if(where_clause.isEmpty())
            table_model->setQuery(sql
                                  + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : "")
                                  + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset_list.last(), database_connection);
        else
            table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ")
                                  + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : "")
                                  + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset_list.last(), database_connection);

        rows_to = rows_from + table_model->rowCount() - 1;
        column_count = table_model->columnCount();
        can_fetch_more = true;

        emit updRowCntSignal("previous");
    }
}

void TableView::fetchConditionDataInitial()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        buildTableQuery();

        QSqlDatabase temp_database_connection;
        temp_database_connection = QSqlDatabase::addDatabase("QPSQL", "temp tableview" + sql + QString::number(thisTableViewId));
        temp_database_connection.setHostName(database->getHost());
        temp_database_connection.setPort(database->getPort().toInt());
        temp_database_connection.setDatabaseName(database->getName());
        temp_database_connection.setUserName(database->getUser());
        temp_database_connection.setPassword(database->getPassword());
        if (!temp_database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        // Test the next batch of offset to set 'can_fetch_more'.
        // This boolean variable determines if we need to fetch more data
        // after current batch.
        QSqlQueryModel temp_query_model;
        QString offset = " OFFSET " + QString::number(offset_list.size()*FETCHSIZ);
        if(where_clause.isEmpty())
            temp_query_model.setQuery(sql
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset, temp_database_connection);
        else
            temp_query_model.setQuery(sql + " WHERE " + where_clause.join(" AND ")
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset, temp_database_connection);
        if(temp_query_model.lastError().isValid()) {
            queryFailed(temp_query_model.lastError().databaseText());
            if(where_clause.count() > 0)
                where_clause.removeLast();
            emit notBusySignal();
        }
        else {
            QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
            QSqlDatabase database_connection;
            database_connection = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
            database_connection.setHostName(database->getHost());
            database_connection.setPort(database->getPort().toInt());
            database_connection.setDatabaseName(database->getName());
            database_connection.setUserName(database->getUser());
            database_connection.setPassword(database->getPassword());
            if (!database_connection.open()) {
                qDebug() << tr("Couldn't connect to database.\n"
                             "Check connection parameters.\n");
                return;
            }

            if(temp_query_model.rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            rows_from = (offset_list.size()-1)*FETCHSIZ + 1;
            if(where_clause.isEmpty())
                table_model->setQuery(sql
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset_list.last(), database_connection);
            else
                table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ")
                                      + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : QString(""))
                                      + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "") + limit + offset_list.last(), database_connection);
            if(table_model->lastError().isValid()) {

            }
            else {
                rows_to = rows_from + table_model->rowCount() - 1;
                column_count = table_model->columnCount();
            }
            emit updRowCntSignal("next");
        }
    }
    QSqlDatabase::removeDatabase("temp tableview" + sql + QString::number(thisTableViewId));
}

void TableView::fetchDataSlot()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(table_model->rowCount() >= FETCHSIZ &&
        table_view->verticalScrollBar()->value() == table_view->verticalScrollBar()->maximum()) {
        if(can_fetch_more) {
            statusBar()->showMessage(tr("Fetching data..."));
            QtConcurrent::run(this, &TableView::fetchNextData);
        }
    }
    //Check if vertical scrollbar is at the top-most position to trigger
    //fetching of previous dataset from database. Data retrieval launched as a
    //future object (separate thread).
    else if(rows_from > 1 &&
            table_view->verticalScrollBar()->value() == table_view->verticalScrollBar()->minimum()) {
        statusBar()->showMessage(tr("Fetching data..."));
        QtConcurrent::run(this, &TableView::fetchPreviousData);
    }
}

void TableView::closeEvent(QCloseEvent *event)
{
    event->accept();
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //TableView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    emit tableViewClosing(this);

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("tableview_maximized", true);
        showNormal();
    }
    else
        settings.setValue("tableview_maximized", false);

    settings.setValue("tableview_pos", pos());
    settings.setValue("tableview_size", size());
    settings.setValue("icon_size", toolbar->iconSize());

    if(!thread_busy) {
        delete toolbar;
        delete table_view;
        delete table_model;
        delete new_row_view;
        delete new_row_model;
        delete dock_widget;
        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QMainWindow::closeEvent(event);
    }
    else
        hide();
}

void TableView::copyToClipboard()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
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
        QVariant data = table_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(table_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void TableView::copyToClipboardWithHeaders()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage(tr("Nothing copied"));
        return;
    }
    qSort(indices);
    QString headerText;
    QModelIndex current;
    int prevRow = indices.at(0).row();
    foreach(current, indices) {
        if(current.row() == prevRow) {
            QVariant data = table_view->model()->headerData(current.column(), Qt::Horizontal);
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
        QVariant data = table_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(table_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void TableView::removeColumns()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage("No column(s) removed");
        return;
    }
    qSort(indices);
    int current_row = indices.at(0).row();
    int i = 0;
    while(indices.at(i).isValid()) {
        int column = indices.at(i).column();
        table_view->hideColumn(column);
        new_row_view->hideColumn(column);
        i++;
        if(i >= indices.size())
            break;
        if(indices.at(i).row() != current_row)
            break;
    }
}

void TableView::customFilterReturnPressed()
{
    filter(filter_text->text());
    filter_text->clear();
}

void TableView::bulkUpdate()
{
    statusBar()->showMessage(tr("Updating data..."));
    status_message = statusBar()->currentMessage();
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage(tr("Nothing selected"));
        return;
    }

    QVariant data;
    bool ok = true;
    if(table_model->data(indices.first()).type() == QMetaType::Double)
        data.setValue(bulk_update->text().toDouble(&ok));
    else if(table_model->data(indices.first()).type() == QMetaType::Int)
        data.setValue(bulk_update->text().toInt(&ok));
    else if(table_model->data(indices.first()).type() == QMetaType::Long)
        data.setValue(bulk_update->text().toLong(&ok));
    else
        data.setValue(bulk_update->text());

    if(ok == false)
        emit queryFailed(tr("Couldn't convert input \"%1\" to a number.").arg(bulk_update->text()));
    else
        QtConcurrent::run(this, &TableView::bulkUpdateData, table_view->selectionModel()->selectedIndexes(), data);
    context_menu.hide();
    bulk_update->clear();
}

void TableView::bulkUpdateData(QModelIndexList indices, QVariant data)
{
    //We want to ensure that we have only one thread acting
    //at a given point in time.
    //If the previous thread is not done with, abort spawning
    //a new thread to avoid the possibility of a crash.
    if(thread_busy)
        return;

    //Indicate that we are going to be retrieving data and busy.
    emit busySignal();
    qSort(indices);

    QModelIndex index;

    if((indices.size() > 1) && !can_fetch_more &&
       (indices.first().column() == indices.last().column()))
    {
        foreach(index, indices) {
            if(error_status)
                break;
            table_model->setData(index, data, Qt::EditRole);
        }
    }
    emit notBusySignal();
}

void TableView::defaultView()
{
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    where_clause.clear();
    offset_list.clear();
    group_clause.clear();
    order_clause.clear();
    error_status = false;

    current_column_aggregates.clear();
    column_aggregates.clear();
    for(int i=0; i<column_list.length(); i++) {
        current_column_aggregates << QString("");
        column_aggregates.append(QStringList() << "");
    }

    QtConcurrent::run(this, &TableView::fetchDefaultData);

    emit functionsUpdated();

    disableActions();
}

void TableView::refreshView()
{
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    error_status = false;
    QtConcurrent::run(this, &TableView::fetchRefreshData, QString("next"));
    disableActions();
}

void TableView::addRowRefreshView()
{
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    QtConcurrent::run(this, &TableView::fetchRefreshData, QString("previous"));
    disableActions();
}

void TableView::filter()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    QVariant header = table_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    header_string = column_list.at(index.column());
    QVariant data = table_view->model()->data(index);
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
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::filter(QString filter)
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));
    status_message = statusBar()->currentMessage();
    QVariant header = table_view->model()->headerData(index.column(), Qt::Horizontal);
    //QString header_string = header.toString();
    //where_clause.append("\"" + header_string + "\" " + filter);
    QString item = column_list.at(index.column());
    where_clause.append(item + " " + filter);
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    context_menu.hide();
}

void TableView::exclude()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QVariant header = table_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    QVariant data = table_view->model()->data(index);
    if(data.isNull()) {
        where_clause.append("\"" + header_string + "\" IS NOT NULL");
    }
    else
        if(data.type() == QMetaType::Int || data.type() == QMetaType::Long)
            where_clause.append("\"" + header_string + "\"<>" + data.toString());
        else
            where_clause.append("\"" + header_string + "\"<>'" + data.toString() + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::ascend()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QVariant header = table_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains("\"" + header_string + "\" DESC"))
        order_clause.removeOne("\"" + header_string + "\" DESC");
    if(!order_clause.contains("\"" + header_string + "\" ASC")) {
        order_clause.append("\"" + header_string + "\" ASC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    }
}

void TableView::descend()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QVariant header = table_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains("\"" + header_string + "\" ASC"))
        order_clause.removeOne("\"" + header_string + "\" ASC");
    if(!order_clause.contains("\"" + header_string + "\" DESC")) {
        order_clause.append("\"" + header_string + "\" DESC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    }
}

void TableView::group()
{
    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(tr("Fetching data..."));

    QString header_string = column_list.at(index.column());

    if(!group_clause.contains(header_string)) {
        group_clause.append(header_string);
        offset_list.clear();
        offset_list.append(" OFFSET 0");

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
                    column_types.at(i).contains("time")) {
                current_column_aggregates.replace(i, "count");
                QStringList fs;
                fs << "count" << "min" << "max";
                column_aggregates.append(fs);
            }
            else if(column_types.at(i).contains("int") ||
                    column_types.at(i).startsWith("real") ||
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

        QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    }
    emit functionsUpdated();
}

void TableView::regroup(QStringList aggs)
{
    offset_list.clear();
    offset_list.append(" OFFSET 0");

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

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::window()
{

}

void TableView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        time_elapsed_string = tr("Time elapsed:");
        rows_string = tr("Rows:");
        rows_string_2 = tr(" of whole set");
        colums_string = tr("Columns:");
        seconds_string = tr("s");

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
        dock_widget->setWindowTitle(tr("Insert new data"));
        deselect_menu.setTitle(tr("Remove filter"));
        deselect_menu.setStatusTip(tr("Remove filter"));
        disarrange_menu.setTitle(tr("Remove order"));
        disarrange_menu.setStatusTip(tr("Remove order"));
        ungroup_menu.setTitle(tr("Remove group"));
        ungroup_menu.setStatusTip(tr("Remove group"));

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
        group_action->setText(tr("Group"));
        group_action->setStatusTip(tr("Group this column by this item value"));
        ascend_action->setText(tr("Ascending order"));
        ascend_action->setStatusTip(tr("Arrange items of this column in ascending order"));
        descend_action->setText(tr("Descending order"));
        descend_action->setStatusTip(tr("Arrange items of this column in descending order"));
        remove_all_filters_action->setText(tr("All filters"));
        remove_all_filters_action->setStatusTip(tr("Remove all filters"));
        remove_all_grouping_action->setText(tr("All grouping"));
        remove_all_grouping_action->setStatusTip(tr("Remove all grouping"));
        remove_all_ordering_action->setText(tr("All ordering"));
        remove_all_ordering_action->setStatusTip(tr("Remove all ordering"));
        truncate_action->setText(tr("Clear table"));
        truncate_action->setStatusTip(tr("Delete the contents of the table"));
        custom_filter_action->setText(tr("Custom filter"));
        custom_filter_action->setStatusTip(tr("Custom filter"));
        bulk_update_action->setText(tr("Bulk update"));
        bulk_update_action->setStatusTip(tr("Bulk update"));
        copy_query_action->setText(tr("Copy query"));
        copy_query_action->setStatusTip(tr("Copy the query to clipboard"));

        filter_text->setPlaceholderText(tr("custom filter"));
        bulk_update->setPlaceholderText(tr("bulk update"));
    }
}

void TableView::customContextMenuViewport()
{
    if(thread_busy)
        return;

    filter_text->clear();
    bulk_update->clear();
    context_menu.clear();

    QModelIndexList indices = table_view->selectionModel()->selectedIndexes();
    if(indices.isEmpty())
        return;

    //Need to sort the retrieved indices first.
    qSort(indices);

    QModelIndex index;
    QVariant data;
    int order_clause_size = order_clause.size();
    int group_clause_size = group_clause.size();

    if(indices.size() > 1) {
        if((indices.first().column() == indices.last().column()) &&
            !can_fetch_more &&
            !primary_key.isEmpty())
            context_menu.addAction(bulk_update_action);
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
        data = table_view->model()->data(index);
        if(data.canConvert<QString>()) {
            filter_action->setEnabled(true);
            exclude_action->setEnabled(true);
            group_action->setEnabled(true);
            ascend_action->setEnabled(true);
            descend_action->setEnabled(true);
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
            if(table_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
            return;
        }
    }

    else if(a->icon().cacheKey() == group_icon.cacheKey()) {
        QString item_for_removal = a->text();
        if((status = group_clause.indexOf(item_for_removal)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            group_clause.removeAt(status);

            if(table_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            column_aggregates.clear();
            if(group_clause.size() == 0) {
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
                            column_types.at(i).contains("time")) {
                        current_column_aggregates.replace(i, "count");
                        QStringList fs;
                        fs << "count" << "min" << "max";
                        column_aggregates.append(fs);
                    }
                    else if(column_types.at(i).contains("int") ||
                            column_types.at(i).startsWith("real") ||
                            column_types.at(i).startsWith("numeric")) {
                        current_column_aggregates.replace(i, "sum");
                        QStringList fs;
                        fs << "sum" << "count" << "min" << "max" << "avg";
                        column_aggregates.append(fs);
                    }
                }
            }

            QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
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
            if(table_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
            return;
        }
    }
    else if(a->icon().cacheKey() == descend_icon.cacheKey()) {
        QString order = a->text();
        order.append(" DESC");
        if((status = order_clause.indexOf(order)) != -1) {
            statusBar()->showMessage(tr("Fetching data..."));

            order_clause.removeAt(status);
            if(table_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
            return;
        }
    }
}

void TableView::customContextMenuHeader()
{
    return;
    if(thread_busy)
        return;
    context_menu.clear();

    QModelIndexList indices = table_view->selectionModel()->selectedRows();
    if(indices.isEmpty()) {
        return;
    }

    context_menu.addAction(delete_rows_action);
    context_menu.exec(QCursor::pos());
}

void TableView::removeAllFilters()
{
    statusBar()->showMessage(tr("Fetching data..."));
    where_clause.clear();

    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::removeAllGrouping()
{
    column_aggregates.clear();
    for(int i=0; i < current_column_aggregates.length(); i++) {
        current_column_aggregates.replace(i, "");
        column_aggregates.append(QStringList() << "");
    }

    statusBar()->showMessage(tr("Fetching data..."));

    group_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    emit functionsUpdated();
}

void TableView::removeAllOrdering()
{
    statusBar()->showMessage(tr("Fetching data..."));

    order_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::copyQuery()
{
    QString copy_sql;

    if(where_clause.isEmpty())
        copy_sql = sql.remove("::text")
                 + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : "")
                 + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "");
    else
        copy_sql = sql.remove("::text") + " WHERE " + where_clause.join(" AND ")
                 + (group_clause.size() > 0 ? QString(" GROUP BY " + group_clause.join(",")) : "")
                 + (order_clause.size() > 0 ? QString(" ORDER BY " + order_clause.join(",")) : "");

    qApp->clipboard()->setText(copy_sql);
}

bool TableView::insertRow()
{
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QLatin1String("insert ") + objectName());
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                   "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return false;
        }

        QStringList fields;
        QStringList bind_fields;
        QStringList values;
        QSqlRecord rec = table_model->record();

        for (int column = 0; column < column_count; ++column) {
            QVariant cell_value = new_row_model->item(0, column)->data(Qt::EditRole);
            if(!cell_value.toString().isEmpty()) {
                fields.append("\"" + rec.fieldName(column) + "\"");
                bind_fields.append("?");
                values.append(cell_value.toString());
            }
        }
        QString field_string = fields.join(",");
        QString bind_field_string = bind_fields.join(",");

        QString query_string = QLatin1String("INSERT INTO ");
        query_string.append(table_name);
        query_string.append(QLatin1String("("));
        query_string.append(field_string);
        query_string.append(QLatin1String(") VALUES ("));
        query_string.append(bind_field_string);
        query_string.append(QLatin1String(")"));

        QSqlQuery query(database_connection);
        query.prepare(query_string);
        foreach(QString value, values)
            query.addBindValue(value);
        query.exec();
        if(query.lastError().isValid()) {
            QMessageBox::critical(this, tr("Database error"),
            query.lastError().databaseText(), QMessageBox::Close);
            statusBar()->showMessage(tr("Error: Couldn't insert data"), 1000);
        }
        else {
            statusBar()->showMessage(tr("Inserted data successfully"), 1000);
            for (int column = 0; column < column_count; ++column)
                new_row_model->item(0, column)->setData("", Qt::EditRole);
            if(!can_fetch_more) {
                addRowRefreshView();
            }
        }
    }
    QSqlDatabase::removeDatabase(QLatin1String("insert ") + objectName());
    new_row_view->setFocus();
    QModelIndex idx = new_row_model->index(0, 0, QModelIndex());
    new_row_view->edit(idx);
    return true;
}

void TableView::truncateTable()
{
    QMessageBox *warning = new QMessageBox(this);
    //warning->setWindowFlags(Qt::FramelessWindowHint);
    warning->setText(tr("This action will destroy all data in this table and cannot be undone.\n"
                        "Do you want to continue?"));
    warning->setWindowTitle(QLatin1String("pgXplorer"));
    warning->addButton(QMessageBox::Ok);
    warning->addButton(QMessageBox::Cancel);
    warning->setIcon(QMessageBox::Warning);

    if(warning->exec() == QMessageBox::Cancel)
        return;
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "clear " + objectName());
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                   "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery query(QString("TRUNCATE TABLE ").append(objectName()), database_connection);
        query.exec();
        if(query.lastError().isValid()) {
            //QMessageBox::critical(0, tr("Database error"),
            //query.lastError().databaseText(), QMessageBox::Close);
        }
        statusBar()->showMessage(tr("All table rows deleted"));
    }
    QSqlDatabase::removeDatabase("clear " + objectName());
    defaultView();
}

void TableView::deleteRows()
{
    QMessageBox *warning = new QMessageBox(this);
    warning->setText(tr("This action will delete this data and cannot be undone.\n"
                        "Do you want to continue?"));
    warning->setWindowTitle(QLatin1String("pgXplorer"));
    warning->addButton(QMessageBox::Ok);
    warning->addButton(QMessageBox::Cancel);
    warning->setIcon(QMessageBox::Warning);
    if(warning->exec() == QMessageBox::Cancel)
        return;
    statusBar()->showMessage(tr("Deleting data..."));
    QtConcurrent::run(this, &TableView::deleteData);
}

void TableView::deleteData()
{
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "clear " + objectName());
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QString delete_query("DELETE FROM ");
        delete_query.append(objectName());
        delete_query.append(" WHERE " + where_clause.join(" AND "));
        QSqlQuery query(delete_query, database_connection);

        query.exec();
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
        //statusBar()->showMessage(tr("All table rows deleted"));
    }
    QSqlDatabase::removeDatabase("clear " + objectName());
    fetchConditionDataInitial();
}

void TableView::deleteRow(int row)
{
    {
        //We want to ensure that we have only one thread acting
        //at a given point in time.
        //If the previous thread is not done with, abort spawning
        //a new thread to avoid the possibility of a crash.
        if(thread_busy)
            return;

        //Indicate that we are going to be deleting data and busy.
        emit busySignal();

        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("delete ") + objectName() + QString::number(thisTableViewId));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QString delete_query("DELETE FROM ");
        delete_query.append(objectName());
        //delete_query.append(" WHERE ");
        QSqlQuery query(delete_query, database_connection);
        query.exec();
        if(query.lastError().isValid()) {
            //qDebug() << query.lastError().databaseText();
        }
    }
    QSqlDatabase::removeDatabase("delete " + objectName() + QString::number(thisTableViewId));
}

void TableView::fullscreen()
{
    this->showFullScreen();
}

void TableView::restore()
{
    this->showNormal();
}

void TableView::toggleActions()
{
    if(table_view->selectionModel()) {
        //Only enable filter, exclude and ordering actions
        //when a single cell is selected.
        if(table_view->selectionModel()->selectedIndexes().isEmpty()) {
            copy_action->setEnabled(false);
            copy_with_headers_action->setEnabled(false);
        }
        else {
            copy_action->setEnabled(true);
            copy_with_headers_action->setEnabled(true);
            if(table_view->selectionModel()->selectedIndexes().size() == 1)
                enableActions();
            else
                disableActions();
        }
    }
}

void TableView::enableActions()
{
    if(!thread_busy) {
        filter_action->setEnabled(true);
        exclude_action->setEnabled(true);
        group_action->setEnabled(true);
        ascend_action->setEnabled(true);
        descend_action->setEnabled(true);
    }
}

void TableView::disableActions()
{
    //if(tview->selectionModel()->selectedIndexes().isEmpty()) {
    //    copy_action->setEnabled(false);
    //    copy_with_headers_action->setEnabled(false);
    //}
    filter_action->setEnabled(false);
    exclude_action->setEnabled(false);
    ascend_action->setEnabled(false);
    descend_action->setEnabled(false);
    delete_rows_action->setEnabled(false);
}

void TableView::createBrushes()
{
    QLinearGradient red_lineargradient(0, 0, 1.0, 0.25);
    red_lineargradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    red_lineargradient.setColorAt(0, QColor::fromRgb(0xDE,0x00,0x00,160));
    red_lineargradient.setColorAt(1, QColor::fromRgb(0xEF,255,255,160));
    red_brush = QBrush(red_lineargradient);

    QLinearGradient green_lineargradient(0, 0, 1.0, 0.25);
    green_lineargradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    green_lineargradient.setColorAt(0, QColor::fromRgb(0x00,0xDE,0x00,160));
    green_lineargradient.setColorAt(1, QColor::fromRgb(255,0xEF,255,160));
    green_brush = QBrush(green_lineargradient);
}

void TableView::createActions()
{
    default_action = new QAction(QIcon(":/icons/table.png"), tr("Default"), this);
    default_action->setShortcut(QKeySequence("Ctrl+D"));
    default_action->setStatusTip(tr("Default"));
    connect(default_action, SIGNAL(triggered()), SLOT(defaultView()));

    refresh_action = new QAction(QIcon(":/icons/refresh.png"), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    refresh_action->setStatusTip(tr("Refresh"));
    connect(refresh_action, SIGNAL(triggered()), SLOT(refreshView()));

    copy_action = new QAction(QIcon(":/icons/copy_without_headers.png"), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected"));
    //copy_action->setEnabled(false);
    connect(copy_action, SIGNAL(triggered()), SLOT(copyToClipboard()));

    copy_with_headers_action = new QAction(QIcon(":/icons/copy_with_headers.png"), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    //copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, SIGNAL(triggered()), SLOT(copyToClipboardWithHeaders()));

    remove_columns_action = new QAction(QIcon(":/icons/removecolumn.png"), tr("Remove column(s)"), this);
    remove_columns_action->setStatusTip(tr("Removes the column from this display."));
    connect(remove_columns_action, SIGNAL(triggered()), SLOT(removeColumns()));

    filter_action = new QAction(filter_icon, tr("Filter"), this);
    filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
    filter_action->setEnabled(false);
    connect(filter_action, SIGNAL(triggered()), SLOT(filter()));

    exclude_action = new QAction(exclude_icon, tr("Exclude"), this);
    exclude_action->setStatusTip(tr("Filter table exclusive of selected cell value on column"));
    exclude_action->setEnabled(false);
    connect(exclude_action, SIGNAL(triggered()), SLOT(exclude()));

    group_action = new QAction(group_icon, tr("Group"), this);
    group_action->setStatusTip(tr("Group"));
    group_action->setEnabled(false);
    connect(group_action, SIGNAL(triggered()), SLOT(group()));

    window_action = new QAction(group_icon, tr("Window"), this);
    window_action->setStatusTip(tr("Window function"));
    window_action->setEnabled(false);
    connect(window_action, SIGNAL(triggered()), SLOT(window()));

    ascend_action = new QAction(ascend_icon, tr("Ascending order"), this);
    ascend_action->setStatusTip(tr("Ascending order"));
    ascend_action->setEnabled(false);
    connect(ascend_action, SIGNAL(triggered()), SLOT(ascend()));

    descend_action = new QAction(descend_icon, tr("Descending order"), this);
    descend_action->setStatusTip(tr("Descending order"));
    descend_action->setEnabled(false);
    connect(descend_action, SIGNAL(triggered()), SLOT(descend()));

    remove_all_filters_action = new QAction(tr("All filters"), this);
    remove_all_filters_action->setStatusTip(tr("Remove all filters"));
    connect(remove_all_filters_action, SIGNAL(triggered()), SLOT(removeAllFilters()));

    remove_all_grouping_action = new QAction(tr("All grouping"), this);
    remove_all_grouping_action->setStatusTip(tr("Remove all grouping"));
    connect(remove_all_grouping_action, SIGNAL(triggered()), SLOT(removeAllGrouping()));

    remove_all_ordering_action = new QAction(tr("All ordering"), this);
    remove_all_ordering_action->setStatusTip(tr("Remove all ordering"));
    connect(remove_all_ordering_action, SIGNAL(triggered()), SLOT(removeAllOrdering()));

    custom_filter_action = new QWidgetAction(this);
    custom_filter_action->setStatusTip(tr("Custom filter"));
    custom_filter_action->setIcon(filter_icon);
    custom_filter_action->setDefaultWidget(filter_text);
    connect(filter_text, SIGNAL(returnPressed()), SLOT(customFilterReturnPressed()));

    bulk_update_action = new QWidgetAction(this);
    bulk_update_action->setStatusTip(tr("Bulk update"));
    //bulk_update_action->setIcon(filter_icon);
    bulk_update_action->setDefaultWidget(bulk_update);
    connect(bulk_update, SIGNAL(returnPressed()), SLOT(bulkUpdate()));

    copy_query_action = new QAction(QIcon(":/icons/copy_sql.png"), tr("Copy query"), this);
    copy_query_action->setStatusTip(tr("Copy the query to clipboard"));
    connect(copy_query_action, SIGNAL(triggered()), SLOT(copyQuery()));

    truncate_action = new QAction(QIcon(":/icons/truncate.png"), tr("Clear table"), this);
    truncate_action->setStatusTip(tr("Delete the contents of the table"));
    connect(truncate_action, SIGNAL(triggered()), SLOT(truncateTable()));

    delete_rows_action = new QAction(QIcon(":/icons/delete_rows.png"), tr("Delete row(s)"), this);
    delete_rows_action->setEnabled(false);
    delete_rows_action->setStatusTip(tr("Delete the selected row(s) of the table"));
    connect(delete_rows_action, SIGNAL(triggered()), SLOT(deleteRows()));

    previous_set_action = new QAction(QIcon(":/icons/previous.png"), "", this);
    previous_set_action->setToolTip(tr("Fetch previous set"));
    connect(previous_set_action, SIGNAL(triggered()), SLOT(fetchPreviousData()));
    previous_set_button = new QToolButton;
    previous_set_button->setDefaultAction(previous_set_action);
    previous_set_button->setEnabled(false);

    next_set_action = new QAction(QIcon(":/icons/next.png"), "", this);
    next_set_action->setToolTip(tr("Fetch next set"));
    connect(next_set_action, SIGNAL(triggered()), SLOT(fetchNextData()));
    next_set_button = new QToolButton;
    next_set_button->setDefaultAction(next_set_action);
    next_set_button->setEnabled(false);
}
