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

    createIcons();
    createActions();

    toolbar = new QToolBar;
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
    toolbar->addAction(ascend_action);
    toolbar->addAction(descend_action);
    toolbar->addSeparator();
    toolbar->addAction(copy_query_action);
    toolbar->addSeparator();

    deselect_menu.setTitle(tr("Remove filter"));
    deselect_menu.setStatusTip(tr("Remove filter"));
    disarrange_menu.setTitle(tr("Remove order"));
    disarrange_menu.setStatusTip(tr("Remove order"));

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
    sql = "SELECT " + column_list.join(", ") + " FROM " + view_name;
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
    sql.append(view_name);

    setWindowTitle(name);
    setObjectName(name);

    view_view = new QTableView(this);
    view_view->viewport()->installEventFilter(this);
    view_view->installEventFilter(this);
    view_view->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);    
    view_view->setAlternatingRowColors(true);
    view_view->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    //Create key-sequences for fullscreen and restore.
    QShortcut *shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), this, SLOT(fullscreen()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    //Tie vertical scrollbar of QTableView to fetch more data
    connect(view_view->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchDataSlot()));

    //Tie thread finish to an update slot that refreshes meta-information.
    connect(this, SIGNAL(updRowCntSignal(QString)), this, SLOT(updRowCntSlot(QString)));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), this, SLOT(busySlot()));

    connect(view_view->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(customContextMenuHeader()));

    setCentralWidget(view_view);

    //Initialise the status bar.
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    statusBar()->addPermanentWidget(previous_set_button, 0);
    statusBar()->addPermanentWidget(next_set_button, 0);

    //Launch data retrieval as a future object (a different thread).
    defaultView();
}

//Mouse release event should enable/disable actions.
bool ViewView::eventFilter(QObject *obj, QEvent *event)
{
    if(view_view->model() == NULL)
        return QMainWindow::eventFilter(obj, event);

    if (obj == view_view->viewport()) {
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
/*
void ViewView::contextMenuEvent(QContextMenuEvent *event)
{
    filter_text->clear();
    context_menu.clear();

    QString status_message = statusBar()->currentMessage();
    QItemSelectionModel *s = vview->selectionModel();
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
        data = vview->model()->data(index);
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

            QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
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

            QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
            return;
        }
    }
}
*/
void ViewView::fetchDefaultData()
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
        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "viewview " + sql + QString::number(thisViewViewId));
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
            query_model->setQuery(sql + limit + offset_list.last(), database_connection);
        }
        else {
            query_model->setQuery(sql, database_connection);
        }

        rows_from = 1;
        rows_to = query_model->rowCount();
        column_count = query_model->columnCount();
        emit updRowCntSignal("next");
    }
}

void ViewView::fetchRefreshData(QString mode)
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

        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "viewview " + sql + QString::number(thisViewViewId));
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

        query_model->setQuery(query_model->query().lastQuery(), database_connection);
        emit updRowCntSignal(mode);
    }
}

void ViewView::busySlot()
{
    thread_busy = true;
    t.start();
    setCursor(Qt::WaitCursor);
}

void ViewView::updRowCntSlot(QString dataset)
{
    time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
    rows_string = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
    rows_string_2 = QApplication::translate("QueryView", " of whole set", 0, QApplication::UnicodeUTF8);
    colums_string = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
    seconds_string = QApplication::translate("QueryView", "s", 0, QApplication::UnicodeUTF8);

    if(query_model->lastError().isValid()) {
        updateFailedSlot(query_model->lastError().text());
        statusBar()->showMessage(QLatin1String(""));
        if(query_model->lastError().number() != -1) {
            if(where_clause.count() > 0)
                where_clause.removeLast();
            statusBar()->showMessage(tr("Error: Incorrect filter"));
        }
    }
    else {
        query_model->setRowsFrom(rows_from);
        view_view->setModel(query_model);
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
}

void ViewView::updateFailedSlot(QString error_text)
{
    error_message_box->setText(error_text);
    error_message_box->setStandardButtons(QMessageBox::Close);
    error_message_box->show();
}

void ViewView::bringOnTop()
{
    activateWindow();
    raise();
}

void ViewView::fetchNextData()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "viewview " + sql + QString::number(thisViewViewId));
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
            query_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            query_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);

        rows_to = rows_from + query_model->rowCount() - 1;
        column_count = query_model->columnCount();

        emit updRowCntSignal("next");
    }
}

