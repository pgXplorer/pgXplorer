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

#include "reportwindow.h"
#include <QSqlDatabase>

#ifdef HPDF_DLL
void __stdcall
#else
void
#endif
error_handler (HPDF_STATUS error_no,
               HPDF_STATUS detail_no,
               void *user_data)
{
    printf("ERROR: error_no=%04X, detail_no=%u\n",
           (HPDF_UINT)error_no,
           (HPDF_UINT)detail_no);
}

/*QVariant TopMargin::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QRectF rect = scene()->sceneRect();
        QPointF new_pos = value.toPointF();
        new_pos.setX(0);

        qreal max_y = rect.bottom();
        if(bottom_margin)
            max_y = bottom_margin->pos().y() - 100.0;

        new_pos.setY(qMin(max_y, qMax(new_pos.y(), 0.0)));

        return new_pos;
    }
    return QGraphicsItem::itemChange(change, value);
}

QVariant LeftMargin::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QRectF rect = scene()->sceneRect();
        QPointF new_pos = value.toPointF();
        new_pos.setY(0);

        qreal max_x = rect.right();
        //Runtime error when we check for right_margin.
        //Don't know why in this case runtime crash occurs.
        //Quick hack by defining a right_margin_set flag.
        if(right_margin_set)
            max_x = right_margin->pos().x() - 100;

        new_pos.setX(qMin(max_x, qMax(new_pos.x(), rect.left())));

        return new_pos;
    }
    return QGraphicsItem::itemChange(change, value);
}*/

void ReportTable::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = true;
    hover_spot = event->pos();

    if(qFuzzyCompare(hover_spot.y(), top_left.y()+height))
        setCursor(Qt::SizeVerCursor);
    else if(onCol(hover_spot.x()))
        setCursor(Qt::SizeHorCursor);
    else
        unsetCursor();

    update();
    QGraphicsItem::hoverMoveEvent(event);
}

bool ReportTable::onCol(qreal hover_point_x)
{
    bool match = false;
    for(int i=0; i<enabled_cols.size(); i++) {
        if((match = qFuzzyCompare(hover_point_x, top_left.x() + widthTill(i)))) {
            break;
        }
    }
    if(!match) {
        match = qFuzzyCompare(round(hover_point_x), round(top_left.x()+totalWidth()));
    }
    if(!match) {
        match = qFuzzyCompare(round(hover_point_x), round(top_left.x()));
        on_left_edge = true;
    }
    return match;
}

void ReportTable::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = false;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void ReportTable::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (x_isResizing) {
        int dx = int(event->pos().x() - x_mouse_pressed);
        prepareGeometryChange();
        if(col_to_be_moved < 0) {
            qreal x = top_left.x();
            x += dx;
            top_left.setX(event->pos().x());
            width.replace(0, x_width - dx);
        }
        else {
            width.replace(col_to_be_moved, x_width + dx);
        }
    }
    else if(y_isResizing) {
        int dy = int(event->pos().y() - y_mouse_pressed);
        if(y_height + dy > font().pointSize()) {
            prepareGeometryChange();
            height = y_height + dy;
        }
    }
    else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

/*void ItemBase::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_isResizing || (isInResizeArea(event->pos()) && isSelected()))
        setCursor(Qt::SizeFDiagCursor);
    else
        setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverMoveEvent(event);
}*/

void ReportTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && cursor().shape() == Qt::SizeHorCursor) {
        x_isResizing = true;
        x_mouse_pressed = event->pos().x();
        bool match = false;
        for(int i=1; i<enabled_cols.size(); i++) {
            if(match = qFuzzyCompare(x_mouse_pressed, top_left.x() + widthTill(i))) {
                x_width = getWidth(i-1);
                col_to_be_moved = i-1;
                break;
            }
        }
        if(!match) {
            if(match = qFuzzyCompare(x_mouse_pressed, top_left.x()+totalWidth())) {
                x_width = getWidth(enabled_cols.size()-1);
                col_to_be_moved = enabled_cols.size()-1;
            }
        }
        if(!match) {
            if(match = qFuzzyCompare(x_mouse_pressed, top_left.x())) {
                x_width = getWidth(0);
                col_to_be_moved = -1;
            }
        }
    }
    else if (event->button() == Qt::LeftButton && cursor().shape() == Qt::SizeVerCursor) {
        y_isResizing = true;
        y_height = height;
        y_mouse_pressed = event->pos().y();
    }
    else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void ReportTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && x_isResizing) {
        x_isResizing = false;
        x_mouse_pressed = 0.0;
        col_to_be_moved = 0;
        on_left_edge = false;
    }
    else if (event->button() == Qt::LeftButton && y_isResizing) {
        y_isResizing = false;
        y_mouse_pressed = 0.0;
    }
    else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void ReportTable::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu context_menu;
    //context_menu.addAction(tr("Insert column"));
    if(repeat_header_every_page)
        context_menu.addAction(QIcon(":/icons/ok.svg"), tr("Repeat header on every page"));
    else
        context_menu.addAction(tr("Repeat on every page"));
    context_menu.addAction(tr("Change font..."));
    context_menu.addSeparator();
    context_menu.addAction(tr("Set colour..."));

    hidden_cols_menu.clear();
    for(int i=0; i<enabled_cols.size(); i++) {
        if(!enabled_cols.at(i))
            hidden_cols_menu.addAction(column_header_list.at(i));
    }
    if(hidden_cols_menu.actions().isEmpty()) {
        hidden_cols_menu.setEnabled(false);
    }
    else
        hidden_cols_menu.setEnabled(true);

    context_menu.addSeparator();
    if(column_header_list.size() - hidden_cols_menu.actions().size() == 1)
        context_menu.addAction(tr("Hide column"))->setEnabled(false);
    else
        context_menu.addAction(tr("Hide column"))->setEnabled(true);

    context_menu.addMenu(&hidden_cols_menu);
    context_menu.addSeparator();
    context_menu.addAction(tr("Delete"));

    QAction *a = context_menu.exec(event->screenPos());

    if(a && QString::compare(a->text(), tr("Insert column")) == 0) {
    }
    else if(a && QString::compare(a->text(), tr("Repeat header on every page")) == 0) {
        repeat_header_every_page = !repeat_header_every_page;
    }
    else if(a && QString::compare(a->text(), tr("Change font...")) == 0) {
        bool ok;
        QFont f = QFontDialog::getFont(&ok, font(), 0);
        setFont(f);
        height = f.pointSize()*2;
    }
    else if(a && QString::compare(a->text(), tr("Set colour...")) == 0) {
        QColor c = QColorDialog::getColor(Qt::black, (QWidget*) this->parentWidget());
        if(c.isValid()) {
            setTextColor(event->scenePos(), c);
            scene()->update();
        }
    }
    else if(a && QString::compare(a->text(), tr("Hide column")) == 0) {
        disableColumnAt(event->scenePos());
        scene()->update();
    }
    else if(a && QString::compare(a->text(), tr("Delete")) == 0) {
        emit deletingTable(this);
        deleteLater();
    }
    else {
        for(int i=0; i<enabled_cols.size(); i++) {
            if(a && QString::compare(a->text(), column_header_list.at(i)) == 0)
                enabled_cols.toggleBit(i);
        }
    }
}

