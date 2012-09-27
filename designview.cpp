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

#include <QtGui>
#include "designview.h"
#include "checkboxdelegate.h"
#include "tableproperties.h"

ulong DesignView::designViewObjectId = 0;

class CompleterDelegate : public QStyledItemDelegate
{
public:
    CompleterDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        QStringList stringList;
        stringList << "bigint" << "bigserial" << "bit" << "bit varying"
                   << "boolean" << "box" << "bytea" << "character varying"
                   << "character" << "cidr" << "circle" << "date"
                   << "double precision" << "inet" << "integer" << "interval"
                   << "line" << "lseg" << "macaddr" << "money"
                   << "numeric" << "path" << "point" << "polygon"
                   << "real" << "smallint" << "serial" << "text" << "time"
                   << "time without time zone" << "timestamp"
                   << "timestamp without time zone" << "tsquery"
                   << "tsvector" << "txid_snapshot" << "uuid" << "xml";
        QCompleter* completer = new QCompleter(stringList, lineEdit);
        completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        completer->setCaseSensitivity(Qt::CaseInsensitive);

        lineEdit->setCompleter(completer);
        return lineEdit;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QStyledItemDelegate::setEditorData(editor, index);
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
};

DesignView::DesignView(Database *database, Table *table, QString const table_name, QString const name, QStringList column_list, QStringList primary_key, QStringList column_types, QStringList column_lengths, QStringList column_nulls, bool read_only, Qt::WidgetAttribute f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon(":/icons/design.png"));
    menuBar()->setVisible(false);
    createBrushes();
    createActions();

    error_status = false;
    error_message_box = new QMessageBox(this);
    error_message_box->setWindowModality(Qt::WindowModal);

    this->database = database;
    this->table = table;
    this->table_name = table_name;
    this->primary_key = primary_key;
    this->column_list = column_list;
    this->column_types = column_types;
    this->column_lengths = column_lengths;
    this->column_nulls = column_nulls;

    setWindowTitle(name);
    setObjectName(name);

    toolbar = new QToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("designview");
    toolbar->setMovable(false);
    toolbar->addAction(save_action);
    toolbar->addSeparator();
    toolbar->addAction(properties_action);
    toolbar->addSeparator();
    toolbar->addAction(insert_column_left_action);
    toolbar->addAction(delete_column_action);

    addToolBar(toolbar);
    statusBar()->show();

    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisDesignViewId = designViewObjectId++;

    //Thread busy indicator to avoid overlapping of threads.
    //Initialise to false because obviously we don't have TableView
    //GUI artifacts to create overlapping threads yet.
    thread_busy = false;

    setContextMenuPolicy(Qt::NoContextMenu);

    design_model = new QStandardItemModel(5, column_list.size()+1);
    initialiseModel();

    design_view = new QTableView(this);
    design_view->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    CheckBoxDelegate *check_delegate = new CheckBoxDelegate(design_view);
    design_view->setItemDelegateForRow(2, check_delegate);
    design_view->setItemDelegateForRow(3, check_delegate);
    CompleterDelegate *completer_delegate = new CompleterDelegate(design_view);
    design_view->setItemDelegateForRow(1, completer_delegate);
    design_view->viewport()->installEventFilter(this);
    design_view->installEventFilter(this);
    design_view->setAlternatingRowColors(true);
    design_view->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    design_view->setModel(design_model);
    connect(design_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateDesigner(QModelIndex,QModelIndex)));
    connect(design_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateSelectionChanged()));

    setCentralWidget(design_view);
}