void ViewView::fetchPreviousData()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "viewview " + sql + QString::number(thisViewViewId));
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
            query_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            query_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);

        rows_to = rows_from + query_model->rowCount() - 1;
        column_count = query_model->columnCount();
        can_fetch_more = true;

        emit updRowCntSignal("previous");
    }
}

void ViewView::fetchConditionDataInitial()
{
    {
        //If previous thread is not done with, abort.
        if(thread_busy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "viewview " + sql + QString::number(thisViewViewId));
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
            query_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            query_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        rows_to = rows_from + query_model->rowCount() - 1;
        column_count = query_model->columnCount();
        emit updRowCntSignal("next");
    }
}

void ViewView::fetchDataSlot()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(query_model->rowCount() >= FETCHSIZ &&
        view_view->verticalScrollBar()->value() == view_view->verticalScrollBar()->maximum()) {
        if(can_fetch_more) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
            QtConcurrent::run(this, &ViewView::fetchNextData);
        }
    }
    //Check if vertical scrollbar is at the top-most position to trigger
    //fetching of previous dataset from database. Data retrieval launched as a
    //future object (separate thread).
    else if(rows_from > 1 &&
            view_view->verticalScrollBar()->value() == view_view->verticalScrollBar()->minimum()) {
        statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
        QtConcurrent::run(this, &ViewView::fetchPreviousData);
    }
}

void ViewView::closeEvent(QCloseEvent *event)
{
    event->accept();
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

    if(!thread_busy)
    {
        delete toolbar;
        delete view_view;
        delete query_model;
        QSqlDatabase::removeDatabase("viewview " + sql + QString::number(thisViewViewId));
        QMainWindow::closeEvent(event);
    }
    else {
        hide();
    }
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
    filter(filter_text->text());
    filter_text->clear();
    context_menu.hide();
}

void ViewView::defaultView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    where_clause.clear();

    offset_list.clear();
    order_clause.clear();
    QtConcurrent::run(this, &ViewView::fetchDefaultData);
    disableActions();
}

void ViewView::refreshView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QtConcurrent::run(this, &ViewView::fetchRefreshData, QLatin1String("next"));
    disableActions();
}

void ViewView::addRowRefreshView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QtConcurrent::run(this, &ViewView::fetchRefreshData, QLatin1String("previous"));
    disableActions();
}

void ViewView::filter()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = view_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    QVariant data = view_view->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        if(!where_clause.contains("\"" + header_string + "\" IS NULL",
                            Qt::CaseInsensitive))
            where_clause.append("\"" + header_string + "\" IS NULL");
    }
    else
        if(data.type() == QMetaType::Int || data.type() == QMetaType::Long)
            where_clause.append("\"" + header_string + "\"=" + data.toString());
        //else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
        //    where_clause.append("\"" + header_string + "\"='" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header_string + "\"='" + data.toString().replace("'","\\'") + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
}

void ViewView::filter(QString filter)
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = view_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    where_clause.append("\"" + header_string + "\" " + filter);
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
    context_menu.hide();
}

void ViewView::exclude()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = view_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    QVariant data = view_view->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        where_clause.append("\"" + header_string + "\" IS NOT NULL");
    }
    else
        if(data.type() == QMetaType::Int || data.type() == QMetaType::Long)
            where_clause.append("\"" + header_string + "\"<>" + data.toString());
        //else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
        //    where_clause.append("\"" + header_string + "\"<>'" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header_string + "\"<>'" + data.toString() + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
}

void ViewView::ascend()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = view_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains(header_string + " DESC"))
        order_clause.removeOne(header_string + " DESC");
    if(!order_clause.contains(header_string + " ASC")) {
        order_clause.append(header_string + " ASC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
    }
}

void ViewView::descend()
{
    QModelIndexList indices = view_view->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = view_view->model()->headerData(index.column(), Qt::Horizontal);
    QString header_string = header.toString();
    if(order_clause.contains(header_string + " ASC"))
        order_clause.removeOne(header_string + " ASC");
    if(!order_clause.contains(header_string + " DESC")) {
        order_clause.append(header_string + " DESC");
        offset_list.clear();
        offset_list.append(" OFFSET 0");

        QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
    }
}