ReportView::ReportView(QGraphicsScene &s, ReportWindow *parent,
                       const char *name, Qt::WindowFlags f) :
      QGraphicsView(&s, parent)
{
    report_window = parent;
    setMouseTracking(false);
    setAcceptDrops(true);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

ReportWindow::ReportWindow(Database *database, QString sql)
{
    page_sizes_list << "Letter" << "Legal" << "A3" << "A4" << "A5" << "B4" << "B5"
                    << "Executive" << "US 4x6" << "US 4x8" << "US 5x7" << "Common 10";
    this->database = database;
    this->sql = sql;
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("reportview ").append(sql));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }

        QSqlQueryModel query_model;
        QString pruned_sql = sql.trimmed();
        pruned_sql.remove(';');
        int limit_keyword_pos;
        if((limit_keyword_pos = sql.indexOf("limit", 0, Qt::CaseInsensitive)) > 0)
            pruned_sql.truncate(limit_keyword_pos);

        query_model.setQuery(pruned_sql + " LIMIT 0", database_connection);
        if(query_model.lastError().isValid()) {
            QStringList messages = query_model.lastError().databaseText().split("\n");
            messages.removeLast();
            QMessageBox::critical(this, MainWin::tr("Database error"),
            messages.join("\n"), QMessageBox::Close);
            return;
        }

        if(query_model.query().isSelect()) {
            int column_count = query_model.query().record().count();
            for(int col=0; col < column_count; col++)
                column_list << query_model.query().record().fieldName(col);
        }
        else {
        }
    }
    QSqlDatabase::removeDatabase(QString("reportview ").append(sql));

    createActions();

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("reportwindow");
    toolbar->setMovable(false);
    toolbar->addAction(pdf_print_action);
    toolbar->addWidget(font_combo_box);
    addToolBar(toolbar);

    progress_dialog = new QProgressDialog(tr("Creating file..."), tr("Cancel"), 0, 0, this);
    progress_dialog->setAttribute(Qt::WA_DeleteOnClose);
    progress_dialog->setWindowModality(Qt::WindowModal);

    QToolButton *label = new QToolButton;
    label->setToolTip(tr("Label"));
    label->setIcon(QIcon(":/icons/label.png"));
    label->setIconSize(QSize(48,36));
    QToolButton *databox = new QToolButton;
    databox->setToolTip(tr("Databox"));
    databox->setIcon(QIcon(":/icons/databox.png"));
    databox->setIconSize(QSize(48,36));
    QToolButton *table = new QToolButton;
    table->setToolTip(tr("Table"));
    table->setIcon(QIcon(":/icons/report_table.png"));
    table->setIconSize(QSize(48,36));
    QToolButton *hline = new QToolButton;
    hline->setToolTip(tr("Horizontal line"));
    hline->setIcon(QIcon(":/icons/hline.png"));
    hline->setIconSize(QSize(48,36));

    button_group = new QButtonGroup;
    button_group->addButton(label, 1);
    button_group->addButton(databox, 2);
    button_group->addButton(table, 3);
    button_group->addButton(hline, 4);

    QHBoxLayout *item_layout = new QHBoxLayout;
    item_layout->addWidget(label);
    item_layout->addWidget(databox);
    item_layout->addWidget(table);
    item_layout->addWidget(hline);
    item_layout->addStretch();

    setOrientation(ReportWindow::Portrait);
    setPageSize(ReportWindow::A4);

    scene.setSceneRect(pages_sizes[pageSize()]);
    report_view = new ReportView(scene, this);
    report_view->setDragMode(QGraphicsView::RubberBandDrag);

    //initMargins();

    connect(button_group, SIGNAL(buttonPressed(int)), report_view, SLOT(changeCursor(int)));
    connect(report_view, SIGNAL(droppedLabel(QPointF)), SLOT(drawLabel(QPointF)));
    connect(report_view, SIGNAL(droppedDatabox(QPointF)), SLOT(drawDatabox(QPointF)));
    connect(report_view, SIGNAL(droppedTable(QPointF)), SLOT(drawTable(QPointF)));
    connect(report_view, SIGNAL(droppedHLine(QPointF)), SLOT(drawHLine(QPointF)));

    connect(&scene, &QGraphicsScene::changed, this, &ReportWindow::togglePrint);

    QWidget *main = new QWidget;

    QVBoxLayout *labelLayout = new QVBoxLayout;
    labelLayout->addLayout(item_layout);
    labelLayout->addWidget(report_view);

    main->setLayout(labelLayout);

    setCentralWidget(main);

    //Define shortcuts
    QShortcut *cancel = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(cancel, SIGNAL(activated()), SLOT(restore()));

    QShortcut *select_all = new QShortcut(QKeySequence::SelectAll, this);
    connect(select_all, SIGNAL(activated()), SLOT(selectAll()));

    QShortcut *shortcut_del = new QShortcut(QKeySequence::Delete, this);
    connect(shortcut_del, SIGNAL(activated()), SLOT(deleteSeletectedItems()));

    QShortcut *shortcut_default_view = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this);
    connect(shortcut_default_view, SIGNAL(activated()), SLOT(noZoom()));

    connect(this, &ReportWindow::orientationChanged, this, &ReportWindow::changeOrientation);
    connect(this, &ReportWindow::pageSizeChanged, [this](){if(orientation() == ReportWindow::Portrait) {scene.setSceneRect(pages_sizes[pageSize()]);}
                                                           else if(orientation() == ReportWindow::Landscape) {qreal w = pages_sizes[pageSize()].width(); qreal h = pages_sizes[pageSize()].height(); scene.setSceneRect(0, 0, h, w);} });
}

