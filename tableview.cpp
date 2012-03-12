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
        else if(key_event->key() == Qt::Key_Tab)
        {
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

    error_message_box = new QMessageBox(this);
    //error_message_box->setWindowFlags(Qt::FramelessWindowHint);

    this->database = database;
    this->table_name = table_name;
    this->primary_key = primary_key;
    this->column_list = column_list;
    this->column_types = column_types;
    this->column_lengths = column_lengths;

    filter_text = new QLineEdit;
    filter_text->setPlaceholderText(tr("custom filter"));

    createIcons();
    createBrushes();
    createActions();
    setWindowTitle(name);
    setObjectName(name);

    toolbar = new QToolBar;
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

    addToolBar(toolbar);
    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisTableViewId = tableViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have TableView
    //GUI artifacts to create overlapping threads yet.
    thread_busy = false;
    table_model = new TableModel(database, this->primary_key, this->table_name);

    //Quick fetch feature that is supposed to fetch only a fixed
    //number of rows of a large table to populate the view.
    //For now, we will always enable Quick fetch.
    quick_fetch = true;

    //Construct the SQL query needed to populate the view.
    //Cycle through the column list and cast PostgreSQL
    //time related data types to text. Otherwise, updates
    //don't work too well.
    sql = QString("SELECT ");
    column_count = column_list.count();
    for(int column = 0; column < column_count; column++) {
        if(column == column_count-1) {
            if(column_types.value(column).startsWith("time"))
                sql.append(column_list.value(column) + "::text");
            else if(column_types.value(column).compare("double precision") == 0)
                sql.append(column_list.value(column) + "::text");
            else
                sql.append(column_list.value(column));
        }
        else {
            if(column_types.value(column).startsWith("time"))
                sql.append(column_list.value(column) + "::text, ");
            else if(column_types.value(column).compare("double precision") == 0)
                sql.append(column_list.value(column) + "::text, ");
            else
                sql.append(column_list.value(column) + ", ");
        }
    }
    sql.append(" FROM ");
    sql.append(table_name);

    //
    tview = new QTableView(this);
    tview->viewport()->installEventFilter(this);
    tview->installEventFilter(this);
    tview->setStyleSheet("QTableView {font-weight: 400;}");
    tview->setAlternatingRowColors(true);
    tview->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    tview->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //QShortcut *shortcut_copy_with_headers = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    //connect(shortcut_copy_with_headers, SIGNAL(activated()), this, SLOT(copych()));

    //Create key-sequences for fullscreen and restore.
    QShortcut *shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), this, SLOT(fullscreen()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    //Tie vertical scrollbar of TableView to fetch more data
    connect(tview->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchDataSlot()));

    //Tie thread finish to an update slot that refreshes meta-information.
    connect(this, SIGNAL(updRowCntSignal(QString)), this, SLOT(updRowCntSlot(QString)));
    connect(this, SIGNAL(updRowCntSignal(QString)), table_model, SLOT(clearCache()));
    connect(table_model, SIGNAL(updateFailed(QString)), this, SLOT(updateFailedSlot(QString)));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), this, SLOT(busySlot()));

    connect(tview->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(customContextMenuHeader()));

    setCentralWidget(tview);

    new_row_model = new QStandardItemModel(1, column_list.size());
    for (int column = 0; column < column_list.size(); ++column) {
             QStandardItem *item = new QStandardItem(QString(""));
             new_row_model->setItem(0, column, item);
    }
    new_row_model->setHeaderData(0, Qt::Vertical, QString());

    dock_widget = new QDockWidget(tr("Insert new data"), this);
    dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea);
    dock_widget->contextMenuPolicy();
    addDockWidget(Qt::BottomDockWidgetArea, dock_widget);

    new_row_view = new NewRowTableView;
    new_row_view->setColumnCount(column_list.size());
    new_row_view->setModel(new_row_model);
    dock_widget->setWidget(new_row_view);
    new_row_view->horizontalHeader()->setVisible(false);
    new_row_view->setFixedHeight(new_row_view->rowHeight(0));
    new_row_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(tview->horizontalScrollBar(), SIGNAL(valueChanged(int)), new_row_view->horizontalScrollBar(), SLOT(setValue(int)));
    connect(new_row_view->horizontalScrollBar(), SIGNAL(valueChanged(int)), tview->horizontalScrollBar(), SLOT(setValue(int)));
    QShortcut *insert_row_search = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
    connect(insert_row_search, SIGNAL(activated()), this, SLOT(insertRow()));
    connect(new_row_view, SIGNAL(insertRow()), this, SLOT(insertRow()));
    QShortcut *shortcut_focus_add_row = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I), this);
    connect(shortcut_focus_add_row, SIGNAL(activated()), new_row_view, SLOT(setFocus()));
    connect(new_row_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updatePrimaryKeyInfo()));
    connect(tview->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), new_row_view, SLOT(resizeCells(int,int,int)));
    new_row_view->setEnabled(false);
    new_row_view->show();

    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    statusBar()->addPermanentWidget(previous_set_button, 0);
    statusBar()->addPermanentWidget(next_set_button, 0);

    //Launch data retrieval as a future object (a different thread).
    defaultView();
}

