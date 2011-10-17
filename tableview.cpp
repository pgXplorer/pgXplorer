#include "tableview.h"

TableView::TableView(QString const tblName, QString const name, Qt::WidgetAttribute f)
{
    t.start();
    quickFetch = true;
    tb = new QToolBar("Edit");
    this->addToolBar(Qt::TopToolBarArea, tb);
    tb->addSeparator();
    tb->setMovable(false);
    sql = "SELECT * FROM " + tblName;
    
    tview = new QTableView(this);
    tview->resizeColumnsToContents();

    this->setWindowTitle(name);
    tview->setStyleSheet("QTableView {font-weight: 400;}");
    tview->setAlternatingRowColors(true);
    this->setGeometry(100,100,640,480);
    connect(tview->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(fetchMore()));
    QShortcut* shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    connect(shortcut_ctrl_c, SIGNAL(activated()), this, SLOT(copyc()));
    QShortcut* shortcut_ctrl_shft_c = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(shortcut_ctrl_shft_c, SIGNAL(activated()), this, SLOT(copych()));
    connect(this, SIGNAL(updRowCntSignal(qint32,qint32,int)), this, SLOT(updRowCntSlot()));
    setCentralWidget(tview);
    show();
    
    QFuture<void> future = QtConcurrent::run(this, &TableView::fetchData);
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
        menu.addAction("Select");
        menu.addAction("~Select");
        QMenu* deselectMenu = new QMenu("Deselect");
        for(int i=0; i<whereCl.size(); i++)
            deselectMenu->addAction(whereCl.at(i));
        if(whereCl.size() > 1) {
            deselectMenu->addSeparator();
            deselectMenu->addAction("All");
        }
        menu.addMenu(deselectMenu);
        menu.addSeparator();
        menu.addAction(QString(QChar(0x21e9)).append("Order"));
        menu.addAction(QString(QChar(0x21e7)).append("Order"));
        QMenu* disarrangeMenu = new QMenu("Disarrange");
        int orderClSiz = orderCl.size();
        for(int i=0; i<orderClSiz; i++) {
            QString order = orderCl.at(i);
            order.replace(" ASC", QChar(0x21e9));
            order.replace(" DESC", QChar(0x21e7));
            disarrangeMenu->addAction(order);
        }
        if(orderClSiz > 1) {
            disarrangeMenu->addSeparator();
            disarrangeMenu->addAction("All");
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
        if(a && QString::compare(a->text(),"Select")==0) {
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
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
            return;
        }
        else if(a && QString::compare(a->text(),"~Select")==0) {
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
            qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderClSiz > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
            return;
        }
        else if(a && QString::compare(a->text(),"All")==0) {
            statusBar()->showMessage("Fetching data...");
            whereCl.clear();
            QTime t;
            t.start();
            offsetList.clear();
            offsetList.append(" OFFSET 0");
            orderCl.clear();
            orderClSiz = 0;
            qryMdl->setQuery(sql + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
            return;
        }
        else if(a && QString::compare(a->text(),QString(QChar(0x21e9)).append("Order"))==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            orderCl.append(hdr.toString() + " ASC");
            orderClSiz++;
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
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
            return;
        }
        else if(a && QString::compare(a->text(),QString(QChar(0x21e7)).append("Order"))==0) {
            statusBar()->showMessage("Fetching data...");
            QTime t;
            t.start();
            QVariant hdr = tview->model()->headerData(index.column(), Qt::Horizontal);
            orderCl.append(hdr.toString() + " DESC");
            orderClSiz++;
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
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
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
            rowcount = qryMdl->rowCount();
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
                    rowcount = qryMdl->rowCount();
                    colcount = qryMdl->columnCount();
                    tview->setModel(qryMdl);
                    //tview->hideColumn(0);
                    statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                             " s \t Rows: " + QString::number(rowcount) +
                                             " \t Columns: " + QString::number(colcount));
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
                    rowcount = qryMdl->rowCount();
                    colcount = qryMdl->columnCount();
                    tview->setModel(qryMdl);
                    //tview->hideColumn(0);
                    statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                             " s \t Rows: " + QString::number(rowcount) +
                                             " \t Columns: " + QString::number(colcount));
                    return;
                }
            }
            return;
        }
    }
}

void TableView::fetchData()
{
    qryMdl = new QSqlQueryModel;
    if(quickFetch) {
        QString offset = " OFFSET " + QString::number(FETCHSIZ);
        limit = " LIMIT " + QString::number(FETCHSIZ);
        qryMdl->setQuery(sql + limit + offset);
        if(qryMdl->rowCount() == 0)
            canFetchMore = false;
        else
            canFetchMore = true;
        offsetList.append(" OFFSET 0");
        qryMdl->setQuery(sql + limit + offsetList.last());
    }
    else {
        qryMdl->setQuery(sql);
    }
    rowcount = qryMdl->rowCount();
    colcount = qryMdl->columnCount();
    updRowCntSignal(rowcount,colcount,t.elapsed());
}

void TableView::updRowCntSlot()
{
    tview->setModel(qryMdl);
    statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                             " s \t Rows: " + QString::number(rowcount) +
                             " \t Columns: " + QString::number(colcount));
}

void TableView::fetchMore()
{
    if(qryMdl->rowCount() >= FETCHSIZ &&
       tview->verticalScrollBar()->value() == tview->verticalScrollBar()->maximum())
        if(canFetchMore) {
            statusBar()->showMessage("Fetching more data...");
            QTime t;
            t.start();
            QString offset = " OFFSET " + QString::number((offsetList.size()+1)*FETCHSIZ);
            if(whereCl.isEmpty())
                qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offset);
            else
                qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offset);
            if(qryMdl->rowCount() == 0)
                canFetchMore = false;
            else
                canFetchMore = true;
            offsetList.append(" OFFSET " + QString::number(offsetList.size()*FETCHSIZ));
            if(whereCl.isEmpty())
                qryMdl->setQuery(sql + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit + offsetList.last());
            else
                qryMdl->setQuery(sql + " WHERE " + whereCl.join(" AND ") + (orderCl.size() > 0 ? " ORDER BY " + orderCl.join(","):"") + limit+ offsetList.last());
            if (qryMdl->lastError().isValid()) {
                QMessageBox* dbErr = new QMessageBox("pgXplorer", qryMdl->lastError().text(),
                                                       QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
                dbErr->setButtonText(1, "Close");
                dbErr->show();
                statusBar()->showMessage("An error occurred.");
                return;
            }
            rowcount = qryMdl->rowCount();
            colcount = qryMdl->columnCount();
            tview->setModel(qryMdl);
            //tview->hideColumn(0);
            statusBar()->showMessage("Time elapsed: " + QString::number((double)t.elapsed()/1000) +
                                     " s \t Rows: " + QString::number(rowcount) +
                                     " \t Columns: " + QString::number(colcount));
        }
}

void TableView::closeEvent(QCloseEvent *event)
{
    delete tview;
    delete qryMdl;
    close();
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
