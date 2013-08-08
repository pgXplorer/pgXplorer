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

#include "pgxeditor.h"

ulong PgxEditor::editor_widow_id = 0;

PgxEditor::PgxEditor(Database *database, QString editor_name, QString function_args)
{
    ++editor_widow_id;
    this->database = database;
    this->editor_name = editor_name;
    this->function_args = function_args;

    foreach(Schema *schema, database->getSchemaList()) {
        foreach (Table *table, schema->getTableList())
            completer_list.append(schema->getName().append(".\"" + table->getName() + "\""));
        foreach (View *view, schema->getViewList())
            completer_list.append(schema->getName().append(".\"" + view->getName() + "\""));
        foreach (Function *function, schema->getFunctionList()) {
            if(function->getArgs().isEmpty())
                completer_list.append(schema->getName().append(".\"" + function->getName() + "\"()"));
            else
                completer_list.append(schema->getName().append(".\"" + function->getName() + "\"(" + function->getArgsToText() + ")"));
        }
    }
    completer_list.append(database->keywords());

    next_icon = QIcon(":/icons/next.png");
    createActions();
    breakpointArea = new BreakPointArea(this);
    lineNumberArea = new LineNumberArea(this);
    font_size = 14;
    style_sheet = QString("QPlainTextEdit{background-color: white; \
                          font: bold %1px 'Courier New';}").arg(QString::number(font_size));
    setStyleSheet(style_sheet);
    lineNumberArea->setStyleSheet(style_sheet);
    highlighter = new Highlighter(document());

    if(editor_name.isEmpty())
        setAcceptDrops(true);

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("pgxeditor");
    toolbar->setMovable(false);
    toolbar->addAction(newpgxeditor_action);
    toolbar->addSeparator();
    toolbar->addAction(cut_action);
    toolbar->addAction(copy_action);
    toolbar->addAction(paste_action);
    toolbar->addSeparator();
    toolbar->addAction(save_action);
    toolbar->addAction(execute_action);
    toolbar->addAction(selected_execute_action);
    toolbar->addAction(explain_action);
    toolbar->addAction(wrap_action);
    toolbar->addAction(find_action);
    toolbar->addAction(complete_action);

    pgxeditor_mainwin = new PgxEditorMainWindow(this);
    pgxeditor_mainwin->addToolBar(toolbar);
    pgxeditor_mainwin->setCentralWidget(this);
    pgxeditor_mainwin->setAttribute(Qt::WA_DeleteOnClose);

    find_bar = new QLineEdit;
    find_bar->setMaximumSize(100,find_bar->height());
    find_bar->setPlaceholderText(tr("Find text"));
    find_bar->setVisible(false);
    pgxeditor_mainwin->statusBar()->setSizeGripEnabled(false);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(casesensitivity_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(wholeword_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(find_bar);
    replace_bar = new QLineEdit;
    replace_bar->setMaximumSize(100,find_bar->height());
    replace_bar->setPlaceholderText(tr("Replace text"));
    replace_bar->setVisible(false);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(replace_bar);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(find_previous_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(find_next_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(replace_previous_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(replace_next_button, 0);

    connect(find_bar, SIGNAL(returnPressed()), SLOT(findNext()));
    connect(replace_bar, SIGNAL(returnPressed()), SLOT(replaceNext()));
    connect(this, SIGNAL(blockCountChanged(int)), SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(selectionChanged()), SLOT(selectionChangedSlot()));
    connect(this, SIGNAL(textChanged()), SLOT(textChangedSlot()));
    connect(this, SIGNAL(cursorPositionChanged()), SLOT(makeFirstBlockReadonly()));
    connect(pgxeditor_mainwin, SIGNAL(pgxeditorClosing()), SLOT(pgxeditorClosing()));
    connect(pgxeditor_mainwin, SIGNAL(changeIcons(bool)), SLOT(changeIcons(bool)));
    updateLineNumberAreaWidth(0);

    QShortcut *shortcut_default_view = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this);
    connect(shortcut_default_view, SIGNAL(activated()), SLOT(noZoom()));

    installEventFilter(this);
    pgxeditor_mainwin->installEventFilter(pgxeditor_mainwin);
}

void PgxEditor::hightlightFirstBlock()
{
    if(!editor_name.isEmpty()) {
        QTextCursor text_cursor = textCursor();
        text_cursor.movePosition(QTextCursor::Start);

        QList<QTextEdit::ExtraSelection> extraSelections;

        QTextEdit::ExtraSelection selection;
        QColor line_colour = QColor(220,220,230);
        selection.format.setBackground(line_colour);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = text_cursor;
        selection.cursor.clearSelection();
        extraSelections.append(selection);

        setExtraSelections(extraSelections);
    }
}

int PgxEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10, max /= 10)
        ++digits;
    int space = (font_size-2) * digits;
    return space;
}

void PgxEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(fontMetrics().width(QLatin1Char('9'))+lineNumberAreaWidth(), 0, 0, 0);
}

void PgxEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
     if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void PgxEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(!editor_name.isEmpty()) {
        QTextCursor text_cursor = textCursor();
        text_cursor.movePosition(QTextCursor::Start);

        QTextEdit::ExtraSelection selection;
        QColor line_colour = QColor(220,220,230);
        selection.format.setBackground(line_colour);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = text_cursor;
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    QTextEdit::ExtraSelection border;
    QPen outline(Qt::darkGreen, 2, Qt::SolidLine);
    //outline.setJoinStyle(Qt::RoundJoin);
    //outline.setCapStyle(Qt::RoundCap);
    border.format.setProperty(QTextFormat::OutlinePen, outline);
    border.cursor = textCursor();
    extraSelections.append(border);

    QTextEdit::ExtraSelection back_brush;
    QBrush brush(Qt::yellow);
    back_brush.format.setProperty(QTextFormat::BackgroundBrush, brush);
    back_brush.cursor = textCursor();
    extraSelections.append(back_brush);

    setExtraSelections(extraSelections);
}

void PgxEditor::removeHighlighting()
{
    QList<QTextEdit::ExtraSelection> extraSelections; 
    setExtraSelections(extraSelections);

    hightlightFirstBlock();
}

void PgxEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    breakpointArea->setGeometry(QRect(cr.left(), cr.top(), fontMetrics().width(QLatin1Char('9')), cr.height()));
    lineNumberArea->setGeometry(QRect(cr.left()+fontMetrics().width(QLatin1Char('9')), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void PgxEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(230,230,230,128));
    painter.setFont(QFont("Courier new", font_size-2));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(-1, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void PgxEditor::breakpointAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(breakpointArea);
    painter.fillRect(event->rect(), QColor(230,230,230,128));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(-1, top+2, breakpointArea->width(), fontMetrics().height(),
                             Qt::AlignRight, "");
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void PgxEditor::createActions()
{
    newpgxeditor_action = new QAction(QIcon(":/icons/editor.png"), tr("New"), this);
    newpgxeditor_action->setShortcuts(QKeySequence::New);
    newpgxeditor_action->setStatusTip(tr("New editor"));
    connect(newpgxeditor_action, SIGNAL(triggered()), this, SIGNAL(newPgxeditor()));

    cut_action = new QAction(QIcon(":/icons/cut.png"), tr("Cut"), this);
    cut_action->setShortcuts(QKeySequence::Cut);
    cut_action->setStatusTip(tr("Cut selected text and copy to clipboard"));
    connect(cut_action, SIGNAL(triggered()), SLOT(cut()));

    copy_action = new QAction(QIcon(":/icons/copy.png"), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected text to clipboard"));
    connect(copy_action, SIGNAL(triggered()), SLOT(copy()));

    paste_action = new QAction(QIcon(":/icons/paste.png"), tr("Paste"), this);
    paste_action->setShortcuts(QKeySequence::Paste);
    paste_action->setStatusTip(tr("Paste text from clipboard"));
    connect(paste_action, SIGNAL(triggered()), SLOT(paste()));

    save_action = new QAction(QIcon(":/icons/save.png"), tr("&Save"), this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip(tr("Save function"));
    connect(save_action, SIGNAL(triggered()), SLOT(saveFunction()));

    execute_action = new QAction(QIcon(":/icons/execute.png"), tr("&Execute"), this);
    execute_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    execute_action->setStatusTip(tr("Execute function"));
    connect(execute_action, SIGNAL(triggered()), SLOT(executeFunction()));

    explain_action = new QAction(QIcon(":/icons/explain.png"), tr("&Explain"), this);
    explain_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    explain_action->setStatusTip(tr("Explain (selected) text"));
    connect(explain_action, SIGNAL(triggered()), SLOT(explainText()));

    if(editor_name.isEmpty()) {
        execute_action->setVisible(false);
        save_action->setVisible(false);
        explain_action->setVisible(true);
    }
    else {
        execute_action->setEnabled(false);
        save_action->setEnabled(false);
        explain_action->setVisible(false);
    }

    selected_execute_action = new QAction(QIcon(":/icons/selected_execute.png"), tr("&Run"), this);
    selected_execute_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    selected_execute_action->setStatusTip(tr("Execute (selected) text"));
    if(!editor_name.isEmpty())
        selected_execute_action->setEnabled(false);
    connect(selected_execute_action, SIGNAL(triggered()), SLOT(executeText()));

    wrap_action = new QAction(QIcon(":/icons/wrap.png"), tr("Wrap/Un-wrap lines"),this);
    wrap_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
    wrap_action->setStatusTip(tr("Toggle line wrapping"));
    wrap_action->setCheckable(true);
    wrap_action->setChecked(true);
    connect(wrap_action, SIGNAL(triggered()), SLOT(toggleWrap()));

    find_action = new QAction(QIcon(":/icons/find.png"), tr("Find"), this);
    find_action->setShortcuts(QKeySequence::Find);
    find_action->setCheckable(true);
    find_action->setStatusTip(tr("Find/replace text"));
    connect(find_action, SIGNAL(triggered()), SLOT(toggleFindBar()));

    complete_action = new QAction(QIcon(":/icons/completer.png"), tr("Complete"), this);
    complete_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    complete_action->setCheckable(true);
    complete_action->setChecked(true);
    complete_action->setStatusTip(tr("Enable completion popup"));

    casesensitivity_action = new QAction(tr("Cs"), this);
    casesensitivity_action->setToolTip(tr("Case sensitive"));
    casesensitivity_action->setCheckable(true);
    casesensitivity_button = new QToolButton;
    casesensitivity_button->setDefaultAction(casesensitivity_action);
    casesensitivity_button->setVisible(false);

    wholeword_action = new QAction(tr("W"), this);
    wholeword_action->setToolTip(tr("Whole word"));
    wholeword_action->setCheckable(true);
    wholeword_button = new QToolButton;
    wholeword_button->setDefaultAction(wholeword_action);
    wholeword_button->setVisible(false);

    //backwards_action = new QAction(tr("B"), this);
    //backwards_action->setToolTip(tr("Backwards"));
    //backwards_action->setCheckable(true);
    //backwards_button = new QToolButton;
    //backwards_button->setDefaultAction(backwards_action);
    //backwards_button->setVisible(false);

    find_previous_action = new QAction(QIcon(":/icons/find_previous.png"), "", this);
    find_previous_action->setToolTip(tr("Find previous"));
    connect(find_previous_action, SIGNAL(triggered()), SLOT(findPrevious()));
    find_previous_button = new QToolButton;
    find_previous_button->setAutoRaise(true);
    find_previous_button->setDefaultAction(find_previous_action);
    find_previous_button->setVisible(false);

    find_next_action = new QAction(QIcon(":/icons/find_next.png"), "", this);
    find_next_action->setToolTip(tr("Find next"));
    connect(find_next_action, SIGNAL(triggered()), SLOT(findNext()));
    find_next_button = new QToolButton;
    find_next_button->setAutoRaise(true);
    find_next_button->setDefaultAction(find_next_action);
    find_next_button->setVisible(false);

    replace_previous_action = new QAction(QIcon(":/icons/replace_previous.png"), "", this);
    replace_previous_action->setToolTip(tr("Replace previous"));
    connect(replace_previous_action, SIGNAL(triggered()), SLOT(replacePrevious()));
    replace_previous_button = new QToolButton;
    replace_previous_button->setAutoRaise(true);
    replace_previous_button->setDefaultAction(replace_previous_action);
    replace_previous_button->setVisible(false);

    replace_next_action = new QAction(QIcon(":/icons/replace_next.png"), "", this);
    replace_next_action->setToolTip(tr("Replace next"));
    connect(replace_next_action, SIGNAL(triggered()), SLOT(replaceNext()));
    replace_next_button = new QToolButton;
    replace_next_button->setAutoRaise(true);
    replace_next_button->setDefaultAction(replace_next_action);
    replace_next_button->setVisible(false);
}

void PgxEditor::saveFunction()
{
    bool is_saved = false;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("save function ").append(editor_name).append(QString::number(editor_widow_id)));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                   "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery temp_query(database_connection);
        is_saved = temp_query.exec(toPlainText().replace(QChar::ParagraphSeparator,"\n"));
        if(temp_query.lastError().isValid()) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("Database error"),
                temp_query.lastError().text(), QMessageBox::Cancel);
        }
    }
    QSqlDatabase::removeDatabase(QString("save function ").append(editor_name).append(QString::number(editor_widow_id)));
    QApplication::restoreOverrideCursor();
    if(is_saved)
        save_action->setEnabled(false);
    else
        save_action->setEnabled(true);
}

void PgxEditor::executeText()
{
    if(toPlainText().simplified().isEmpty())
        return;
    if(textCursor().selectedText().isEmpty())
            emit showQueryView(database, toPlainText().replace(QChar::ParagraphSeparator,"\n"));
    else
        emit showQueryView(database, textCursor().selectedText().replace(QChar::ParagraphSeparator,"\n"));
}

void PgxEditor::executeFunction()
{
    QString query("SELECT ");
    query.append(editor_name);
    query.append(QLatin1Char('('));
    query.append(function_args.split(" ").join(","));
    query.append(QLatin1Char(')'));
    emit newPgxeditorQuery(query);
}

void PgxEditor::explainText()
{
    QString sql;

    if(toPlainText().simplified().isEmpty())
        return;
    if(textCursor().selectedText().isEmpty())
        sql = QLatin1String("BEGIN; EXPLAIN ANALYZE VERBOSE ") + toPlainText().replace(QChar::ParagraphSeparator,"\n");
    else
        sql = QLatin1String("BEGIN; EXPLAIN ANALYZE VERBOSE ") + textCursor().selectedText().replace(QChar::ParagraphSeparator,"\n");

    emit showQueryView(database, sql);
}

void PgxEditor::selectionChangedSlot()
{
    if(!editor_name.isEmpty()) {
        if(textCursor().selectedText().isEmpty())
            selected_execute_action->setEnabled(false);
        else
            selected_execute_action->setEnabled(true);
    }
}

void PgxEditor::findPrevious()
{
    QTextDocument::FindFlags find_flags;
    find_flags |= QTextDocument::FindBackward;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxeditor_mainwin->statusBar()->clearMessage();
    if(find_cursor.atStart()) {
        find_cursor.movePosition(QTextCursor::End);
        setTextCursor(find_cursor);
    }

    if(find(find_bar->text(), find_flags)) {
        find_cursor = textCursor();
        find_cursor.movePosition(QTextCursor::StartOfWord);
        highlightCurrentLine();
    }
    else {
        pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
        find_cursor.movePosition(QTextCursor::Start);
    }
}

void PgxEditor::findNext()
{
    QTextDocument::FindFlags find_flags;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxeditor_mainwin->statusBar()->clearMessage();
    if(find_cursor.atEnd()) {
        find_cursor.movePosition(QTextCursor::Start);
        setTextCursor(find_cursor);
    }

    if(find(find_bar->text(), find_flags)) {
        find_cursor = textCursor();
        highlightCurrentLine();
    }
    else {
        pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
        find_cursor.movePosition(QTextCursor::End);
    }
}
/*
void PgxEditor::replaceText()
{
    QTextDocument::FindFlags find_flags;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;
    if(backwards_action->isChecked())
        find_flags |= QTextDocument::FindBackward;
    pgxeditor_mainwin->statusBar()->clearMessage();
    if(find_cursor.atEnd() && !backwards_action->isChecked()) {
        find_cursor.movePosition(QTextCursor::Start);
        setTextCursor(find_cursor);
    }
    if(find_cursor.atStart() && backwards_action->isChecked()) {
        find_cursor.movePosition(QTextCursor::End);
        setTextCursor(find_cursor);
    }

    if(!find(find_bar->text(), find_flags)) {
        if(backwards_action->isChecked()) {
            pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
            find_cursor.movePosition(QTextCursor::Start);
        }
        else {
            pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
            find_cursor.movePosition(QTextCursor::End);
        }
    }
    else {
        QTextCursor before_replace = textCursor();
        before_replace.movePosition(QTextCursor::WordLeft);
        textCursor().insertText(replace_bar->text());
        if(backwards_action->isChecked()) {
            setTextCursor(before_replace);
            find_cursor = before_replace;
        }
        highlightCurrentLine();
    }
}*/

void PgxEditor::replacePrevious()
{
    QTextDocument::FindFlags find_flags;
    find_flags |= QTextDocument::FindBackward;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxeditor_mainwin->statusBar()->clearMessage();
    if(find_cursor.atStart()) {
        find_cursor.movePosition(QTextCursor::End);
        setTextCursor(find_cursor);
    }
    QTextCursor before_replace = textCursor();
    before_replace.setPosition(textCursor().selectionEnd());
    setTextCursor(before_replace);
    if(find(find_bar->text(), find_flags)) {
        before_replace = textCursor();
        before_replace.movePosition(QTextCursor::WordLeft);
        textCursor().insertText(replace_bar->text());
        setTextCursor(before_replace);
        find_cursor = before_replace;
        highlightCurrentLine();
    }
    else {
        pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
        find_cursor.movePosition(QTextCursor::Start);
    }
}

void PgxEditor::replaceNext()
{
    QTextDocument::FindFlags find_flags;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxeditor_mainwin->statusBar()->clearMessage();
    if(find_cursor.atEnd()) {
        find_cursor.movePosition(QTextCursor::Start);
        setTextCursor(find_cursor);
    }
    QTextCursor before_replace = textCursor();
    before_replace.setPosition(textCursor().selectionStart());
    setTextCursor(before_replace);
    if(find(find_bar->text(), find_flags)) {
        before_replace = textCursor();
        textCursor().insertText(replace_bar->text());
        setTextCursor(before_replace);
        find_cursor = before_replace;
        highlightCurrentLine();
    }
    else {
        pgxeditor_mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
        find_cursor.movePosition(QTextCursor::End);
    }
}

/*bool PgxEditor::eventFilter(QObject *object, QEvent *event)
{
    if(!find_action->isChecked())
        return QPlainTextEdit::eventFilter(object, event);

    if (object == this) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->type() == QEvent::KeyPress && key_event->key() == Qt::Key_Control) {
            //changeIcons(false);
        }
        else if (key_event->type() == QEvent::KeyRelease && key_event->key() == Qt::Key_Control) {
            //changeIcons(true);
        }
        return QPlainTextEdit::eventFilter(object, event);
    }
    else
        return QPlainTextEdit::eventFilter(object, event);
}*/

void PgxEditor::dragEnterEvent(QDragEnterEvent *event)
{
    if(editor_name.isEmpty())
        event->acceptProposedAction();
    else
        QPlainTextEdit::dragEnterEvent(event);
}

void PgxEditor::dragMoveEvent(QDragMoveEvent *event)
{
    if(editor_name.isEmpty())
        event->acceptProposedAction();
    else
        QPlainTextEdit::dragMoveEvent(event);
}

void PgxEditor::dropEvent(QDropEvent *event)
{
    if(editor_name.isEmpty())
        event->acceptProposedAction();
    if(event->mimeData()->hasUrls()) {
        foreach(QUrl f, event->mimeData()->urls()) {
            QFile file(f.toLocalFile());
            if(file.size() > 1048576) {
                pgxeditor_mainwin->statusBar()->showMessage(tr("Skipping file %1 "
                "because it is greater then 1MB").arg(file.fileName()));
                continue;
            }
            if(!file.open(QIODevice::ReadOnly))
                QMessageBox::information(this, "error", file.errorString());
            QTextStream in(&file);
            while(!in.atEnd()) {
                appendPlainText(in.readLine());
            }
            file.close();
        }
    }
    else {
        QPlainTextEdit::dropEvent(event);
    }
}

/*bool PgxEditorMainWindow::eventFilter(QObject *object, QEvent *event)
{
    if(!pgxeditor->findActionStatus())
        return QMainWindow::eventFilter(object, event);

    if (object == this) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->type() == QEvent::KeyPress && key_event->key() == Qt::Key_Control) {
            //changeIcons(false);
        }
        else if (key_event->type() == QEvent::KeyRelease && key_event->key() == Qt::Key_Control) {
            //changeIcons(true);
        }
        return QMainWindow::eventFilter(object, event);
    }
    else
        return QMainWindow::eventFilter(object, event);
}*/

void PgxEditor::toggleWrap()
{
    if(lineWrapMode() == QPlainTextEdit::WidgetWidth)
        setLineWrapMode(QPlainTextEdit::NoWrap);
    else
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
}

void PgxEditor::toggleFindBar()
{
    if(!find_action->isChecked()) {
        if(find_bar->isActiveWindow()) {
            pgxeditor_mainwin->statusBar()->clearMessage();
            removeHighlighting();
            casesensitivity_button->setVisible(false);
            wholeword_button->setVisible(false);
            find_bar->setVisible(false);
            replace_bar->setVisible(false);
            find_next_button->setVisible(false);
            find_previous_button->setVisible(false);
            replace_next_button->setVisible(false);
            replace_previous_button->setVisible(false);
        }
        else
            find_bar->setFocus();
    }
    else {
        find_cursor = textCursor();
        setTextCursor(find_cursor);
        find_bar->setFocus();
        casesensitivity_button->setVisible(true);
        wholeword_button->setVisible(true);
        find_bar->setVisible(true);
        replace_bar->setVisible(true);
        find_next_button->setVisible(true);
        find_previous_button->setVisible(true);
        replace_next_button->setVisible(true);
        replace_previous_button->setVisible(true);
    }
}

void PgxEditor::textChangedSlot()
{
    if(editor_name.isEmpty()) {
        execute_action->setVisible(false);
        save_action->setVisible(false);
    }
    else {
        execute_action->setVisible(true);
        save_action->setVisible(true);
        execute_action->setEnabled(true);
        save_action->setEnabled(true);
    }
}

void PgxEditor::makeFirstBlockReadonly()
{
    matchParentheses();
    if(!editor_name.isEmpty()) {
        QTextCursor selection_start_cursor = textCursor();
        selection_start_cursor.setPosition(textCursor().selectionStart());
        if(selection_start_cursor.block() == document()->firstBlock())
            setReadOnly(true);
        else
            setReadOnly(false);
    }
}

void PgxEditor::matchParentheses()
{
    QList<QTextEdit::ExtraSelection> selections;
    setExtraSelections(selections);

    hightlightFirstBlock();

    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data) {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i) {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == QChar('(')) {
                if (matchLeftParenthesis(textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
                else
                    warnParenthesisSelection(pos + info->position);
            } else if (info->position == curPos - 1 && info->character == QChar(')')) {
                if (matchRightParenthesis(textCursor().block(), i - 1, 0, true))
                    createParenthesisSelection(pos + info->position);
                else
                    warnParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool PgxEditor::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i) {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == QChar('(')) {
            ++numLeftParentheses;
            continue;
        }
        if (info->character == QChar(')') && numLeftParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numLeftParentheses;
    }
    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(currentBlock, 0, numLeftParentheses);
    return false;
}

bool PgxEditor::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses, bool firstTime)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();
    int info_size = parentheses.size();

    if(!firstTime)
        i = info_size - 1;

    int docPos = currentBlock.position();
    for (; i > -1 && info_size > 0; i--) {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == QChar(')')) {
            ++numRightParentheses;
            continue;
        }
        if (info->character == QChar('(') && numRightParentheses == 0) {
            createParenthesisSelection(info->position + docPos);
            return true;
        } else
            --numRightParentheses;
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(currentBlock, 0, numRightParentheses);
    return false;
}