//Mouse release event should enable/disable actions.
bool TableView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tview->viewport()) {
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
    else if(obj == tview) {
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

/*
void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    filter_text->clear();
    context_menu.clear();

    QString status_message = statusBar()->currentMessage();
    QItemSelectionModel *s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();

    QModelIndex index;
    QVariant data;
    order_clause_size = order_clause.size();

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
        data = tview->model()->data(index);
        if(data.canConvert<QString>()) {
            filter_action->setEnabled(true);
            exclude_action->setEnabled(true);
            ascend_action->setEnabled(true);
            descend_action->setEnabled(true);
        }
    }

    QAction *a = context_menu.exec(QCursor::pos());

    for(int i=0; i<where_clause.size(); i++) {
        if(a && QString::compare(a->text(),where_clause.at(i))==0) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
            where_clause.removeAt(i);
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
            return;
        }
    }

    for(int i=0; i<order_clause.size(); i++) {
        if(!a)
            return;
        QString order = a->text();
        if(a->icon().cacheKey() == ascend_icon.cacheKey())
            order.append(" ASC");
        else if(a->icon().cacheKey() == descend_icon.cacheKey())
            order.append(" DESC");
        if(QString::compare(order, order_clause.at(i))==0) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

            order_clause.removeAt(i);
            order_clause_size--;
            if(query_model->rowCount() == 0)
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
*/
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
            table_model->setQuery(sql + limit + offset_list.last(), database_connection);
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
    thread_busy = true;
    new_row_view->setEnabled(false);
    t.start();
    setCursor(Qt::WaitCursor);
}

void TableView::updRowCntSlot(QString dataset)
{
    time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
    rows_string = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
    rows_string_2 = QApplication::translate("QueryView", " of whole set", 0, QApplication::UnicodeUTF8);
    colums_string = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
    seconds_string = QApplication::translate("QueryView", "s", 0, QApplication::UnicodeUTF8);

    if(table_model->lastError().isValid()) {
        if(where_clause.count() > 0)
            where_clause.removeLast();
        statusBar()->showMessage(tr("Error: Incorrect filter"));
    }
    else {
        table_model->setRowsFrom(rows_from);
        tview->setModel(table_model);
        int column_count = table_model->columnCount();
        for(int column = 0; column < column_count; column++) {
            tview->showColumn(column);
            new_row_view->showColumn(column);
        }
        if(dataset.compare("previous") == 0)
            tview->scrollToBottom();
        else if(dataset.compare("next") == 0)
            tview->scrollToTop();
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

        if(tview->verticalScrollBar()->isVisible())
            new_row_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        else
            new_row_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        if(!where_clause.isEmpty())
        {
            delete_rows_action->setEnabled(true);
        }
        new_row_view->setEnabled(true);
    }
    thread_busy = false;
    status_message = statusBar()->currentMessage();
    setCursor(Qt::ArrowCursor);
}

void TableView::updateFailedSlot(QString error_text)
{
    error_message_box->setText(error_text);
    error_message_box->setStandardButtons(QMessageBox::Close);
    error_message_box->show();
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
            temp_query_model.setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset, database_connection);
        else
            temp_query_model.setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset, database_connection);
        if(temp_query_model.rowCount() == 0)
            can_fetch_more = false;
        else
            can_fetch_more = true;
        rows_from = offset_list.size()*FETCHSIZ + 1;
        offset_list.append(" OFFSET " + QString::number(rows_from - 1));
        if(where_clause.isEmpty())
            table_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);

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
            table_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);

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
        QSqlQueryModel temp_query_model;
        QString offset = " OFFSET " + QString::number(offset_list.size()*FETCHSIZ);
        if(where_clause.isEmpty())
            temp_query_model.setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset, database_connection);
        else
            temp_query_model.setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset, database_connection);
        if(temp_query_model.rowCount() == 0)
            can_fetch_more = false;
        else
            can_fetch_more = true;
        rows_from = (offset_list.size()-1)*FETCHSIZ + 1;
        if(where_clause.isEmpty())
            table_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            table_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        rows_to = rows_from + table_model->rowCount() - 1;
        column_count = table_model->columnCount();
        emit updRowCntSignal("next");
    }
}

