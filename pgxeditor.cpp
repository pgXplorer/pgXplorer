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
    setStyleSheet("QPlainTextEdit{background-color: white; font: bold 14px 'Courier New';}");
    highlighter = new Highlighter(document());

    toolbar = new QToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("pgxeditor");
    toolbar->setMovable(false);
    toolbar->addAction(cut_action);
    toolbar->addAction(copy_action);
    toolbar->addAction(paste_action);
    if(!editor_name.isEmpty()) {
        toolbar->addSeparator();
        toolbar->addAction(save_action);
        toolbar->addSeparator();
        toolbar->addAction(execute_action);
    }
    toolbar->addSeparator();
    toolbar->addAction(selected_execute_action);
    toolbar->addAction(find_action);

    mainwin = new PgxEditorMainWindow;
    mainwin->addToolBar(toolbar);
    mainwin->setCentralWidget(this);
    mainwin->setAttribute(Qt::WA_DeleteOnClose);

    find_bar = new QLineEdit;
    find_bar->setPlaceholderText(tr("Find"));
    find_bar->setVisible(false);
    mainwin->statusBar()->setSizeGripEnabled(false);
    mainwin->statusBar()->addPermanentWidget(casesensitivity_button, 0);
    mainwin->statusBar()->addPermanentWidget(wholeword_button, 0);
    mainwin->statusBar()->addPermanentWidget(backwards_button, 0);
    mainwin->statusBar()->addPermanentWidget(find_bar);
    replace_bar = new QLineEdit;
    replace_bar->setPlaceholderText(tr("Replace"));
    replace_bar->setVisible(false);
    mainwin->statusBar()->addPermanentWidget(replace_bar);

    connect(find_bar, SIGNAL(returnPressed()), this, SLOT(findText()));
    connect(replace_bar, SIGNAL(returnPressed()), this, SLOT(replaceText()));
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChangedSlot()));
    connect(this, SIGNAL(textChanged()), this, SLOT(textChangedSlot()));
    connect(mainwin, SIGNAL(pgxeditorClosing()), this, SLOT(pgxeditorClosing()));
    updateLineNumberAreaWidth(0);
}

int PgxEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void PgxEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
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

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(-1, top+2, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
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
    cut_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/cut.png")), tr("Cut"), this);
    cut_action->setText(tr("Cut"));
    cut_action->setShortcuts(QKeySequence::Cut);
    cut_action->setStatusTip(tr("Cut selected text and copy to clipboard"));
    connect(cut_action, SIGNAL(triggered()), this, SLOT(cut()));

    copy_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/copy.png")), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected text to clipboard"));
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copy()));

    paste_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/paste.png")), tr("Paste"), this);
    paste_action->setShortcuts(QKeySequence::Paste);
    paste_action->setStatusTip(tr("Save function"));
    connect(paste_action, SIGNAL(triggered()), this, SLOT(paste()));

    if(!editor_name.isEmpty()) {
        save_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/save.png")), tr("&Save"), this);
        save_action->setShortcuts(QKeySequence::Save);
        save_action->setStatusTip(tr("Save function"));
        save_action->setEnabled(false);
        connect(save_action, SIGNAL(triggered()), this, SLOT(saveFunction()));

        execute_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/execute.png")), tr("&Execute"), this);
        execute_action->setShortcuts(QKeySequence::Save);
        execute_action->setStatusTip(tr("Execute function"));
        connect(execute_action, SIGNAL(triggered()), this, SLOT(executeFunction()));
    }

    selected_execute_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/selected_execute.png")), tr("&Run"), this);
    selected_execute_action->setShortcuts(QKeySequence::Save);
    selected_execute_action->setStatusTip(tr("Execute function"));
    if(!editor_name.isEmpty())
        selected_execute_action->setEnabled(false);
    connect(selected_execute_action, SIGNAL(triggered()), this, SLOT(executeText()));

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
        QSqlQuery temp_query(toPlainText().replace(QChar::ParagraphSeparator,"\n"), database_connection);
    }
    QSqlDatabase::removeDatabase(QString("save function ").append(editor_name).append(QString::number(editor_widow_id)));
    QApplication::restoreOverrideCursor();
    save_action->setEnabled(false);
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
    QString query;
    emit showQueryView(database, query);
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
    mainwin->statusBar()->clearMessage();
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
            mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
            find_cursor.movePosition(QTextCursor::Start);
        }
        else {
            mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
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
    mainwin->statusBar()->clearMessage();
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
            mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
            find_cursor.movePosition(QTextCursor::Start);
        }
        else {
            mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
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

void PgxEditor::toggleFindBar()
{
    if(find_bar->isVisible()) {
        if(find_bar->isActiveWindow()) {
            mainwin->statusBar()->clearMessage();
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
    if(!editor_name.isEmpty())
        save_action->setEnabled(true);
}

void PgxEditor::setText(QString editor_text, bool save_state)
{
    this->setPlainText(editor_text);
    if(!editor_name.isEmpty())
        save_action->setEnabled(save_state);
}

void PgxEditor::setResizePos(QSize size, QPoint pos)
{
    mainwin->resize(size);
    mainwin->move(pos);
    mainwin->show();
}

void PgxEditor::pgxeditorClosing()
{
    emit pgxeditorClosing(this);
}

void PgxEditor::closeMain()
{
    mainwin->close();
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
