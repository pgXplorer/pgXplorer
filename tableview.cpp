#include "tableview.h"

ulong TableView::tableViewObjectId = 0;

TableView::TableView(Database* db, QString const tblName, QString const name, Qt::WidgetAttribute f)
{
    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisTableViewId = tableViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have TableView
    //GUI artifacts to create overlapping threads.
    threadBusy = false;

    //Bring in database connection parameters from Database object.
    host =  db->getDb().hostName();
    port = db->getDb().port();
    dbname = db->getDb().databaseName();
    user = db->getDb().userName();
    password = db->getDb().password();
    t.start();

    //
    quickFetch = true;
    tb = new QToolBar("Edit");
    this->addToolBar(Qt::TopToolBarArea, tb);
    tb->addSeparator();
    tb->setMovable(false);
    sql = "SELECT * FROM " + tblName;
    tview = new QTableView(this);
    tview->resizeColumnsToContents();
    this->setWindowTitle(name);
    this->setObjectName(name);
    tview->setStyleSheet("QTableView {font-weight: 400;}");
    tview->setAlternatingRowColors(true);
    this->setGeometry(100,100,640,480);

    //Create key-sequences for fullscreen and restore.
    QShortcut* shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), this, SLOT(fullscreen()));
    QShortcut* shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    //Tie vertical scrollbar of TableView to fetch more data
    connect(tview->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchMore()));

    //Create Ctrl+Shift+C key combo to copy selected table contents without headers.
    QShortcut* shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    connect(shortcut_ctrl_c, SIGNAL(activated()), this, SLOT(copyc()));

    //Create Ctrl+Shift+C key combo to copy selected table contents with headers.
    QShortcut* shortcut_ctrl_shft_c = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(shortcut_ctrl_shft_c, SIGNAL(activated()), this, SLOT(copych()));

    //Tie thread finish to an update slot that refreshes meta-information.
    connect(this, SIGNAL(updRowCntSignal()), this, SLOT(updRowCntSlot()));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), this, SLOT(busySlot()));
    setCentralWidget(tview);
    statusBar()->showMessage("Fetching data...");
    show();

    //Launch data retrieval as a future object (a different thread).
    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchData, host, port, dbname, user, password);
}