void TableView::fetchDataSlot()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(table_model->rowCount() >= FETCHSIZ &&
        tview->verticalScrollBar()->value() == tview->verticalScrollBar()->maximum()) {
        if(can_fetch_more) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
            QtConcurrent::run(this, &TableView::fetchNextData);
        }
    }
    //Check if vertical scrollbar is at the top-most position to trigger
    //fetching of previous dataset from database. Data retrieval launched as a
    //future object (separate thread).
    else if(rows_from > 1 &&
            tview->verticalScrollBar()->value() == tview->verticalScrollBar()->minimum()) {
        statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
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

    if(!thread_busy)
    {
        delete toolbar;
        delete tview;
        delete table_model;
        delete new_row_view;
        delete new_row_model;
        delete dock_widget;
        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QMainWindow::closeEvent(event);
    }
    else {
        hide();
    }
}

void TableView::copyToClipboard()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
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
        QVariant data = tview->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(tview->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void TableView::copyToClipboardWithHeaders()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
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
            QVariant data = tview->model()->headerData(current.column(), Qt::Horizontal);
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
        QVariant data = tview->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(tview->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void TableView::removeColumns()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.isEmpty()) {
        statusBar()->showMessage("No column(s) removed");
        return;
    }
    qSort(indices);
    int current_row = indices.at(0).row();
    int i = 0;
    while(indices.at(i).isValid()) {
        int column = indices.at(i).column();
        tview->hideColumn(column);
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
    context_menu.hide();
}

void TableView::defaultView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    where_clause.clear();

    offset_list.clear();
    order_clause.clear();
    QtConcurrent::run(this, &TableView::fetchDefaultData);
    disableActions();
}

void TableView::refreshView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QtConcurrent::run(this, &TableView::fetchRefreshData, QString("next"));
    disableActions();
}

void TableView::addRowRefreshView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QtConcurrent::run(this, &TableView::fetchRefreshData, QString("previous"));
    disableActions();
}

void TableView::filter()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    QVariant data = tview->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        if(!where_clause.contains("\"" + header_string + "\" IS NULL",
                            Qt::CaseInsensitive))
            where_clause.append("\"" + header_string + "\" IS NULL");
    }
    else
        if(typ.compare("int", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header_string + "\"=" + data.toString());
        //else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
        //    where_clause.append("\"" + header_string + "\"='" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header_string + "\"='" + data.toString().replace("'","\\'") + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::filter(QString filter)
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    where_clause.append("\"" + header_string + "\" " + filter);
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    context_menu.hide();
}

void TableView::exclude()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    QVariant data = tview->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        where_clause.append("\"" + header_string + "\" IS NOT NULL");
    }
    else
        if(typ.compare("int", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header_string + "\"<>" + data.toString());
        //else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
        //    where_clause.append("\"" + header_string + "\"<>'" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header_string + "\"<>'" + data.toString() + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::ascend()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains(header_string + " DESC"))
        order_clause.removeOne(header_string + " DESC");
    if(!order_clause.contains(header_string + " ASC")) {
        order_clause.append(header_string + " ASC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    }
}

void TableView::descend()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains(header_string + " ASC"))
        order_clause.removeOne(header_string + " ASC");
    if(!order_clause.contains(header_string + " DESC")) {
        order_clause.append(header_string + " DESC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
    }
}

void TableView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
        rows_string = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
        rows_string_2 = QApplication::translate("QueryView", " of whole set", 0, QApplication::UnicodeUTF8);
        colums_string = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
        seconds_string = QApplication::translate("QueryView", "s", 0, QApplication::UnicodeUTF8);

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
        truncate_action->setText(tr("Clear table"));
        truncate_action->setStatusTip(tr("Delete the contents of the table"));
        custom_filter_action->setText(tr("Custom filter"));
        custom_filter_action->setStatusTip(tr("Custom filter"));
        copy_query_action->setText(tr("Copy query"));
        copy_query_action->setStatusTip(tr("Copy the query to clipboard"));

        filter_text->setPlaceholderText(tr("custom filter"));
    }
}

