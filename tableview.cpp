/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

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

TableView::TableView(Database *database, QString const table_name, QString const name, QStringList column_list, Qt::WidgetAttribute f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    menuBar()->setVisible(false);
    this->database = database;
    filter_text = new QLineEdit;
    filter_text->setPlaceholderText(tr("custom filter"));

    filter_icon = QIcon(qApp->applicationDirPath().append("/icons/filter.png"));
    ascend_icon = QIcon(qApp->applicationDirPath().append("/icons/ascending.png"));
    descend_icon = QIcon(qApp->applicationDirPath().append("/icons/descending.png"));

    createActions();

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
    toolbar->addAction(truncate_action);

    deselect_menu.setTitle(tr("Remove filter"));
    disarrange_menu.setTitle(tr("Remove order"));

    addToolBar(toolbar);
    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisTableViewId = tableViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have TableView
    //GUI artifacts to create overlapping threads.
    thread_busy = false;
    query_model = new QSqlQueryModel;

    quick_fetch = true;
    sql = "SELECT " + column_list.join(", ") + " FROM " + table_name;
    tview = new QTableView(this);
    tview->viewport()->installEventFilter(this);
    tview->installEventFilter(this);
    this->setWindowTitle(name);
    this->setObjectName(name);
    tview->setStyleSheet("QTableView {font-weight: 400;}");
    tview->setAlternatingRowColors(true);

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

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), this, SLOT(busySlot()));

    setCentralWidget(tview);
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    //Launch data retrieval as a future object (a different thread).
    defaultView();
}

//Mouse release event should enable/disable actions.
bool TableView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tview->viewport()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            toggleActions();
        }
        return false;
    }
    else if(obj == tview) {
        if (event->type() == QEvent::MouseButtonRelease) {
            toggleActions();
        }
        return false;
    }
    else
        return QMainWindow::eventFilter(obj, event);
}

//Key release event should enable/disable actions.
void TableView::keyReleaseEvent(QKeyEvent*)
{
    toggleActions();
}

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

void TableView::fetchRefreshData()
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

        query_model->setQuery(query_model->query().lastQuery(), database_connection);

        emit updRowCntSignal("next");
    }
}

void TableView::busySlot()
{
    thread_busy = true;
    t.start();
    setCursor(Qt::WaitCursor);
}

void TableView::updRowCntSlot(QString dataset)
{
    QString time_elapsed = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
    QString rows_string = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
    QString rows_string_2 = QApplication::translate("QueryView", " of whole set", 0, QApplication::UnicodeUTF8);
    QString colums_string = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
    QString seconds_string = QApplication::translate("QueryView", "s", 0, QApplication::UnicodeUTF8);

    if(query_model->lastError().isValid()) {
        if(where_clause.count() > 0)
            where_clause.removeLast();
        statusBar()->showMessage(tr("Error: Incorrect filter"));
    }
    else {
        tview->setModel(query_model);
        int column_count = query_model->columnCount();
        for(int i = 0; i < column_count; i++)
            tview->showColumn(i);
        if(dataset.compare("previous") == 0)
            tview->verticalScrollBar()->setValue(tview->verticalScrollBar()->maximum());
        else if(dataset.compare("next") == 0)
            tview->verticalScrollBar()->setValue(tview->verticalScrollBar()->minimum());

        if(rows_to == 0)
            statusBar()->showMessage(time_elapsed + QString::number((double)t.elapsed()/1000) +
                                     " " + seconds_string + " \t " +  rows_string + "0" +
                                     " \t " + colums_string + QString::number(column_count));
        else {
            if(can_fetch_more)
                statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                     " - " + QString::number(rows_to) + rows_string_2 +
                                     " \t " + colums_string + QString::number(column_count));
            else
                statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(rows_from) +
                                     " - " + QString::number(rows_to) +
                                     " \t " + colums_string + QString::number(column_count));
        }
    }
    thread_busy = false;
    setCursor(Qt::ArrowCursor);
}

void TableView::fetchNextData()
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

        QSqlQueryModel temp_query_model;

        offset_list.removeLast();
        rows_from = (offset_list.size()-1)*FETCHSIZ + 1;
        if(where_clause.isEmpty())
            query_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            query_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);

        rows_to = rows_from + query_model->rowCount() - 1;
        column_count = query_model->columnCount();

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
            query_model->setQuery(sql + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        else
            query_model->setQuery(sql + " WHERE " + where_clause.join(" AND ") + (order_clause.size() > 0 ? " ORDER BY " + order_clause.join(","):"") + limit + offset_list.last(), database_connection);
        rows_to = rows_from + query_model->rowCount() - 1;
        column_count = query_model->columnCount();
        emit updRowCntSignal("next");
    }
}