void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    QItemSelectionModel* s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty() || indices.size() > 1) {
        QMenu menu;
        menu.addAction("Copy");
        menu.addAction("Copy with headers");
        QAction *a = menu.exec(QCursor::pos());
        if(a && QString::compare(a->text(),"Copy")==0)
            copyc();
        else if(a && QString::compare(a->text(),"Copy with headers")==0)
            copych();
        return;
    }
    QModelIndex index = indices.first();
    QVariant data = tview->model()->data(index);
    if(data.canConvert<QString>()) {
        QMenu menu;
        menu.addAction("Default");
        menu.addSeparator();
        menu.addAction("Filter");
        menu.addAction("Exclude");
        QMenu* deselectMenu = new QMenu("Remove filter");
        for(int i=0; i<whereCl.size(); i++)
            deselectMenu->addAction(whereCl.at(i));
        if(whereCl.size() > 1) {
            deselectMenu->addSeparator();
            deselectMenu->addAction("All filters");
        }
        menu.addMenu(deselectMenu);
        menu.addSeparator();
        menu.addAction(QString(QChar(0x21e9)).append("Order"));
        menu.addAction(QString(QChar(0x21e7)).append("Order"));
        QMenu* disarrangeMenu = new QMenu("Remove order");
        int orderClSiz = orderCl.size();
        for(int i=0; i<orderClSiz; i++) {
            QString order = orderCl.at(i);
            order.replace(" ASC", QChar(0x21e9));
            order.replace(" DESC", QChar(0x21e7));
            disarrangeMenu->addAction(order);
        }
        if(orderClSiz > 1) {
            disarrangeMenu->addSeparator();
            disarrangeMenu->addAction("All orders");
        }
        menu.addMenu(disarrangeMenu);
        /*
        menu.addSeparator();
        menu.addAction("Group");
        QMenu* ungroupMenu = new QMenu("Ungroup");
        int groupClSiz = groupCl.size();
        for(int i=0; i<groupClSiz; i++) {
            QString group = groupCl.at(i);
            ungroupMenu->addAction(group);
        }
        if(groupClSiz > 1) {
            ungroupMenu->addSeparator();
            ungroupMenu->addAction("All");
        }
        menu.addMenu(ungroupMenu);
        */
        menu.addSeparator();
        menu.addAction("Copy query");

        QAction *a = menu.exec(QCursor::pos());
        if(a && QString::compare(a->text(),"Default")==0) {
            statusBar()->showMessage("Fetching data...");
            whereCl.clear();
            QTime t;
            t.start();
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            orderCl.clear();
            orderClSiz = 0;
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchData, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),"Filter")==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            QVariant data = tview->model()->data(index);
            QString typ(data.typeName());
            if(data.isNull()) {
                if(!whereCl.contains("\"" + hdr.toString() + "\" IS NULL",
                                    Qt::CaseInsensitive))
                    whereCl.append("\"" + hdr.toString() + "\" IS NULL");
            }
            else
                if(typ.compare("int", Qt::CaseInsensitive) == 0)
                    whereCl.append("\"" + hdr.toString() + "\"=" + data.toString());
                else if(typ.compare("int", Qt::CaseInsensitive) == 0)
                    whereCl.append("\"" + hdr.toString() + "\"=" + data.toString());
                else
                    whereCl.append("\"" + hdr.toString() + "\"='" + data.toString() + "'");
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo  = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),"Exclude")==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            QVariant data = tview->model()->data(index);
            QString typ(data.typeName());
            if(data.isNull()) {
                whereCl.append("\"" + hdr.toString() + "\" IS NOT NULL");
            }
            else
                if(typ.compare("int", Qt::CaseInsensitive) == 0)
                    whereCl.append("\"" + hdr.toString() + "\"<>" + data.toString());
                else if(typ.compare("int", Qt::CaseInsensitive) == 0)
                    whereCl.append("\"" + hdr.toString() + "\"<>" + data.toString());
                else
                    whereCl.append("\"" + hdr.toString() + "\"<>'" + data.toString() + "'");
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),"All filters")==0) {
            statusBar()->showMessage("Fetching data...");
            whereCl.clear();
            QTime t;
            t.start();
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            qryMdl->setQuery(sql + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),QString(QChar(0x21e9)).append("Order"))==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            orderCl.append(hdr.toString() + " ASC");
            orderClSiz++;
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            if(whereCl.isEmpty())
                qryMdl->setQuery(sql + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
            else
                qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),QString(QChar(0x21e7)).append("Order"))==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            orderCl.append(hdr.toString() + " DESC");
            orderClSiz++;
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            if(whereCl.isEmpty())
                qryMdl->setQuery(sql + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
            else
                qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        else if(a && QString::compare(a->text(),"All orders")==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            orderCl.clear();
            orderClSiz = 0;
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            /*
            qryMdl->setQuery(sql + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowsFrom) +
                                     " - " + QString::number(rowsTo) +
                                     " \t Columns: " + QString::number(colcount));
            */
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
            return;
        }
        /*else if(a && QString::compare(a->text(),"Group")==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            groupCl.append(hdr.toString());
            groupClSiz++;
            if(whereCl.isEmpty())
                qryMdl->setQuery(sql
                                 + (groupClSiz > 0 ? " GROUP BY " + groupCl.join(","):"")
                                 + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"")
                                 + limit + offsetList.last());
            else
                qryMdl->setQuery(sql
                                 + " WHERE " + whereCl.join(" AND ")
                                 + (groupClSiz > 0 ? " GROUP BY " + groupCl.join(","):"")
                                 + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"")
                                 + limit + offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowsTo = rowsFrom + qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
            return;
        }*/
        else if(a && QString::compare(a->text(),"Copy query")==0) {
            QString copy_sql;
            if(whereCl.isEmpty())
                copy_sql = sql + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"");
            else
                copy_sql = sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"");
            qApp->clipboard()->setText(copy_sql);
        }
        else {
            for(int i=0; i<whereCl.size(); i++) {
                if(a && QString::compare(a->text(),whereCl.at(i))==0) {
                    statusBar()->showMessage("Fetching data...");
                    QTime t;
                    t.start();
                    whereCl.removeAt(i);
                    if(qryMdl->rowCount() == 0)
                        canFetchMore = false;
                    else
                        canFetchMore = true;
                    offsetList.clear();
                    offsetList.append(" OFFSET 0");
                    /*
                    if(whereCl.isEmpty())
                        qryMdl->setQuery(sql + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
                    else
                        qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
                    if (qryMdl->lastError().isValid()) {
                        QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                               QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                        dbErr->setButtonText(1, "Close");
                        dbErr->show();
                        statusBar()->showMessage("An error occurred.");
                        return;
                    }
                    rowsTo = rowsFrom + qryMdl->rowCount();
                    colcount = qryMdl->columnCount();
                    tview->setModel(qryMdl);
                    //tview->hideColumn(0);
                    statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                             " s \t Rows: " + QString::number(rowsFrom) +
                                             " - " + QString::number(rowsTo) +
                                             " \t Columns: " + QString::number(colcount));
                    */
                    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
                    return;
                }
            }
            for(int i=0; i<orderCl.size(); i++) {
                if(!a)
                    return;
                QString order = a->text();
                order.replace(QChar(0x21e9), " ASC");
                order.replace(QChar(0x21e7), " DESC");
                if(QString::compare(order, orderCl.at(i))==0) {
                    statusBar()->showMessage("Fetching data...");
                    QTime t;
                    t.start();
                    orderCl.removeAt(i);
                    orderClSiz--;
                    if(qryMdl->rowCount() == 0)
                        canFetchMore = false;
                    else
                        canFetchMore = true;
                    offsetList.clear();
                    offsetList.append(" OFFSET 0");
                    /*
                    if(whereCl.isEmpty())
                        qryMdl->setQuery(sql + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
                    else
                        qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
                    if (qryMdl->lastError().isValid()) {
                        QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                               QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                        dbErr->setButtonText(1, "Close");
                        dbErr->show();
                        statusBar()->showMessage("An error occurred.");
                        return;
                    }
                    rowsTo = rowsFrom + qryMdl->rowCount();
                    colcount = qryMdl->columnCount();
                    tview->setModel(qryMdl);
                    //tview->hideColumn(0);
                    statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                             " s \t Rows: " + QString::number(rowsFrom) +
                                             " - " + QString::number(rowsTo) +
                                             " \t Columns: " + QString::number(colcount));
                    */
                    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData2, host, port, dbname, user, password);
                    return;
                }
            }
            return;
        }
    }
}