void TableView::customContextMenuViewport()
{
    if(thread_busy)
        return;
    filter_text->clear();
    context_menu.clear();

    QItemSelectionModel *s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();

    QModelIndex index;
    QVariant data;
    int order_clause_size = order_clause.size();

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
        data = tview->model()->data(index);
        if(data.canConvert<QString>()) {
            filter_action->setEnabled(true);
            exclude_action->setEnabled(true);
            ascend_action->setEnabled(true);
            descend_action->setEnabled(true);
        }
    }

    QAction *a = context_menu.exec(QCursor::pos());

    for(int i=0; i<where_clause.size(); i++) {
        if(a && QString::compare(a->text(),where_clause.at(i))==0) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
            where_clause.removeAt(i);
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

    for(int i=0; i<order_clause.size(); i++) {
        if(!a)
            return;
        QString order = a->text();
        if(a->icon().cacheKey() == ascend_icon.cacheKey())
            order.append(" ASC");
        else if(a->icon().cacheKey() == descend_icon.cacheKey())
            order.append(" DESC");
        if(QString::compare(order, order_clause.at(i))==0) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

            order_clause.removeAt(i);
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

    QModelIndexList indices = tview->selectionModel()->selectedRows();
    if(indices.isEmpty()) {
        return;
    }

    context_menu.addAction(delete_rows_action);
    context_menu.exec(QCursor::pos());
}

void TableView::removeAllFilters()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    where_clause.clear();

    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::removeAllOrdering()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    order_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::copyQuery()
{
    QString copy_sql;

    if(where_clause.isEmpty())
        copy_sql = sql.remove("::text") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"");
    else
        copy_sql = sql.remove("::text") + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"");

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
            QMessageBox::critical(0, tr("Database error"),
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
            QMessageBox::critical(0, tr("Database error"),
            query.lastError().databaseText(), QMessageBox::Close);
            statusBar()->showMessage(tr("Error: Couldn't insert data"), 1000);
        }
        else {
            statusBar()->showMessage(tr("Inserted data successfully"), 1000);
            for (int column = 0; column < column_count; ++column)
                new_row_model->item(0, column)->setData("", Qt::EditRole);
            if(!can_fetch_more)
                addRowRefreshView();
        }
    }
    QSqlDatabase::removeDatabase(QLatin1String("insert ") + objectName());
    return true;
}

void TableView::truncateTable()
{
    QMessageBox *warning = new QMessageBox;
    //warning->setWindowFlags(Qt::FramelessWindowHint);
    warning->setText(tr("This action will destroy all data in this table and cannot be undone.\n"
                        "<html><i>Do you want to continue?</i></html>"));
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
            QMessageBox::critical(0, tr("Database error"),
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
    fetchDefaultData();
}

void TableView::deleteRows()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Deleting data...", 0, QApplication::UnicodeUTF8));
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
            QMessageBox::critical(0, tr("Database error"),
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

        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "delete " + objectName() + thisTableViewId);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QString delete_query("DELETE FROM ");
        delete_query.append(objectName());
        //delete_query.append(" WHERE ");
        qDebug() << delete_query;
        QSqlQuery query(delete_query, database_connection);
        query.exec();
        if(query.lastError().isValid())
            qDebug() << query.lastError().databaseText();
    }
    QSqlDatabase::removeDatabase("delete " + objectName() + thisTableViewId);
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
    //Only enable filter, exclude and ordering actions
    //when a single cell is selected.
    if(tview->selectionModel()->selectedIndexes().isEmpty()) {
        copy_action->setEnabled(false);
        copy_with_headers_action->setEnabled(false);
    }
    else {
        copy_action->setEnabled(true);
        copy_with_headers_action->setEnabled(true);
        if(tview->selectionModel()->selectedIndexes().size() == 1)
            enableActions();
        else
            disableActions();
    }
}