void ReportWindow::deleteSeletectedItems()
{
    QList<QGraphicsItem*> selected_items = scene.selectedItems();

    foreach(QGraphicsItem* item, selected_items) {
        scene.removeItem(item);
        label_list.removeOne(static_cast<ReportLabel *>(item));
        hline_list.removeOne(static_cast<QGraphicsLineItem *>(item));
        table_list.removeOne(static_cast<ReportTable *>(item));
        delete item;
    }
}

void ReportWindow::noZoom()
{
    report_view->resetTransform();
}

void ReportView::changeCursor(int i)
{
    switch(i) {
        case 1: {
            drag = new QDrag(this);
            mime_data = new QMimeData;
            mime_data->setText("label");
            drag->setMimeData(mime_data);
            drag->setPixmap(QPixmap(":/icons/label.png"));
            //drag->setHotSpot(QPoint(drag->pixmap().width()/2,
            //                        drag->pixmap().height()));
            drag->exec();
            break;
        }
        case 2: {
            drag = new QDrag(this);
            mime_data = new QMimeData;
            mime_data->setText("databox");
            drag->setMimeData(mime_data);
            drag->setPixmap(QPixmap(":/icons/databox.png"));
            //drag->setHotSpot(QPoint(drag->pixmap().width()/2,
            //                        drag->pixmap().height()));
            drag->exec();
            break;
        }
        case 3: {
            drag = new QDrag(this);
            mime_data = new QMimeData;
            mime_data->setText("table");
            drag->setMimeData(mime_data);
            drag->setPixmap(QPixmap(":/icons/report_table.png"));
            //drag->setHotSpot(QPoint(drag->pixmap().width()/2,
            //                        drag->pixmap().height()));
            drag->exec();
            break;
        }
        case 4: {
            drag = new QDrag(this);
            mime_data = new QMimeData;
            mime_data->setText("hline");
            drag->setMimeData(mime_data);
            drag->setPixmap(QPixmap(":/icons/hline.png"));
            //drag->setHotSpot(QPoint(drag->pixmap().width()/2,
            //                        drag->pixmap().height()));
            drag->exec();
            break;
        }
        default:
            break;
    }
}

void ReportView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData())
        event->acceptProposedAction();
    else
        event->ignore();
}

void ReportView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData())
        event->acceptProposedAction();
    else
        event->ignore();
}

void ReportView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()) {
        event->acceptProposedAction();
        if(event->mimeData()->text().compare("label") == 0)
            emit droppedLabel(mapToScene(event->pos()));
        else if(event->mimeData()->text().compare("databox") == 0)
            emit droppedDatabox(mapToScene(event->pos()));
        else if(event->mimeData()->text().compare("table") == 0)
            emit droppedTable(mapToScene(event->pos()));
        else if(event->mimeData()->text().compare("hline") == 0)
            emit droppedHLine(mapToScene(event->pos()));
    }
    else {
        event->ignore();
    }
}

void ReportView::wheelEvent(QWheelEvent *wheelEvent)
{
    //wheelEvent->accept();

    //Capture Control key pressed with mouse wheel
    //events to zoom-in or zoom-out the canvas.
    if(wheelEvent->modifiers() == Qt::ControlModifier) {
        //zoom out
        if (wheelEvent->delta()>0)
            scale( 0.9, 0.9 );
        //zoom in
        else
            scale( 1.1, 1.1 );
    }
    //Shift key pressed mouse wheel should scroll the canvas left-right.
    else if(wheelEvent->modifiers() == Qt::ShiftModifier) {
        int value = horizontalScrollBar()->value();
        horizontalScrollBar()->setValue(value - (wheelEvent->delta() >> 1));
    }
    else {
        QGraphicsView::wheelEvent(wheelEvent);
    }
}

/*void ReportWindow::initMargins()
{
    left_margin = new LeftMargin(&scene);
    left_margin->setLine(0,0,0,842);
    left_margin->setPos(70,0);

    right_margin = new RightMargin(&scene);
    right_margin->setLine(0,0,0,842);
    right_margin->setPos(525,0);

    right_margin->setLeftMargin(left_margin);
    left_margin->setRightMargin(right_margin);

    top_margin = new TopMargin(&scene);
    top_margin->setLine(0,0,595,0);
    top_margin->setPos(0,70);
    bottom_margin = new BottomMargin(&scene);
    bottom_margin->setLine(0,0,595,0);
    bottom_margin->setPos(0,772);

    bottom_margin->setTopMargin(top_margin);
    top_margin->setBottomMargin(bottom_margin);
}*/

void ReportWindow::restore()
{
    scene.setSelectionArea(QPainterPath());
}

void ReportWindow::togglePrint()
{
    if(scene.items().isEmpty()) {
        pdf_print_action->setEnabled(false);
    }
    else {
        pdf_print_action->setEnabled(true);
    }
}

void ReportWindow::selectAll()
{
    foreach(QGraphicsItem *item, scene.items())
        item->setSelected(true);
}

void ReportWindow::drawLabel(QPointF pos)
{
    report_view->setFocus(Qt::ActiveWindowFocusReason);
    ReportLabel *new_label = new ReportLabel(pos);
    new_label->setParent(this);
    new_label->setFocus(Qt::ActiveWindowFocusReason);
    connect(new_label, SIGNAL(deletingLabel(ReportLabel*)), SLOT(labelDeleted(ReportLabel*)));
    label_list.append(new_label);
    scene.addItem(new_label);
}

void ReportWindow::labelDeleted(ReportLabel *label)
{
    label_list.removeOne(label);
}

void ReportWindow::drawDatabox(QPointF pos)
{
    QComboBox *databox = new QComboBox;
    databox->addItems(column_list);
    QGraphicsProxyWidget *databox_proxy = scene.addWidget(databox);
    databox_proxy->setPos(pos);
    databox_proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
}

