#include "explainview.h"

ExplainView::ExplainView(QSqlQueryModel* model, QString const name, Qt::WidgetAttribute f)
{
    this->setMod(model);
    this->setWindowTitle(name);
    this->setStyleSheet("QTableView {font-weight: 400;}");
    this->setGeometry(110,110,640,480);

    //Create Ctrl+Shift+C key combo to copy selected table contents without headers.
    QShortcut* shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    connect(shortcut_ctrl_c, SIGNAL(activated()), this, SLOT(copyc()));

    //Create Ctrl+Shift+C key combo to copy selected table contents with headers.
    QShortcut* shortcut_ctrl_shft_c = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(shortcut_ctrl_shft_c, SIGNAL(activated()), this, SLOT(copych()));

    //Create key-sequences for fullscreen and restore.
    QShortcut* shortcut_fs_win = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fs_win, SIGNAL(activated()), this, SLOT(fullscreen()));
    QShortcut* shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    this->show();
}

void ExplainView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QPalette palette;
    palette.setColor(menu.backgroundRole(), QColor(205,205,205));
    menu.setPalette(palette);
    menu.addAction("Remove");
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Remove")==0) {
    }
    else if(a && QString::compare(a->text(),"Expand")==0) {
    }
    else if(a && QString::compare(a->text(),"Collapse")==0) {
    }
}

void ExplainView::closeEvent(QCloseEvent *event)
{
    if(this->getMod())
        delete this->getMod();
    QWidget::closeEvent(event);
}

/*void ExplainView::resizeEvent(QResizeEvent *event)
{
    //int width = this->verticalHeader()->length();
    //if(this->verticalScrollBar()->isVisible()) {
    //    width += this->verticalScrollBar()->width();
    //}
    this->setColumnWidth(0, this->width()-50);
    QWidget::resizeEvent(event);
}*/

/*
void ExplainView::copyc()
{
    QItemSelectionModel* s = qview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty()) {
        return;
    }
    qSort(indices);
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    QModelIndex current;
    QString selectedText;

    foreach(current, indices) {
        QVariant data = qview->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(qview->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void ExplainView::copych()
{
    QAbstractItemModel* atm = qview->model();
    QItemSelectionModel* s = qview->selectionModel();
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

void ExplainView::fetchDataSlot(SqlMdl* smdl, int time, qint32 rows, qint32 cols)
{
    if(smdl->lastError().isValid()) {
        //emit errMesg(smdl->lastError().databaseText(), 1);
        //return;

        QStandardItemModel* model = new QStandardItemModel(0,1);
        QStringList mesgs = smdl->lastError().databaseText().split("\n");
        QStandardItem *item = new QStandardItem(mesgs[0]);
        model->appendRow(item);
        item->appendRow(new QStandardItem(mesgs[1]));
        model->appendRow(new QStandardItem(mesgs[1]));
        item->appendRow(new QStandardItem(mesgs[2]));
        model->appendRow(new QStandardItem(mesgs[2]));
        QFont serifFont("Courier", 10, QFont::Bold);
        qview->setFont(serifFont);
        qview->setModel(model);
        qview->resizeColumnToContents(0);
        QStringList hdr;
        hdr << "Error messages";
        model->setHorizontalHeaderLabels(hdr);
        QString timeel = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
        QString rowsStr = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
        QString colsStr = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
        statusBar()->showMessage(timeel + QString::number((double)time/1000) +
                                 " s \t " + rowsStr + "3" +
                                 " \t " + colsStr + "1");
    }
    else
    {
        QString timeel = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
        QString rowsStr = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
        QString colsStr = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
        statusBar()->showMessage(timeel + QString::number((double)time/1000) +
                                 " s \t " + rowsStr + QString::number(rows) +
                                 " \t " + colsStr + QString::number(cols));
        qview->setModel(smdl);
    }
    setCursor(Qt::ArrowCursor);
    threadBusy = false;
    emit finished();
}

void ExplainView::busySlot()
{
    threadBusy = true;
    setCursor(Qt::WaitCursor);
}

void ExplainView::fullscreen()
{
    this->showFullScreen();
}

void ExplainView::restore()
{
    this->showNormal();
}
*/