void TableView::enableActions()
{
    if(!thread_busy) {
        filter_action->setEnabled(true);
        exclude_action->setEnabled(true);
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

void TableView::createIcons()
{
    key_icon = QIcon(qApp->applicationDirPath().append("/icons/key.png"));
    filter_icon = QIcon(qApp->applicationDirPath().append("/icons/filter.png"));
    ascend_icon = QIcon(qApp->applicationDirPath().append("/icons/ascending.png"));
    descend_icon = QIcon(qApp->applicationDirPath().append("/icons/descending.png"));
}

void TableView::createBrushes()
{
    QLinearGradient red_lineargradient(0, 0, 0.75, 0.25);
    red_lineargradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    red_lineargradient.setColorAt(0, QColor::fromRgb(0xDE,0x00,0x00,160));
    red_lineargradient.setColorAt(1, QColor::fromRgb(0xEF,255,255,160));
    red_brush = QBrush(red_lineargradient);

    QLinearGradient green_lineargradient(0, 0, 0.75, 0.25);
    green_lineargradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    green_lineargradient.setColorAt(0, QColor::fromRgb(0x00,0xDE,0x00,160));
    green_lineargradient.setColorAt(1, QColor::fromRgb(255,0xEF,255,160));
    green_brush = QBrush(green_lineargradient);
}

void TableView::createActions()
{
    default_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/table.png")), tr("Default"), this);
    default_action->setShortcut(QKeySequence("Ctrl+D"));
    default_action->setStatusTip(tr("Default"));
    connect(default_action, SIGNAL(triggered()), this, SLOT(defaultView()));

    refresh_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/refresh.png")), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    refresh_action->setStatusTip(tr("Refresh"));
    connect(refresh_action, SIGNAL(triggered()), this, SLOT(refreshView()));

    copy_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy.svgz")), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected"));
    //copy_action->setEnabled(false);
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

    copy_with_headers_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy_with_headers.svg")), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    //copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, SIGNAL(triggered()), this, SLOT(copyToClipboardWithHeaders()));

    remove_columns_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/removecolumn.png")), tr("Remove column(s)"), this);
    remove_columns_action->setStatusTip(tr("Removes the column from this display."));
    connect(remove_columns_action, SIGNAL(triggered()), this, SLOT(removeColumns()));

    filter_action = new QAction(filter_icon, tr("Filter"), this);
    filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
    filter_action->setEnabled(false);
    connect(filter_action, SIGNAL(triggered()), this, SLOT(filter()));

    exclude_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/exclude.png")), tr("Exclude"), this);
    exclude_action->setStatusTip(tr("Filter table exclusive of selected cell value on column"));
    exclude_action->setEnabled(false);
    connect(exclude_action, SIGNAL(triggered()), this, SLOT(exclude()));

    ascend_action = new QAction(ascend_icon, tr("Ascending order"), this);
    ascend_action->setStatusTip(tr("Ascending order"));
    ascend_action->setEnabled(false);
    connect(ascend_action, SIGNAL(triggered()), this, SLOT(ascend()));

    descend_action = new QAction(descend_icon, tr("Descending order"), this);
    descend_action->setStatusTip(tr("Descending order"));
    descend_action->setEnabled(false);
    connect(descend_action, SIGNAL(triggered()), this, SLOT(descend()));

    remove_all_filters_action = new QAction(tr("All filters"), this);
    remove_all_filters_action->setStatusTip(tr("Remove all filters"));
    connect(remove_all_filters_action, SIGNAL(triggered()), this, SLOT(removeAllFilters()));

    remove_all_ordering_action = new QAction(tr("All ordering"), this);
    remove_all_ordering_action->setStatusTip(tr("Remove all ordering"));
    connect(remove_all_ordering_action, SIGNAL(triggered()), this, SLOT(removeAllOrdering()));

    custom_filter_action = new QWidgetAction(this);
    custom_filter_action->setStatusTip(tr("Custom filter"));
    custom_filter_action->setIcon(filter_icon);
    custom_filter_action->setDefaultWidget(filter_text);
    connect(filter_text, SIGNAL(returnPressed()), this, SLOT(customFilterReturnPressed()));

    copy_query_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy_sql.png")), tr("Copy query"), this);
    copy_query_action->setStatusTip(tr("Copy the query to clipboard"));
    connect(copy_query_action, SIGNAL(triggered()), this, SLOT(copyQuery()));

    truncate_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/truncate.png")), tr("Clear table"), this);
    truncate_action->setStatusTip(tr("Delete the contents of the table"));
    connect(truncate_action, SIGNAL(triggered()), this, SLOT(truncateTable()));

    delete_rows_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/delete_rows.png")), tr("Delete row(s)"), this);
    delete_rows_action->setEnabled(false);
    delete_rows_action->setStatusTip(tr("Delete the selected row(s) of the table"));
    connect(delete_rows_action, SIGNAL(triggered()), this, SLOT(deleteRows()));

    previous_set_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/previous.png")), "", this);
    previous_set_action->setToolTip(tr("Fetch previous set"));
    connect(previous_set_action, SIGNAL(triggered()), this, SLOT(fetchPreviousData()));
    previous_set_button = new QToolButton;
    previous_set_button->setDefaultAction(previous_set_action);
    previous_set_button->setEnabled(false);

    next_set_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/next.png")), "", this);
    next_set_action->setToolTip(tr("Fetch next set"));
    connect(next_set_action, SIGNAL(triggered()), this, SLOT(fetchNextData()));
    next_set_button = new QToolButton;
    next_set_button->setDefaultAction(next_set_action);
    next_set_button->setEnabled(false);
}