void ReportWindow::drawTable(QPointF pos)
{
    ReportTable *report_table = new ReportTable(pos, column_list);
    connect(report_table, SIGNAL(deletingTable(ReportTable*)), SLOT(tableDeleted(ReportTable*)));
    scene.addItem(report_table);
    report_table->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    table_list.append(report_table);
}

void ReportWindow::tableDeleted(ReportTable *report_table)
{
    table_list.removeOne(report_table);
}

void ReportWindow::drawHLine(QPointF pos)
{
    QGraphicsLineItem *hline = scene.addLine(70, 0, 525, 0,QPen(Qt::SolidLine));
    hline->setPos(0, pos.y());
    hline->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    hline_list.append(hline);
}

void ReportWindow::createActions()
{
    pdf_print_action = new QAction(QIcon(":/icons/pdf_print.svg"), tr("Create PDF"), this);
    pdf_print_action->setShortcuts(QKeySequence::Print);
    pdf_print_action->setStatusTip(tr("Print to a PDF document"));
    connect(pdf_print_action, &QAction::triggered, this, &ReportWindow::pdfPrint);

    html_print_action = new QAction(QIcon(":/icons/html.png"), tr("Create HTML"), this);
    //html_print_action->setShortcuts(QKeySequence::Print);
    pdf_print_action->setStatusTip(tr("Print to an HTML document"));
    connect(html_print_action, &QAction::triggered, this, &ReportWindow::htmlPrint);

    odf_print_action = new QAction(QIcon(":/icons/odt.png"), tr("Create OpenDocument file"), this);
    //html_print_action->setShortcuts(QKeySequence::Print);
    odf_print_action->setStatusTip(tr("Print to an HTML document"));
    connect(odf_print_action, &QAction::triggered, this, &ReportWindow::odfPrint);

    orientation_action = new QAction(QIcon(":/icons/portrait.png"), tr("Toggle page orientation"), this);
    connect(orientation_action, &QAction::triggered, this, &ReportWindow::changeOrientation);

    font_combo_box = new QComboBox(this);
    font_combo_box->addItem("MS-Mincyo");
    font_combo_box->addItem("MSP-Mincyo");
    font_combo_box->addItem("MS-Gothic");
    font_combo_box->addItem("MSP-Gothic");
    connect(font_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(setSelectedItemsFont(int)));

    print_cancelled = false;
}

void ReportWindow::pdfPrint()
{
    QString file_name = QFileDialog::getSaveFileName(this,
             tr("Open PDF file"), "C:/", tr("PDF Files (*.pdf)"));

    if(file_name.isEmpty())
        return;

    QFile file(file_name);
    while(!file.open(QIODevice::WriteOnly)) {
        int ret = QMessageBox::critical(this, tr("Print error"),
            tr("Cannot create or open file <b>%1</b>. Is it already open?").arg(file_name),
            QMessageBox::Retry | QMessageBox::Close);
        if(ret == QMessageBox::Close)
            return;
        else
            continue;
    }

    print_cancelled = false;

    progress_dialog->show();
    connect(progress_dialog, SIGNAL(canceled()), SLOT(printingCancelled()));

    QtConcurrent::run(this, &ReportWindow::pdfPrintThread, file_name);

    connect(this, SIGNAL(done()), progress_dialog, SLOT(hide()));
}

void ReportWindow::htmlPrint()
{
    QString file_name = QFileDialog::getSaveFileName(this,
             tr("Open HTML file"), "C:/", tr("HTML Files (*.html)"));

    if(file_name.isEmpty())
        return;

    QFile file(file_name);
    while(!file.open(QIODevice::WriteOnly)) {
        int ret = QMessageBox::critical(this, tr("Print error"),
            tr("Cannot create or open file <b>%1</b>. Is it already open?").arg(file_name),
            QMessageBox::Retry | QMessageBox::Close);
        if(ret == QMessageBox::Close)
            return;
        else
            continue;
    }

    print_cancelled = false;

    progress_dialog->show();
    connect(progress_dialog, SIGNAL(canceled()), SLOT(printingCancelled()));

    QtConcurrent::run(this, &ReportWindow::htmlPrintThread, file_name);

    connect(this, SIGNAL(done()), progress_dialog, SLOT(hide()));
}

void ReportWindow::odfPrint()
{
    QString file_name = QFileDialog::getSaveFileName(this,
             tr("Open ODT file"), "C:/", tr("ODT Files (*.odt)"));

    if(file_name.isEmpty())
        return;

    QFile file(file_name);
    while(!file.open(QIODevice::WriteOnly)) {
        int ret = QMessageBox::critical(this, tr("Print error"),
            tr("Cannot create or open file <b>%1</b>. Is it already open?").arg(file_name),
            QMessageBox::Retry | QMessageBox::Close);
        if(ret == QMessageBox::Close)
            return;
        else
            continue;
    }

    print_cancelled = false;

    progress_dialog->show();
    connect(progress_dialog, SIGNAL(canceled()), SLOT(printingCancelled()));

    QtConcurrent::run(this, &ReportWindow::odfPrintThread, file_name);

    connect(this, SIGNAL(done()), progress_dialog, SLOT(hide()));
}

void ReportWindow::printingCancelled()
{
    print_cancelled = true;
}

