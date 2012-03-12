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

#include "pgxeditor.h"

ulong PgxEditor::editor_widow_id = 0;

PgxEditor::PgxEditor(Database *database, QString editor_name)
{
    ++editor_widow_id;
    this->database = database;
    this->editor_name = editor_name;
    createActions();
    breakpointArea = new BreakPointArea(this);
    lineNumberArea = new LineNumberArea(this);
    font_size = 14;
    style_sheet = QString("QPlainTextEdit{background-color: white; \
                          font: bold %1px 'Courier New';}").arg(QString::number(font_size));
    setStyleSheet(style_sheet);
    lineNumberArea->setStyleSheet(style_sheet);
    highlighter = new Highlighter(document());

    toolbar = new QToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("pgxeditor");
    toolbar->setMovable(false);
    toolbar->addAction(newpgxeditor_action);
    toolbar->addAction(cut_action);
    toolbar->addAction(copy_action);
    toolbar->addAction(paste_action);
    toolbar->addSeparator();
    toolbar->addAction(save_action);
    toolbar->addSeparator();
    toolbar->addAction(execute_action);
    toolbar->addSeparator();
    toolbar->addAction(selected_execute_action);
    toolbar->addAction(wrap_action);
    toolbar->addAction(find_action);

    pgxeditor_mainwin = new PgxEditorMainWindow;
    pgxeditor_mainwin->addToolBar(toolbar);
    pgxeditor_mainwin->setCentralWidget(this);
    pgxeditor_mainwin->setAttribute(Qt::WA_DeleteOnClose);

    find_bar = new QLineEdit;
    find_bar->setPlaceholderText(tr("Find"));
    find_bar->setVisible(false);
    pgxeditor_mainwin->statusBar()->setSizeGripEnabled(false);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(casesensitivity_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(wholeword_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(backwards_button, 0);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(find_bar);
    replace_bar = new QLineEdit;
    replace_bar->setPlaceholderText(tr("Replace"));
    replace_bar->setVisible(false);
    pgxeditor_mainwin->statusBar()->addPermanentWidget(replace_bar);

    connect(find_bar, SIGNAL(returnPressed()), this, SLOT(findText()));
    connect(replace_bar, SIGNAL(returnPressed()), this, SLOT(replaceText()));
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChangedSlot()));
    connect(this, SIGNAL(textChanged()), this, SLOT(textChangedSlot()));
    connect(this, SIGNAL(cursorPositionChanged()), SLOT(makeFirstBlocksReadonly()));
    connect(pgxeditor_mainwin, SIGNAL(pgxeditorClosing()), this, SLOT(pgxeditorClosing()));
    updateLineNumberAreaWidth(0);

    QShortcut *shortcut_default_view = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this);
    connect(shortcut_default_view, SIGNAL(activated()), this, SLOT(noZoom()));
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

    /*QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);*/

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
    newpgxeditor_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/editor.png")), tr("New"), this);
    newpgxeditor_action->setShortcuts(QKeySequence::New);
    newpgxeditor_action->setStatusTip(tr("New editor"));
    connect(newpgxeditor_action, SIGNAL(triggered()), this, SIGNAL(newPgxeditor()));

    cut_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/cut.png")), tr("Cut"), this);
    cut_action->setShortcuts(QKeySequence::Cut);
    cut_action->setStatusTip(tr("Cut selected text and copy to clipboard"));
    connect(cut_action, SIGNAL(triggered()), this, SLOT(cut()));

    copy_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy.png")), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected text to clipboard"));
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copy()));

    paste_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/paste.png")), tr("Paste"), this);
    paste_action->setShortcuts(QKeySequence::Paste);
    paste_action->setStatusTip(tr("Paste text from clipboard"));
    connect(paste_action, SIGNAL(triggered()), this, SLOT(paste()));

    save_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/save.png")), tr("&Save"), this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip(tr("Save function"));
    connect(save_action, SIGNAL(triggered()), this, SLOT(saveFunction()));

    execute_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/execute.png")), tr("&Execute"), this);
    execute_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    execute_action->setStatusTip(tr("Execute function"));
    connect(execute_action, SIGNAL(triggered()), this, SLOT(executeFunction()));

    if(editor_name.isEmpty()) {
        execute_action->setVisible(false);
        save_action->setVisible(false);
    }
    else {
        execute_action->setEnabled(false);
        save_action->setEnabled(false);
    }

    selected_execute_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/selected_execute.png")), tr("&Run"), this);
    selected_execute_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    selected_execute_action->setStatusTip(tr("Execute (selected) text"));
    if(!editor_name.isEmpty())
        selected_execute_action->setEnabled(false);
    connect(selected_execute_action, SIGNAL(triggered()), this, SLOT(executeText()));

    wrap_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/wrap.png")), tr("Wrap/Un-wrap lines"),this);
    wrap_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
    wrap_action->setStatusTip(tr("Toggle line wrapping"));
    wrap_action->setCheckable(true);
    wrap_action->setChecked(true);
    connect(wrap_action, SIGNAL(triggered()), this, SLOT(toggleWrap()));

    find_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/find.png")), tr("Find"), this);
    find_action->setShortcuts(QKeySequence::Find);
    find_action->setStatusTip(tr("Find/replace text"));
    connect(find_action, SIGNAL(triggered()), this, SLOT(toggleFindBar()));

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

    backwards_action = new QAction(tr("B"), this);
    backwards_action->setToolTip(tr("Backwards"));
    backwards_action->setCheckable(true);
    backwards_button = new QToolButton;
    backwards_button->setDefaultAction(backwards_action);
    backwards_button->setVisible(false);
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
            QMessageBox::critical(0, tr("Database error"),
                tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery temp_query(database_connection);
        is_saved = temp_query.exec(toPlainText().replace(QChar::ParagraphSeparator,"\n"));
        if(temp_query.lastError().isValid()) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(0, tr("Database error"),
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
    emit newPgxeditor(query);
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

void PgxEditor::findText()
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
        if(backwards_action->isChecked()) {
            find_cursor.movePosition(QTextCursor::StartOfWord);
        }
        highlightCurrentLine();
    }
}

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
}