void DesignView::initialiseModel()
{
    QModelIndex idx;
    for (int column = 0; column < column_list.size(); ++column) {
        QString column_name = column_list.at(column);
        QStandardItem *item = new QStandardItem(column_name.remove("\""));
             design_model->setItem(0, column, item);
             item = new QStandardItem(column_types.at(column));
             design_model->setItem(1, column, item);
             idx = design_model->index(2, column, QModelIndex());
             design_model->setData(idx, false);
             foreach(QString key_element, primary_key) {
                 int actual_index = column_list.indexOf(key_element);
                 if(actual_index > -1) {
                     QModelIndex idx = design_model->index(2, actual_index, QModelIndex());
                     design_model->setData(idx, true);
                 }
             }
             if(column_nulls.at(column).compare("true") == 0) {
                 idx = design_model->index(3, column, QModelIndex());
                 design_model->setData(idx, true);
             }
             else {
                 idx = design_model->index(3, column, QModelIndex());
                 design_model->setData(idx, false);
             }
    }
    QStandardItem *item = new QStandardItem(QString(""));
    design_model->setItem(0, column_list.size(), item);
    idx = design_model->index(2, column_list.size(), QModelIndex());
    design_model->setData(idx, false);
    idx = design_model->index(0, column_list.size(), QModelIndex());
    design_model->setData(idx, red_brush, Qt::BackgroundRole);
    idx = design_model->index(1, column_list.size(), QModelIndex());
    design_model->setData(idx, red_brush, Qt::BackgroundRole);

    design_model->setHeaderData(0, Qt::Vertical, tr("Name"));
    design_model->setHeaderData(1, Qt::Vertical, tr("Type"));
    design_model->setHeaderData(2, Qt::Vertical, tr("Primary key"));
    design_model->setHeaderData(3, Qt::Vertical, tr("Not null"));
    design_model->setHeaderData(4, Qt::Vertical, tr("Default value"));
}

void DesignView::updateDesigner(QModelIndex from, QModelIndex to)
{
    if(from.row()==0 || from.row()==1) {
        if(from.data().toString().isEmpty())
            design_model->setData(from, red_brush, Qt::BackgroundRole);
        else
            design_model->setData(from, QBrush(), Qt::BackgroundRole);
    }

    if(design_model->columnCount() > 1) {
        save_action->setEnabled(true);
    }

    QModelIndex idx = design_model->index(0, design_model->columnCount()-1, QModelIndex());
    QModelIndex idx2 = design_model->index(1, design_model->columnCount()-1, QModelIndex());
    if(!design_model->data(idx).toString().isEmpty() && !design_model->data(idx2).toString().isEmpty()) {
        QList<QStandardItem *> new_column;
        new_column << new QStandardItem("");
        design_model->appendColumn(new_column);
        new_column_list.append(design_model->data(idx).toString());
        new_column_types.append(design_model->data(idx2).toString());
        idx = design_model->index(0, design_model->columnCount()-1, QModelIndex());
        design_model->setData(idx, red_brush, Qt::BackgroundRole);
        idx = design_model->index(1, design_model->columnCount()-1, QModelIndex());
        design_model->setData(idx, red_brush, Qt::BackgroundRole);
        idx = design_model->index(0, design_model->columnCount()-2, QModelIndex());
        design_model->setData(idx, QBrush(), Qt::BackgroundRole);
        idx = design_model->index(1, design_model->columnCount()-2, QModelIndex());
        design_model->setData(idx, QBrush(), Qt::BackgroundRole);
    }

    if(from != to || from.row() != 2)
        return;
}

void DesignView::updateSelectionChanged()
{
    if(design_view->selectionModel()) {
        insert_column_left_action->setEnabled(true);

        QModelIndexList indices = design_view->selectionModel()->selectedColumns();
        qSort(indices);
        if(indices.isEmpty() || indices.last().column() == design_model->columnCount()-1)
            delete_column_action->setEnabled(false);
        else
            delete_column_action->setEnabled(true);
    }
    else {
        insert_column_left_action->setEnabled(false);
        delete_column_action->setEnabled(false);
    }
}

void DesignView::createBrushes()
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