void ReportWindow::pdfPrintThread(QString file_name)
{
    HPDF_Doc  pdf;
    HPDF_Page page;
    HPDF_Font jfont[4];
    HPDF_Font font[12];
    HPDF_REAL page_height;
    HPDF_REAL page_width;

    pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        //QMessageBox::critical(this, tr("Print error"),
        //    tr("Cannot create or open file. Is it already open?"), QMessageBox::Cancel);
        qDebug() << tr("Cannot create or open file. Is it already open?");
        return;
    }

    HPDF_AddPageLabel(pdf, 0, HPDF_PAGE_NUM_STYLE_DECIMAL, 1, "");

    try {
        page = HPDF_AddPage (pdf);

        HPDF_PageSizes p_size;

        switch(page_size) {
        case ReportWindow::Letter:
            p_size = HPDF_PAGE_SIZE_LETTER;
            break;
        case ReportWindow::Legal:
            p_size = HPDF_PAGE_SIZE_LEGAL;
            break;
        case ReportWindow::A3:
            p_size = HPDF_PAGE_SIZE_A3;
            break;
        case ReportWindow::A4:
            p_size = HPDF_PAGE_SIZE_A4;
            break;
        case ReportWindow::A5:
            p_size = HPDF_PAGE_SIZE_A5;
            break;
        case ReportWindow::B4:
            p_size = HPDF_PAGE_SIZE_B4;
            break;
        case ReportWindow::B5:
            p_size = HPDF_PAGE_SIZE_B5;
            break;
        case ReportWindow::US4x6:
            p_size = HPDF_PAGE_SIZE_US4x6;
            break;
        case ReportWindow::US4x8:
            p_size = HPDF_PAGE_SIZE_US4x8;
            break;
        case ReportWindow::US5x7:
            p_size = HPDF_PAGE_SIZE_US5x7;
            break;
        case ReportWindow::Comm10:
            p_size = HPDF_PAGE_SIZE_COMM10;
            break;
        default:
            p_size = HPDF_PAGE_SIZE_A4;
            break;
        }

        switch(o) {
        case ReportWindow::Landscape:
            HPDF_Page_SetSize(page, p_size, HPDF_PAGE_LANDSCAPE);
            break;
        default:
            HPDF_Page_SetSize(page, p_size, HPDF_PAGE_PORTRAIT);
        }

        page_height = HPDF_Page_GetHeight (page);
        page_width = HPDF_Page_GetWidth (page);

        HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

        HPDF_UseJPFonts(pdf);
        HPDF_UseJPEncodings(pdf);

        /*jfont[0] = HPDF_GetFont (pdf, "MS-Mincyo", "90ms-RKSJ-H");
        jfont[1] = HPDF_GetFont (pdf, "MS-Mincyo,Bold", "90ms-RKSJ-H");
        jfont[2] = HPDF_GetFont (pdf, "MS-Mincyo,Italic", "90ms-RKSJ-H");
        jfont[3] = HPDF_GetFont (pdf, "MS-Mincyo,BoldItalic", "90ms-RKSJ-H");
        jfont[4] = HPDF_GetFont (pdf, "MS-PMincyo", "90msp-RKSJ-H");
        jfont[5] = HPDF_GetFont (pdf, "MS-PMincyo,Bold", "90msp-RKSJ-H");
        jfont[6] = HPDF_GetFont (pdf, "MS-PMincyo,Italic", "90msp-RKSJ-H");
        jfont[7] = HPDF_GetFont (pdf, "MS-PMincyo,BoldItalic", "90msp-RKSJ-H");
        jfont[8] = HPDF_GetFont (pdf, "MS-Gothic", "90ms-RKSJ-H");
        jfont[9] = HPDF_GetFont (pdf, "MS-Gothic,Bold", "90ms-RKSJ-H");
        jfont[10] = HPDF_GetFont (pdf, "MS-Gothic,Italic", "90ms-RKSJ-H");
        jfont[11] = HPDF_GetFont (pdf, "MS-Gothic,BoldItalic", "90ms-RKSJ-H");
        jfont[12] = HPDF_GetFont (pdf, "MS-PGothic", "90msp-RKSJ-H");
        jfont[13] = HPDF_GetFont (pdf, "MS-PGothic,Bold", "90msp-RKSJ-H");
        jfont[14] = HPDF_GetFont (pdf, "MS-PGothic,Italic", "90msp-RKSJ-H");
        jfont[15] = HPDF_GetFont (pdf, "MS-PGothic,BoldItalic", "90msp-RKSJ-H");*/

        jfont[0] = HPDF_GetFont (pdf, "MS-Mincyo", "90ms-RKSJ-H");
        jfont[1] = HPDF_GetFont (pdf, "MS-PMincyo", "90msp-RKSJ-H");
        jfont[2] = HPDF_GetFont (pdf, "MS-Gothic", "90ms-RKSJ-H");
        jfont[3] = HPDF_GetFont (pdf, "MS-PGothic", "90msp-RKSJ-H");


        font[0] = HPDF_GetFont (pdf, "Courier", NULL);
        font[1] = HPDF_GetFont (pdf, "Courier-Bold", NULL);
        font[2] = HPDF_GetFont (pdf, "Courier-Oblique", NULL);
        font[3] = HPDF_GetFont (pdf, "Courier-BoldOblique", NULL);
        font[4] = HPDF_GetFont (pdf, "Helvetica", NULL);
        font[5] = HPDF_GetFont (pdf, "Helvetica-Bold", NULL);
        font[6] = HPDF_GetFont (pdf, "Helvetica-Oblique", NULL);
        font[7] = HPDF_GetFont (pdf, "Helvetica-BoldOblique", NULL);
        font[8] = HPDF_GetFont (pdf, "Times-Roman", NULL);
        font[9] = HPDF_GetFont (pdf, "Times-Bold", NULL);
        font[10] = HPDF_GetFont (pdf, "Times-Italic", NULL);
        font[11] = HPDF_GetFont (pdf, "Times-BoldItalic", NULL);

        int size = 10;

        HPDF_Page_SetFontAndSize (page, jfont[0], size);

        foreach(QGraphicsTextItem *label, label_list) {
            HPDF_Page_SetRGBFill(page,
                                 label->defaultTextColor().redF(),
                                 label->defaultTextColor().greenF(),
                                 label->defaultTextColor().blueF());
            size = label->font().pointSize();

            HPDF_Page_BeginText (page);
            HPDF_Page_MoveTextPos (page, label->x(), page_height - label->y() - label->boundingRect().height());

            QByteArray lba;
            if(label->toPlainText().toLatin1().length() == label->toPlainText().toLocal8Bit().length()) {
                lba = label->toPlainText().toLatin1();
                HPDF_Page_SetFontAndSize (page, font[4], size);
            }
            else {
                lba = text_codec->fromUnicode(label->toPlainText());
                HPDF_Page_SetFontAndSize (page, jfont[0], size);
            }
            const char *l = lba.data();

            HPDF_Page_ShowText (page, l);
            HPDF_Page_EndText (page);
        }

        foreach(ReportTable *report_table, table_list) {
            int column_size = column_list.size();
            size = report_table->font().pointSize();

            HPDF_Page_SetFontAndSize (page, jfont[report_table->japaneseFont()], size);
            for(int col=0; col<column_size; col++) {
                if(report_table->getColumnEnabled(col)) {
                    HPDF_Page_SetRGBStroke(page,0.0,0.0,0.0);
                    HPDF_Page_SetRGBFill(page,0.9,0.9,0.9);
                    HPDF_Page_Rectangle(page, report_table->getTopLeft().x() + report_table->x() + (report_table->widthTill(col)),
                                         page_height - report_table->getTopLeft().y() - report_table->y() - report_table->getHeight(),
                                         report_table->getWidth(col),
                                         report_table->getHeight());
                    HPDF_Page_FillStroke (page);

                    QByteArray tba = text_codec->fromUnicode(QString(column_list.at(col)));
                    const char *text = tba.constData();

                    HPDF_Page_SetRGBFill(page,0.1,0.1,0.1);
                    HPDF_Page_BeginText (page);
                    HPDF_Page_TextRect(page, report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col),
                                        page_height - report_table->getTopLeft().y() - report_table->y(),
                                        report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col) + report_table->getWidth(col),
                                        page_height - report_table->getTopLeft().y() - report_table->y() - report_table->getHeight(),
                                        text, HPDF_TALIGN_CENTER | HPDF_TALIGN_MIDDLE, NULL);
                    HPDF_Page_EndText (page);
                }
            }
        }

        {
            QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("reportview ").append(sql));
            database_connection.setHostName(database->getHost());
            database_connection.setPort(database->getPort().toInt());
            database_connection.setDatabaseName(database->getName());
            database_connection.setUserName(database->getUser());
            database_connection.setPassword(database->getPassword());
            if (!database_connection.open()) {
                QMessageBox::critical(this, MainWin::tr("Database error"),
                    MainWin::tr("Unable to establish a database connection.\n"
                                "No PostgreSQL support.\n"), QMessageBox::Cancel);
                return;
            }

            QSqlQuery sql_query(sql, database_connection);

            int row_count = 1;
            qreal bottom_point = page_height - 70;

            while(sql_query.next()) {
                int wrapped_rows = 1;
                foreach(ReportTable *report_table, table_list) {
                    size = report_table->font().pointSize();
                    HPDF_Page_SetFontAndSize (page, jfont[report_table->japaneseFont()], size);
                    int column_size = column_list.size();
                    for(int col=0; col<column_size; col++) {
                        if(print_cancelled)
                            return;
                        if(report_table->getColumnEnabled(col)) {
                            QString value = sql_query.value(col).toString();
                            if(!value.isEmpty()) {
                                QByteArray tba = text_codec->fromUnicode(value);
                                const char *text = tba.constData();
                                QStringList values;
                                QString tmp_value(value);

                                HPDF_Page_BeginText(page);

                                HPDF_REAL rw = 0.0;

                                while(!tmp_value.isEmpty()) {
                                    tba = text_codec->fromUnicode(tmp_value);
                                    text = tba.constData();
                                    int length = HPDF_Page_MeasureText (page, text, (report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col) + report_table->getWidth(col)) - (report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col)), HPDF_TRUE, &rw);
                                    if(length) {
                                        QByteArray tmp_tba(text, length);
                                        tmp_value = text_codec->toUnicode(tmp_tba);
                                        values << tmp_value;
                                        tmp_value = value.remove(0, tmp_value.length());
                                    }
                                    else {
                                        tmp_value.chop(1);
                                    }
                                }
                                wrapped_rows = values.size() > wrapped_rows ? values.size() : wrapped_rows;

                                HPDF_Page_SetRGBFill(page,
                                                     report_table->textColor(col).redF(),
                                                     report_table->textColor(col).greenF(),
                                                     report_table->textColor(col).blueF());
                                int row = 0;
                                foreach (QString text_row, values) {
                                    tba = text_codec->fromUnicode(text_row);
                                    text = tba.constData();
                                    HPDF_Page_TextRect(page, report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col),
                                                       page_height - report_table->getTopLeft().y() - report_table->y() - (row_count+row)*report_table->getHeight(),
                                                       report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col) + report_table->getWidth(col),
                                                       page_height - report_table->getTopLeft().y() - report_table->y() - (row_count+row+1)*report_table->getHeight(),
                                                       text, HPDF_TALIGN_MIDDLE, NULL);
                                    row++;
                                }
                                HPDF_Page_EndText(page);
                            }
                        }
                    }
                    for(int col=0; col<column_size; col++) {
                        if(report_table->getColumnEnabled(col)) {
                            HPDF_Page_Rectangle(page, report_table->getTopLeft().x() + report_table->x() + (report_table->widthTill(col)),
                                                 page_height - report_table->getTopLeft().y() - report_table->y() - (row_count+wrapped_rows)*report_table->getHeight(),
                                                 report_table->getWidth(col),
                                                 report_table->getHeight()*wrapped_rows);
                            HPDF_Page_Stroke(page);
                        }
                    }
                    bottom_point = page_height - report_table->getTopLeft().y() - report_table->y() - (row_count+wrapped_rows)*report_table->getHeight();
                }
                row_count += wrapped_rows;
                wrapped_rows = 1;

                if(bottom_point < 70 && sql_query.next()) {
                    sql_query.previous();
                    page = HPDF_AddPage (pdf);

                    HPDF_PageSizes p_size;

                    switch(page_size) {
                    case ReportWindow::Letter:
                        p_size = HPDF_PAGE_SIZE_LETTER;
                        break;
                    case ReportWindow::Legal:
                        p_size = HPDF_PAGE_SIZE_LEGAL;
                        break;
                    case ReportWindow::A3:
                        p_size = HPDF_PAGE_SIZE_A3;
                        break;
                    case ReportWindow::A4:
                        p_size = HPDF_PAGE_SIZE_A4;
                        break;
                    case ReportWindow::A5:
                        p_size = HPDF_PAGE_SIZE_A5;
                        break;
                    case ReportWindow::B4:
                        p_size = HPDF_PAGE_SIZE_B4;
                        break;
                    case ReportWindow::B5:
                        p_size = HPDF_PAGE_SIZE_B5;
                        break;
                    case ReportWindow::US4x6:
                        p_size = HPDF_PAGE_SIZE_US4x6;
                        break;
                    case ReportWindow::US4x8:
                        p_size = HPDF_PAGE_SIZE_US4x8;
                        break;
                    case ReportWindow::US5x7:
                        p_size = HPDF_PAGE_SIZE_US5x7;
                        break;
                    case ReportWindow::Comm10:
                        p_size = HPDF_PAGE_SIZE_COMM10;
                        break;
                    default:
                        p_size = HPDF_PAGE_SIZE_A4;
                        break;
                    }

                    switch(o) {
                    case ReportWindow::Landscape:
                        HPDF_Page_SetSize(page, p_size, HPDF_PAGE_LANDSCAPE);
                        break;
                    default:
                        HPDF_Page_SetSize(page, p_size, HPDF_PAGE_PORTRAIT);
                    }

                    HPDF_Page_SetFontAndSize (page, jfont[0], 10);
                    row_count = 1;

                    foreach(ReportLabel *label, label_list) {
                        if(label->repeatOnEveryPage()) {
                            HPDF_Page_SetRGBFill(page,
                                                 label->defaultTextColor().redF(),
                                                 label->defaultTextColor().greenF(),
                                                 label->defaultTextColor().blueF());
                            size = label->font().pointSize();

                            HPDF_Page_BeginText (page);
                            HPDF_Page_MoveTextPos (page, label->x(), page_height - label->y() - label->boundingRect().height());

                            QByteArray lba;
                            if(label->toPlainText().toLatin1().length() == label->toPlainText().toLocal8Bit().length()) {
                                lba = label->toPlainText().toLatin1();
                                HPDF_Page_SetFontAndSize (page, font[4], size);
                            }
                            else {
                                lba = text_codec->fromUnicode(label->toPlainText());
                                HPDF_Page_SetFontAndSize (page, jfont[0], size);
                            }
                            const char *l = lba.data();

                            HPDF_Page_ShowText (page, l);
                            HPDF_Page_EndText (page);
                        }
                    }
                    HPDF_Page_SetRGBStroke(page,0.0,0.0,0.0);

                    foreach(ReportTable *report_table, table_list) {
                        if(report_table->repeatHeaderEveryPage()) {
                            int column_size = column_list.size();
                            size = report_table->font().pointSize();
                            HPDF_Page_SetFontAndSize (page, jfont[report_table->japaneseFont()], size);
                            for(int col=0; col<column_size; col++) {
                                if(report_table->getColumnEnabled(col)) {
                                    HPDF_Page_SetRGBStroke(page,0.0,0.0,0.0);
                                    HPDF_Page_SetRGBFill(page,0.9,0.9,0.9);
                                    HPDF_Page_Rectangle(page, report_table->getTopLeft().x() + report_table->x() + (report_table->widthTill(col)),
                                                         page_height - report_table->getTopLeft().y() - report_table->y() - report_table->getHeight(),
                                                         report_table->getWidth(col),
                                                         report_table->getHeight());
                                    HPDF_Page_FillStroke (page);

                                    QByteArray tba = text_codec->fromUnicode(QString(column_list.at(col)));
                                    const char *text = tba.data();

                                    HPDF_Page_SetRGBFill(page,0.1,0.1,0.1);
                                    HPDF_Page_BeginText (page);
                                    HPDF_Page_TextRect(page, report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col),
                                                        page_height - report_table->getTopLeft().y() - report_table->y(),
                                                        report_table->getTopLeft().x() + report_table->x() + report_table->widthTill(col) + report_table->getWidth(col),
                                                        page_height - report_table->getTopLeft().y() - report_table->y() - report_table->getHeight(),
                                                        text, HPDF_TALIGN_CENTER | HPDF_TALIGN_MIDDLE, NULL);
                                    HPDF_Page_EndText (page);
                                }
                            }
                        }
                    }
                }
            }
        }

        HPDF_Page_SetLineWidth (page, 1.0);
        foreach(QGraphicsLineItem *hline, hline_list) {
            drawPDFLine(page, 70, hline->y());
        }

        QByteArray fba = text_codec->fromUnicode(file_name);
        const char *f = fba.data();
        HPDF_SaveToFile(pdf, f);
        HPDF_Free(pdf);
    } catch (...) {
        HPDF_Free(pdf);
        return;
    }

    QSqlDatabase::removeDatabase(QString("reportview ").append(sql));

    HPDF_FreeDocAll(pdf);

    emit done();
}