void PgxEditor::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    if(!editor_name.isEmpty()) {
        QTextCursor text_cursor = textCursor();
        text_cursor.movePosition(QTextCursor::Start);

        QTextEdit::ExtraSelection selection;
        QColor line_colour = QColor(220,220,230);
        selection.format.setBackground(line_colour);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = text_cursor;
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(QColor(Qt::green).lighter(180));
    format.setForeground(QColor(Qt::darkGreen).darker());
    QPen outline(Qt::darkGreen, 0.1, Qt::DotLine);
    outline.setJoinStyle(Qt::RoundJoin);
    format.setProperty(QTextFormat::OutlinePen, outline);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);
    setExtraSelections(selections);
}

void PgxEditor::warnParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    if(!editor_name.isEmpty()) {
        QTextCursor text_cursor = textCursor();
        text_cursor.movePosition(QTextCursor::Start);

        QTextEdit::ExtraSelection selection;
        QColor line_colour = QColor(220,220,230);
        selection.format.setBackground(line_colour);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = text_cursor;
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(QColor(Qt::red).lighter(180));
    format.setForeground(QColor(Qt::darkRed).darker());
    QPen outline(Qt::darkGreen, 0.1, Qt::DotLine);
    outline.setJoinStyle(Qt::RoundJoin);
    format.setProperty(QTextFormat::OutlinePen, outline);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);
    setExtraSelections(selections);
}