void PgxEditor::toggleWrap()
{
    if(lineWrapMode() == QPlainTextEdit::WidgetWidth)
        setLineWrapMode(QPlainTextEdit::NoWrap);
    else
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
}

void PgxEditor::toggleFindBar()
{
    if(find_bar->isVisible()) {
        if(find_bar->isActiveWindow()) {
            pgxeditor_mainwin->statusBar()->clearMessage();
            removeHighlighting();
            casesensitivity_button->setVisible(false);
            wholeword_button->setVisible(false);
            backwards_button->setVisible(false);
            find_bar->setVisible(false);
            replace_bar->setVisible(false);
        }
        else
            find_bar->setFocus();
    }
    else {
        find_cursor = textCursor();
        //find_cursor.movePosition(QTextCursor::Start);
        setTextCursor(find_cursor);
        find_bar->setFocus();
        casesensitivity_button->setVisible(true);
        wholeword_button->setVisible(true);
        backwards_button->setVisible(true);
        find_bar->setVisible(true);
        replace_bar->setVisible(true);
    }
}

void PgxEditor::textChangedSlot()
{
    /*QString contents = toPlainText();
    if(!contents.isEmpty()) {
        contents = contents.simplified();
        if(contents.startsWith("CREATE OR REPLACE FUNCTION ", Qt::CaseInsensitive)) {
            editor_name = contents.section(' ', 4, 4, QString::SectionSkipEmpty);
            //QString function_name =
            setTitle(editor_name);
        }
        else {
            editor_name = QLatin1String("");
            setTitle(editor_name);
        }
    }*/
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

void PgxEditor::makeFirstBlocksReadonly()
{
    if(!editor_name.isEmpty()) {
        QTextCursor selection_start_cursor = textCursor();
        selection_start_cursor.setPosition(textCursor().selectionStart());
        if(selection_start_cursor.block() == document()->firstBlock())
            setReadOnly(true);
        else
            setReadOnly(false);
    }
}


void PgxEditor::setTitle(QString title)
{
    pgxeditor_mainwin->setWindowTitle(title);
}

void PgxEditor::setText(QString editor_text, bool save_state)
{
    this->setPlainText(editor_text);
    if(!editor_name.isEmpty())
        save_action->setEnabled(save_state);
}

void PgxEditor::setResizePos(QSize size, QPoint pos)
{
    pgxeditor_mainwin->resize(size);
    pgxeditor_mainwin->move(pos);
    pgxeditor_mainwin->show();
}

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
        find_bar->setPlaceholderText(tr("Find"));
        replace_bar->setPlaceholderText(tr("Replace"));

        casesensitivity_action->setToolTip(tr("Case sensitive"));
        wholeword_action->setToolTip(tr("Whole word"));
        backwards_action->setToolTip(tr("Backwards"));
    }
}

void PgxEditor::wheelEvent(QWheelEvent *wheelEvent)
{
    if(wheelEvent->modifiers() == Qt::ControlModifier) {
        if (wheelEvent->delta()>0)
            font_size < 28 ? font_size++:font_size;
        else {
            font_size > 8 ? font_size--:font_size;
        }
        style_sheet = QString("QPlainTextEdit{background-color: white; \
                              font: bold %1px 'Courier New';}").arg(QString::number(font_size));
        setStyleSheet(style_sheet);
    }
    else
        QPlainTextEdit::wheelEvent(wheelEvent);
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

    QMainWindow::closeEvent(event);
}