void ReportWindow::htmlPrintThread(QString file_name)
{
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("reportview ").append(sql));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }

        QSqlQuery sql_query(sql, database_connection);

        QTextDocument* td = new QTextDocument();
        td->setPageSize(QSizeF(595.276,841.89));
        td->setDocumentMargin(70.0);

        QTextCursor cursor(td);
        QTextFrame *main_frame = cursor.currentFrame();
        QTextCharFormat tcf;

        QTextFrameFormat tff;
        tff.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        tff.setBorderBrush(QBrush(Qt::black));
        tff.setBorder(2);

        foreach(QGraphicsTextItem *label, label_list) {
            tcf.setFont(label->font());
            cursor.setCharFormat(tcf);

            QTextFrame *tf = cursor.insertFrame(tff);
            cursor = tf->firstCursorPosition();
            cursor.insertText(label->toPlainText());
            cursor = main_frame->lastCursorPosition();
        }

        QTextTableFormat table_format;
        table_format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        table_format.setBorderBrush(QBrush(Qt::black));
        table_format.setBorder(0.2);
        table_format.setCellPadding(0.0);
        table_format.setCellSpacing(0.2);
        table_format.setMargin(0.0);

        QTextTable* tb;

        int column_size = column_list.size();
        QTextTableCell tc;
        foreach(ReportTable *report_table, table_list) {
            table_format.setWidth(report_table->totalWidth());
            QVector<QTextLength> constraints;
            constraints << QTextLength(QTextLength::VariableLength, report_table->totalWidth());
            table_format.setColumnWidthConstraints(constraints);
            tb = cursor.insertTable(1, column_list.size(), table_format);
            for(int col=0; col<column_size; col++) {
                if(report_table->getColumnEnabled(col)) {
                    tc = tb->cellAt(0, col);
                    cursor = tc.firstCursorPosition();
                    cursor.insertText(column_list.at(col));
                }
            }
        }

        int row = 1;
        while(sql_query.next()) {
            foreach(ReportTable *report_table, table_list) {
                tb->appendRows(1);
                for(int col=0; col<column_size; col++) {
                    if(print_cancelled)
                        return;
                    if(report_table->getColumnEnabled(col)) {
                        tc = tb->cellAt(row++, col);
                        cursor = tc.firstCursorPosition();
                        cursor.insertText(sql_query.value(col).toString());
                    }
                }
            }
        }

        QTextDocumentWriter writer(file_name, "html");
        writer.write(td);
    }

    QSqlDatabase::removeDatabase(QString("reportview ").append(sql));

    emit done();
}

