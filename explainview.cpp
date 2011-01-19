#include "explainview.h"

ExplainView::ExplainView(QSqlQueryModel* model, QString const name, Qt::WidgetAttribute f)
{
    this->setMod(model);
    this->setWindowTitle(name);
    this->setStyleSheet("QTableView {font-weight: 400;}");
    this->setGeometry(110,110,640,480);
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