void PgxEditor::setTitle(QString title)
{
    pgxeditor_mainwin->setWindowTitle(title);
}

void PgxEditor::setText(QString editor_text, bool save_state)
{
    setPlainText(editor_text);
    if(!editor_name.isEmpty())
        save_action->setEnabled(save_state);
}

void PgxEditor::setResizePos(QSize size, QPoint pos, QSize icon_size)
{
    pgxeditor_mainwin->resize(size);
    pgxeditor_mainwin->move(pos);
    toolbar->setIconSize(icon_size);
    pgxeditor_mainwin->show();
}

/*bool PgxEditor::findActionStatus()
{
    return find_action->isChecked();
}*/

void PgxEditor::pgxeditorClosing()
{
    emit pgxeditorClosing(this);
}

void PgxEditor::noZoom()
{
    font_size = 14;
    style_sheet = QString("QPlainTextEdit{background-color: white; \
                          font: bold %1px 'Courier New';}").arg(QString::number(font_size));
    setStyleSheet(style_sheet);
}

void PgxEditor::closeMain()
{
    pgxeditor_mainwin->close();
}

void PgxEditor::changeIcons(bool find_status)
{
    if(find_status) {
        find_previous_button->setVisible(true);
        find_next_button->setVisible(true);
        replace_previous_button->setVisible(false);
        replace_next_button->setVisible(false);
    }
    else {
        find_previous_button->setVisible(false);
        find_next_button->setVisible(false);
        replace_previous_button->setVisible(true);
        replace_next_button->setVisible(true);
    }
}