void ReportWindow::odfPrintThread(QString file_name)
{
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("reportview ").append(sql));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }

        QSqlQuery sql_query(sql, database_connection);

        QTextDocument* td = new QTextDocument();
        td->setPageSize(QSizeF(595.276,841.89));
        td->setDocumentMargin(70.0);

        QTextCursor cursor(td);
        QTextFrame *main_frame = cursor.currentFrame();
        QTextCharFormat tcf;

        QTextFrameFormat tff;
        tff.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        tff.setBorderBrush(QBrush(Qt::black));
        tff.setBorder(2);

        foreach(QGraphicsTextItem *label, label_list) {
            tcf.setFont(label->font());
            cursor.setCharFormat(tcf);

            QTextFrame *tf = cursor.insertFrame(tff);
            cursor = tf->firstCursorPosition();
            cursor.insertText(label->toPlainText());
            cursor = main_frame->lastCursorPosition();
        }

        QTextTableFormat table_format;
        table_format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        table_format.setBorderBrush(QBrush(Qt::black));
        table_format.setBorder(0.2);
        table_format.setCellPadding(0.0);
        table_format.setCellSpacing(0.2);
        table_format.setMargin(0.0);

        QTextTable* tb;

        int column_size = column_list.size();
        QTextTableCell tc;
        foreach(ReportTable *report_table, table_list) {
            table_format.setWidth(report_table->totalWidth());
            QVector<QTextLength> constraints;
            constraints << QTextLength(QTextLength::VariableLength, report_table->totalWidth());
            table_format.setColumnWidthConstraints(constraints);
            tb = cursor.insertTable(1, column_list.size(), table_format);
            for(int col=0; col<column_size; col++) {
                if(report_table->getColumnEnabled(col)) {
                    tc = tb->cellAt(0, col);
                    cursor = tc.firstCursorPosition();
                    cursor.insertText(column_list.at(col));
                }
            }
        }

        int row = 1;
        while(sql_query.next()) {
            foreach(ReportTable *report_table, table_list) {
                tb->appendRows(1);
                for(int col=0; col<column_size; col++) {
                    if(print_cancelled)
                        return;
                    if(report_table->getColumnEnabled(col)) {
                        tc = tb->cellAt(row++, col);
                        cursor = tc.firstCursorPosition();
                        cursor.insertText(sql_query.value(col).toString());
                    }
                }
            }
        }

        QTextDocumentWriter writer(file_name, "odf");
        writer.write(td);
    }

    QSqlDatabase::removeDatabase(QString("reportview ").append(sql));

    emit done();
}