void ViewView::languageChanged(QEvent *event)
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

    QItemSelectionModel *s = view_view->selectionModel();
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
        data = view_view->model()->data(index);
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

            QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
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
            if(query_model->rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
            offset_list.clear();
            offset_list.append(" OFFSET 0");

            QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
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
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    where_clause.clear();

    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
}

void ViewView::removeAllOrdering()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    order_clause.clear();
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &ViewView::fetchConditionDataInitial);
}

void ViewView::copyQuery()
{
    QString copy_sql;
    if(where_clause.isEmpty())
        copy_sql = sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"");
    else
        copy_sql = sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"");

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
        else
            disableActions();
    }
}

void ViewView::enableActions()
{
    if(!thread_busy) {
        filter_action->setEnabled(true);
        exclude_action->setEnabled(true);
        ascend_action->setEnabled(true);
        descend_action->setEnabled(true);
    }
}

void ViewView::disableActions()
{
    //if(vview->selectionModel()->selectedIndexes().isEmpty()) {
    //    copy_action->setEnabled(false);
    //    copy_with_headers_action->setEnabled(false);
    //}
    filter_action->setEnabled(false);
    exclude_action->setEnabled(false);
    ascend_action->setEnabled(false);
    descend_action->setEnabled(false);
}

void ViewView::createIcons()
{
    key_icon = QIcon(":/icons/key.png");
    filter_icon = QIcon(":/icons/filter.png");
    ascend_icon = QIcon(":/icons/ascending.png");
    descend_icon = QIcon(":/icons/descending.png");
}

void ViewView::createActions()
{
    default_action = new QAction(QIcon(":/icons/table.png"), tr("Default"), this);
    default_action->setShortcut(QKeySequence("Ctrl+D"));
    default_action->setStatusTip(tr("Default"));
    connect(default_action, SIGNAL(triggered()), this, SLOT(defaultView()));

    refresh_action = new QAction(QIcon(":/icons/refresh.png"), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    refresh_action->setStatusTip(tr("Refresh"));
    connect(refresh_action, SIGNAL(triggered()), this, SLOT(refreshView()));

    copy_action = new QAction(QIcon(":/icons/copy.svgz"), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected"));
    //copy_action->setEnabled(false);
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

    copy_with_headers_action = new QAction(QIcon(":/icons/copy_with_headers.svg"), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    //copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, SIGNAL(triggered()), this, SLOT(copyToClipboardWithHeaders()));

    remove_columns_action = new QAction(QIcon(":/icons/removecolumn.png"), tr("Remove column(s)"), this);
    remove_columns_action->setStatusTip(tr("Removes the column from this display."));
    connect(remove_columns_action, SIGNAL(triggered()), this, SLOT(removeColumns()));

    filter_action = new QAction(filter_icon, tr("Filter"), this);
    filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
    filter_action->setEnabled(false);
    connect(filter_action, SIGNAL(triggered()), this, SLOT(filter()));

    exclude_action = new QAction(QIcon(":/icons/exclude.png"), tr("Exclude"), this);
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

    copy_query_action = new QAction(QIcon(":/icons/copy_sql.png"), tr("Copy query"), this);
    copy_query_action->setStatusTip(tr("Copy the query to clipboard"));
    connect(copy_query_action, SIGNAL(triggered()), this, SLOT(copyQuery()));

    previous_set_action = new QAction(QIcon(":/icons/previous.png"), "", this);
    previous_set_action->setToolTip(tr("Fetch previous set"));
    connect(previous_set_action, SIGNAL(triggered()), this, SLOT(fetchPreviousData()));
    previous_set_button = new QToolButton;
    previous_set_button->setDefaultAction(previous_set_action);
    previous_set_button->setEnabled(false);

    next_set_action = new QAction(QIcon(":/icons/next.png"), "", this);
    next_set_action->setToolTip(tr("Fetch next set"));
    connect(next_set_action, SIGNAL(triggered()), this, SLOT(fetchNextData()));
    next_set_button = new QToolButton;
    next_set_button->setDefaultAction(next_set_action);
    next_set_button->setEnabled(false);
}