void PgxEditor::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        newpgxeditor_action->setText(tr("New"));
        newpgxeditor_action->setStatusTip(tr("New editor"));

        cut_action->setText(tr("Cut"));
        cut_action->setStatusTip(tr("Cut selected text and copy to clipboard"));

        copy_action->setText(tr("Copy"));
        copy_action->setStatusTip(tr("Copy selected text to clipboard"));

        paste_action->setText(tr("Paste"));
        paste_action->setStatusTip(tr("Paste text from clipboard"));

        if(!editor_name.isEmpty()) {
            save_action->setText(tr("&Save"));
            save_action->setStatusTip(tr("Save function"));

            execute_action->setText(tr("&Execute"));
            execute_action->setStatusTip(tr("Execute function"));
        }

        selected_execute_action->setText(tr("&Run"));
        selected_execute_action->setStatusTip(tr("Execute (selected) text"));

        wrap_action->setText(tr("Wrap/Un-wrap lines"));
        wrap_action->setStatusTip(tr("Toggle line wrapping"));

        find_action->setText(tr("Find"));
        find_action->setStatusTip(tr("Find/replace text"));
        find_bar->setPlaceholderText(tr("Find text"));
        replace_bar->setPlaceholderText(tr("Replace text"));

        casesensitivity_action->setToolTip(tr("Case sensitive"));
        wholeword_action->setToolTip(tr("Whole word"));
    }
}