void DesignView::createActions()
{
    save_action = new QAction(QIcon(":/icons/save.png"), tr("&Save"), this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip(tr("Save the table definition to database"));
    save_action->setEnabled(false);
    connect(save_action, SIGNAL(triggered()), this, SLOT(saveTable()));

    properties_action = new QAction(QIcon(":/icons/properties.png"), tr("&Properties"), this);
    properties_action->setStatusTip(tr("Specify table properties"));
    properties_action->setCheckable(true);
    connect(properties_action, SIGNAL(triggered()), this, SLOT(showTableProperties()));

    insert_column_left_action = new QAction(QIcon(":/icons/insertleft.png"), tr("Insert column left"), this);
    insert_column_left_action->setStatusTip(tr("Insert to the left of the selected column(s)"));
    insert_column_left_action->setEnabled(false);
    connect(insert_column_left_action, SIGNAL(triggered()), this, SLOT(insertLeftColumn()));

    delete_column_action = new QAction(QIcon(":/icons/removecolumn.png"), tr("&Delete column(s)"), this);
    delete_column_action->setStatusTip(tr("Delete selected column(s)"));
    delete_column_action->setEnabled(false);
    connect(delete_column_action, SIGNAL(triggered()), this, SLOT(deleteColumns()));
}

void DesignView::saveTable()
{
    design_view->setDisabled(true);
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("designview ").append(table_name));
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
        sql = QLatin1String("DROP TABLE IF EXISTS ");
        sql.append(table_name);
        sql.append(QLatin1String("; CREATE TABLE "));
        sql.append(table_name);
        sql.append(QLatin1String(" ("));
        new_primary_key.clear();
        int column_count = design_model->columnCount();
        for(int col = 0; col < column_count-1; col++) {
            QModelIndex idx0 = design_model->index(0, col, QModelIndex());
            QModelIndex idx1 = design_model->index(1, col, QModelIndex());
            QModelIndex idx2 = design_model->index(2, col, QModelIndex());
            QModelIndex idx3 = design_model->index(3, col, QModelIndex());
            QModelIndex idx4 = design_model->index(4, col, QModelIndex());
            sql.append("\"" + QString(idx0.data().toString()).remove("\"") + "\"");
            sql.append(QLatin1String(" "));
            sql.append(idx1.data().toString());
            if(idx3.data().toBool())
                sql.append(QLatin1String(" NOT NULL "));
            if(!idx4.data().toString().isEmpty())
                sql.append(QLatin1String(" DEFAULT ")).append(design_model->data(idx4).toString());
            sql.append(QLatin1String(", "));

            if(idx2.data().toBool())
                new_primary_key.append("\"" + QString(idx0.data().toString()).remove("\"") + "\"");
        }
        if(new_primary_key.isEmpty()) {
            sql.remove(sql.length()-2, 2);
        }
        else {
            sql.append("CONSTRAINT \"");
            sql.append(QString(table_name).remove("\""));
            sql.append("_pkey\" PRIMARY KEY");
            sql.append(new_primary_key.join(",").prepend("(").append(")"));
        }
        sql.append(QLatin1String(") "));
        sql.append(properties);

        QSqlQueryModel query;
        query.setQuery(sql, database_connection);
        if(query.lastError().isValid()) {
            QStringList messages = query.lastError().databaseText().split("\n");
            messages.removeLast();
            QMessageBox::critical(this, MainWin::tr("Database error"),
            messages.join("\n"), QMessageBox::Close);
        }
    }
    QSqlDatabase::removeDatabase(QString("designview ").append(table_name));
    design_view->setDisabled(false);
}

void DesignView::bringOnTop()
{
    activateWindow();
    raise();
}

