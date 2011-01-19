#include "queryview.h"

QueryView::QueryView(QWidget *parent, QSqlQueryModel* model, QString const name,
                     int const time, int const rows, int const cols, Qt::WidgetAttribute f)
{
    tview = new QTableView(this);
    tview->resizeColumnsToContents();
    setCentralWidget(tview);
    show();
    QString timeel = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
    QString rowsStr = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
    QString colsStr = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
    statusBar()->showMessage(timeel + QString::number((double)time/1000) +
                             " s \t " + rowsStr + QString::number(rows) +
                             " \t " + colsStr + QString::number(cols));
    tview->setModel(model);
    setWindowTitle(name);
    tview->setStyleSheet("QTableView {font-weight: 400;}");
    tview->setAlternatingRowColors(true);
    setGeometry(100,100,640,480);
    QShortcut* shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    connect(shortcut_ctrl_c, SIGNAL(activated()), this, SLOT(copyc()));
    QShortcut* shortcut_ctrl_shft_c = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(shortcut_ctrl_shft_c, SIGNAL(activated()), this, SLOT(copych()));

}

QueryView::~QueryView()
{
    delete tview->model();
    delete tview;
    close();
}

void QueryView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QPalette palette;
    palette.setColor(menu.backgroundRole(), QColor(205,205,205));
    menu.setPalette(palette);

    menu.addAction("Remove");
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Remove")==0) {

    }
    if(a && QString::compare(a->text(),"Expand")==0) {

    }
    if(a && QString::compare(a->text(),"Collapse")==0) {

    }
}

void QueryView::closeEvent(QCloseEvent *event)
{
    event->accept();
    delete tview->model();
    delete tview;
    close();
}

void QueryView::copyc()
{
    QItemSelectionModel* s = tview->selectionModel();
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

void QueryView::copych()
{
    QAbstractItemModel* atm = tview->model();
    QItemSelectionModel* s = tview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty())
        return;
    qSort(indices);
    QString headerText;
    QModelIndex current;
    foreach(current, indices)
        if(current.row() == 0) {
            QVariant data = atm->headerData(current.column(), Qt::Horizontal);
            headerText.append(data.toString());
            headerText.append(QLatin1Char('\t'));
        }
        else {
            headerText.append(QLatin1Char('\n'));
            break;
        }
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