void PgxEditor::wheelEvent(QWheelEvent *wheelEvent)
{
    if(wheelEvent->modifiers() == Qt::ControlModifier) {
        if (wheelEvent->delta()>0)
            font_size < 36 ? font_size++ : font_size;
        else
            font_size > 8 ? font_size-- : font_size;

        style_sheet = QString("QPlainTextEdit{background-color: white; "
                              "font: bold %1px 'Courier New';}").arg(QString::number(font_size));
        setStyleSheet(style_sheet);
    }
    else
        QPlainTextEdit::wheelEvent(wheelEvent);
}

void PgxEditor::setCompleter(QCompleter *completer)
{
    //if (c)
    //    QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->popup()->setMaximumWidth(200);
    c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(c, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *PgxEditor::completer() const
{
    return c;
}

void PgxEditor::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString PgxEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    int pos = tc.position();
    int block_pos = tc.block().position();
    while(pos > block_pos && pos--) {
        tc.setPosition(pos, QTextCursor::KeepAnchor);
        if(tc.selectedText().contains(QRegExp("\\s"))) {
            tc.setPosition(pos+1, QTextCursor::KeepAnchor);
            break;
        }
    }
    return tc.selectedText();
}

void PgxEditor::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void PgxEditor::keyPressEvent(QKeyEvent *e)
{
    if (!complete_action->isChecked()) {// do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);
        return;
    }
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_L); // CTRL+L
    if (c || !complete_action->isChecked()) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2)) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    int width = c->popup()->sizeHintForColumn(0)>200 ? c->popup()->maximumWidth() : c->popup()->sizeHintForColumn(0);
    cr.setWidth(width + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

void PgxEditor::appendCompleterList(QString word)
{
    completer_list.append(word);
}

QStringList PgxEditor::completerList()
{
    return completer_list;
}

void PgxEditorMainWindow::closeEvent(QCloseEvent *event)
{
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //TableView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    emit pgxeditorClosing();

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("pgxeditor_maximized", true);
        showNormal();
    }
    else
        settings.setValue("pgxeditor_maximized", false);
    settings.setValue("pgxeditor_pos", pos());
    settings.setValue("pgxeditor_size", size());
    //settings.setValue("icon_size", toolbar->iconSize());

    QMainWindow::closeEvent(event);
}

void PgxEditorMainWindow::bringOnTop()
{
    activateWindow();
    raise();
}