void DesignView::showTableProperties()
{
    QStringList completer_list;
    foreach(Schema *schema, database->getSchemaList())
        foreach (Table *table, schema->getTableList())
            completer_list.append(schema->getName().append(".\"" + table->getName() + "\""));

    TableProperties *table_props = new TableProperties(this, table_name, completer_list, properties2.oid, properties2.inherits, properties2.tablespace, properties2.fill_factor);
    connect(this, SIGNAL(changeLanguage(QEvent*)), table_props, SLOT(languageChanged(QEvent*)));
    connect(table_props, SIGNAL(oksignal(bool, QString, QString, int)), this, SLOT(setProperties(bool, QString, QString, int)));
    connect(table_props, SIGNAL(accepted()), this, SLOT(popPropertiesButton()));
    connect(table_props, SIGNAL(rejected()), this, SLOT(popPropertiesButton()));
    table_props->show();
}

void DesignView::popPropertiesButton()
{
    properties_action->setChecked(false);
}

void DesignView::setProperties(bool oid, QString inherits, QString tablespace, int fill_factor)
{
    properties2.oid = oid;
    properties2.inherits = inherits;
    properties2.tablespace = tablespace;
    properties2.fill_factor = fill_factor;

    properties.clear();

    if(!inherits.isEmpty())
        properties.append(QLatin1String(" INHERITS (")).append(inherits).append(QLatin1String(")"));
    if(oid)
        properties.append(QLatin1String(" WITH (oids=true"));
    else
        properties.append(QLatin1String(" WITH (oids=false"));
    if(fill_factor != 100)
        properties.append(QLatin1String(", fillfactor=")).append(QString::number(fill_factor));
    properties.append(QLatin1String(")"));
    if(!tablespace.isEmpty())
        properties.append(QLatin1String(" TABLESPACE ")).append(tablespace);
}

void DesignView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        save_action->setText(tr("&Save"));
        save_action->setStatusTip(tr("Save the table definition to database"));
        properties_action->setText(tr("&Properties"));
        properties_action->setStatusTip(tr("Specify table properties"));
        insert_column_left_action->setText(tr("Insert column left"));
        insert_column_left_action->setStatusTip(tr("Insert to the left of the selected column(s)"));
        delete_column_action->setText(tr("&Delete column(s)"));
        delete_column_action->setStatusTip(tr("Delete selected column(s)"));
        design_model->setHeaderData(0, Qt::Vertical, tr("Name"));
        design_model->setHeaderData(1, Qt::Vertical, tr("Type"));
        design_model->setHeaderData(2, Qt::Vertical, tr("Primary key"));
        design_model->setHeaderData(3, Qt::Vertical, tr("Not null"));
        design_model->setHeaderData(4, Qt::Vertical, tr("Default value"));
        design_model->setHeaderData(5, Qt::Vertical, tr("Comment"));

        emit changeLanguage(event);
    }
}

void DesignView::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit designViewClosing(this);

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("designview_maximized", true);
        showNormal();
    }
    else
        settings.setValue("designview_maximized", false);
    settings.setValue("designview_pos", pos());
    settings.setValue("designview_size", size());

    delete toolbar;
    delete design_view;
    delete design_model;
    QMainWindow::closeEvent(event);
}

void DesignView::insertLeftColumn()
{
    QModelIndexList indices = design_view->selectionModel()->selectedIndexes();
    qSort(indices);
    if(!indices.isEmpty())
        design_model->insertColumn(indices.first().column());
    QModelIndex idx = design_model->index(0, indices.first().column(), QModelIndex());
    design_model->setData(idx, red_brush, Qt::BackgroundRole);
    idx = design_model->index(1, indices.first().column(), QModelIndex());
    design_model->setData(idx, red_brush, Qt::BackgroundRole);
}

void DesignView::deleteColumns()
{
    QModelIndexList indices;

    while(indices = design_view->selectionModel()->selectedColumns(), !indices.isEmpty()) {
        design_model->removeColumn(indices.at(0).column());
    }

    if(design_model->columnCount() == 1)
        save_action->setEnabled(false);

    insert_column_left_action->setEnabled(false);
}