void ReportWindow::changeOrientation()
{
    qreal w = scene.width();
    qreal h = scene.height();
    scene.setSceneRect(0, 0, h, w);
}

void ReportWindow::setSelectedItemsFont(int f)
{
    foreach(QGraphicsTextItem *label, label_list) {
        if(label->isSelected()) {
            label->update();
        }
    }
    foreach(ReportTable *table, table_list) {
        if(table->isSelected()) {
            table->setJapaneseFont((ReportTable::JapaneseFont)f);
            table->update();
        }
    }
}

void ReportWindow::drawPDFLine(HPDF_Page page, float x, float y)
{
    HPDF_Page_MoveTo (page, 70, 842-y);
    HPDF_Page_LineTo (page, 525, 842-y);
    HPDF_Page_Stroke (page);
}

void ReportWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu context_menu;
    QAction *orientation_action = context_menu.addAction(tr("Orientation"));
    QAction *size_action = context_menu.addAction(tr("Size"));

    QAction *a = context_menu.exec(event->globalPos());

    QStringList orientations;
    orientations << "Portrait" << "Landscape";

    if(a) {
        if(a == orientation_action) {
            QString item = QInputDialog::getItem(this, tr("Orientation"), "", orientations, orientation(), false);
            if(item.compare(tr("Portrait")) == 0)
                setOrientation(ReportWindow::Portrait);
            else if(item.compare(tr("Landscape")) == 0)
                setOrientation(ReportWindow::Landscape);
            else
                setOrientation(ReportWindow::Portrait);
        }
        else if(a == size_action) {
            QString item = QInputDialog::getItem(this, tr("Size"), "", pageSizesList(), pageSize(), false);
            if(item.compare("Letter") == 0)
                setPageSize(ReportWindow::Letter);
            else if(item.compare("Legal") == 0)
                setPageSize(ReportWindow::Legal);
            else if(item.compare("A3") == 0)
                setPageSize(ReportWindow::A3);
            else if(item.compare("A4") == 0)
                setPageSize(ReportWindow::A4);
            else if(item.compare("A5") == 0)
                setPageSize(ReportWindow::A5);
            else if(item.compare("B4") == 0)
                setPageSize(ReportWindow::B4);
            else if(item.compare("B5") == 0)
                setPageSize(ReportWindow::B5);
            else if(item.compare("Executive") == 0)
                setPageSize(ReportWindow::Executive);
            else if(item.compare("US 4x6") == 0)
                setPageSize(ReportWindow::US4x6);
            else if(item.compare("US 4x8") == 0)
                setPageSize(ReportWindow::US4x8);
            else if(item.compare("US 5x7") == 0)
                setPageSize(ReportWindow::US5x7);
            else if(item.compare("Common 10") == 0)
                setPageSize(ReportWindow::Comm10);
            else
                setPageSize(ReportWindow::A4);
        }
    }
}

void ReportWindow::closeEvent(QCloseEvent *event)
{
    printingCancelled();
    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("reportwindow_maximized", true);
        showNormal();
    }
    else
        settings.setValue("reportwindow_maximized", false);
    settings.setValue("reportwindow_pos", pos());
    settings.setValue("reportwindow_size", size());
    settings.setValue("icon_size", toolbar->iconSize());
}