void TableView::fetchData(const QString srv, const qint32 port, const QString datab, const QString user, const QString pass)
{
    {
        //We want to ensure that we have only one thread acting
        //at a given point in time.
        //If the previous thread is not done with, abort spawning
        //a new thread to avoid the possibility of a crash.
        if(threadBusy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase sqldb;
        sqldb = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        sqldb.setHostName(srv);
        sqldb.setPort(port);
        sqldb.setDatabaseName(datab);
        sqldb.setUserName(user);
        sqldb.setPassword(pass);
        if (!sqldb.open()) {
            qDebug() << qApp->tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }
        qryMdl = new QSqlQueryModel;
        if(quickFetch) {
            QString offset = " OFFSET " + QString::number(FETCHSIZ);
            limit = " LIMIT " + QString::number(FETCHSIZ);
            qryMdl->setQuery(sql + limit + offset, sqldb);
            if(qryMdl->rowCount() == 0)
                canFetchMore = false;
            else
                canFetchMore = true;
            offsetList.append(" OFFSET 0");
            qryMdl->setQuery(sql + limit + offsetList.last(), sqldb);
        }
        else {
            qryMdl->setQuery(sql, sqldb);
        }

        rowsFrom = 1;
        rowsTo = qryMdl->rowCount();
        colcount = qryMdl->columnCount();
        emit updRowCntSignal();
    }
}

void TableView::busySlot()
{
    threadBusy = true;
    setCursor(Qt::WaitCursor);
}

void TableView::updRowCntSlot()
{
    tview->setModel(qryMdl);
    tview->verticalScrollBar()->setValue(0);
    if(rowsTo == 0)
    {
        statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                 " s \t Rows: 0" +
                                 " \t Columns: " + QString::number(colcount));
    }
    else
    {
        statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                 " s \t Rows: " + QString::number(rowsFrom) +
                                 " - " + QString::number(rowsTo) +
                                 " \t Columns: " + QString::number(colcount));
    }
    setCursor(Qt::ArrowCursor);
    threadBusy = false;
}