void TableView::fetchDataSlot()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(query_model->rowCount() >= FETCHSIZ &&
        tview->verticalScrollBar()->value() == tview->verticalScrollBar()->maximum()) {
        if(can_fetch_more) {
            statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchNextData);
        }
    }
    //Check if vertical scrollbar is at the top-most position to trigger
    //fetching of previous dataset from database. Data retrieval launched as a
    //future object (separate thread).
    else if(rows_from > 1 &&
            tview->verticalScrollBar()->value() == tview->verticalScrollBar()->minimum()) {
        statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
        QFuture<void> future = QtConcurrent::run(this, &TableView::fetchPreviousData);
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
        delete query_model;
        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QMainWindow::closeEvent(event);
    }
    else {
        hide();
    }
}

void TableView::copyc()
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

void TableView::copych()
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
        tview->hideColumn(indices.at(i).column());
        i++;
        if(i >= indices.size())
            break;
        if(indices.at(i).row() != current_row)
            break;
    }

    //query_model->removeColumns(first_column, i);
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
    order_clause_size = 0;
    QtConcurrent::run(this, &TableView::fetchDefaultData);
    disableActions();
}

void TableView::refreshView()
{
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchRefreshData);
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
    QVariant data = tview->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        if(!where_clause.contains("\"" + header.toString() + "\" IS NULL",
                            Qt::CaseInsensitive))
            where_clause.append("\"" + header.toString() + "\" IS NULL");
    }
    else
        if(typ.compare("int", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header.toString() + "\"=" + data.toString());
        else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header.toString() + "\"='" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header.toString() + "\"='" + data.toString() + "'");
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::filter(QString filter)
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    where_clause.append("\"" + header.toString() + "\" " + filter);
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
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
    QVariant data = tview->model()->data(index);
    QString typ(data.typeName());
    if(data.isNull()) {
        where_clause.append("\"" + header.toString() + "\" IS NOT NULL");
    }
    else
        if(typ.compare("int", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header.toString() + "\"<>" + data.toString());
        else if(typ.compare("QDateTime", Qt::CaseInsensitive) == 0)
            where_clause.append("\"" + header.toString() + "\"<>'" + data.toDateTime().toString("yyyy-MM-dd hh:mm:ss.z") + "'");
        else
            where_clause.append("\"" + header.toString() + "\"<>'" + data.toString() + "'");
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
    order_clause.append(header.toString() + " ASC");
    order_clause_size++;
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::descend()
{
    QModelIndexList indices = tview->selectionModel()->selectedIndexes();
    if(indices.size() != 1)
        return;
    QModelIndex index = indices.first();
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));

    QVariant header = tview->model()->headerData(index.column(), Qt::Horizontal);
    order_clause.append(header.toString() + " DESC");
    order_clause_size++;
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
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
    order_clause_size = 0;
    offset_list.clear();
    offset_list.append(" OFFSET 0");

    QtConcurrent::run(this, &TableView::fetchConditionDataInitial);
}

void TableView::copyQuery()
{
    QString copy_sql;
    if(where_clause.isEmpty())
        copy_sql = sql + (order_clause_size > 0 ? " ORDER BY " + order_clause.join(","):"");
    else
        copy_sql = sql + " WHERE " + where_clause.join(" AND ") + (order_clause_size > 0 ? " ORDER BY " + order_clause.join(","):"");
    qApp->clipboard()->setText(copy_sql);
}

void TableView::truncateTable()
{
    int ret = QMessageBox::warning(this, tr("pgXplorer"),
                                    tr("This action will destroy all data in this table and cannot be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "clear" + objectName());
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
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
        statusBar()->showMessage(tr("All table rows deleted"));
    }
    QSqlDatabase::removeDatabase("clear" + objectName());
    fetchDefaultData();
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
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copyc()));

    copy_with_headers_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy_with_headers.svg")), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    //copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, SIGNAL(triggered()), this, SLOT(copych()));

    remove_columns_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/removecolumn.png")), tr("Remove column(s)"), this);
    remove_columns_action->setStatusTip(tr("Removes the column from this display."));
    connect(remove_columns_action, SIGNAL(triggered()), this, SLOT(removeColumns()));

    filter_action = new QAction(filter_icon, tr("Filter"), this);
    filter_action->setStatusTip(tr("Filter table with selected cell value on column"));
    filter_action->setEnabled(false);
    connect(filter_action, SIGNAL(triggered()), this, SLOT(filter()));

    exclude_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/exclude.svg")), tr("Exclude"), this);
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
}