void TableView::fetchMoreData(const QString srv, const qint32 port, const QString datab, const QString user, const QString pass)
{
    {
        //If previous thread is not done with, abort.
        if(threadBusy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase sqldb;
        sqldb = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        sqldb.setHostName(srv);
        sqldb.setPort(port);
        sqldb.setDatabaseName(datab);
        sqldb.setUserName(user);
        sqldb.setPassword(pass);
        if (!sqldb.open()) {
            qDebug() << qApp->tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        // Test the next batch of offset to set 'canFetchMore'.
        // This boolean variable determines if we need to fetch more data
        // after current batch.
        qryMdl = new QSqlQueryModel;
        QString offset = " OFFSET " + QString::number((offsetList.size()+1)*FETCHSIZ);
        if(whereCl.isEmpty())
            qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offset, sqldb);
        else
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offset, sqldb);
        if(qryMdl->rowCount() == 0)
            canFetchMore = false;
        else
            canFetchMore = true;
        rowsFrom = offsetList.size()*FETCHSIZ + 1;
        offsetList.append(" OFFSET " + QString::number(rowsFrom - 1));
        if(whereCl.isEmpty())
            qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last(), sqldb);
        else
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last(), sqldb);
        // Cannot call GUI objects in non-GUI thread.
        // So commenting out the sql error box.
        /*if (qryMdl->lastError().isValid()) {
            QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                   QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
            dbErr->setButtonText(1, "Close");
            dbErr->show();
            statusBar()->showMessage("An error occurred.");
            return;
        }*/
        rowsTo = rowsFrom + qryMdl->rowCount() - 1;
        colcount = qryMdl->columnCount();
        emit updRowCntSignal();
    }
}

void TableView::fetchMoreData2(const QString srv, const qint32 port, const QString datab, const QString user, const QString pass)
{
    {
        //If previous thread is not done with, abort.
        if(threadBusy)
            return;
        //Indicate that we are going to be retrieving data and busy.
        emit busySignal();

        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        QSqlDatabase sqldb;
        sqldb = QSqlDatabase::addDatabase("QPSQL", "tableview" + sql + QString::number(thisTableViewId));
        sqldb.setHostName(srv);
        sqldb.setPort(port);
        sqldb.setDatabaseName(datab);
        sqldb.setUserName(user);
        sqldb.setPassword(pass);
        if (!sqldb.open()) {
            qDebug() << qApp->tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        // Test the next batch of offset to set 'canFetchMore'.
        // This boolean variable determines if we need to fetch more data
        // after current batch.
        qryMdl = new QSqlQueryModel;
        QString offset = " OFFSET " + QString::number(offsetList.size()*FETCHSIZ);
        if(whereCl.isEmpty())
            qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offset, sqldb);
        else
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offset, sqldb);
        if(qryMdl->rowCount() == 0)
            canFetchMore = false;
        else
            canFetchMore = true;
        rowsFrom = (offsetList.size()-1)*FETCHSIZ + 1;
        if(whereCl.isEmpty())
            qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last(), sqldb);
        else
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last(), sqldb);
        // Cannot call GUI objects in non-GUI thread.
        // So commenting out the sql error box.
        /*if (qryMdl->lastError().isValid()) {
            QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                   QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
            dbErr->setButtonText(1, "Close");
            dbErr->show();
            statusBar()->showMessage("An error occurred.");
            return;
        }*/
        rowsTo = rowsFrom + qryMdl->rowCount() - 1;
        colcount = qryMdl->columnCount();
        updRowCntSignal();
    }
}

void TableView::fetchMore()
{
    //Check if vertical scrollbar is at the bottom-most position to trigger
    //fetching of more data from database. Data retrieval launched as a
    //future object (separate thread).
    if(qryMdl->rowCount() >= FETCHSIZ &&
        tview->verticalScrollBar()->value() == tview->verticalScrollBar()->maximum()) {
        if(canFetchMore) {
            statusBar()->showMessage("Fetching more data...");
            t.start();
            QFuture<void> future = QtConcurrent::run(this, &TableView::fetchMoreData, host, port, dbname, user, password);
        }
    }
}

void TableView::closeEvent(QCloseEvent *event)
{
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //TableView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    if(!threadBusy)
    {
        delete tview;
        delete qryMdl;
        QSqlDatabase::removeDatabase("tableview" + sql + QString::number(thisTableViewId));
        close();
    }
}

void TableView::copyc()
{
    QItemSelectionModel* s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty())
        return;
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
    QAbstractItemModel* atm = tview->model();
    QItemSelectionModel* s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty())
        return;
    qSort(indices);
    QString headerText;
    QModelIndex current;
    int prevRow = indices.at(0).row();
    foreach(current, indices) {
        if(current.row() == prevRow) {
            QVariant data = atm->headerData(current.column(), Qt::Horizontal);
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
        QVariant data = atm->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(atm->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void TableView::fullscreen()
{
    this->showFullScreen();
}

void TableView::restore()
{
    this->showNormal();
}
