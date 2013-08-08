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

#include "pgxconsole.h"
#include "queryview.h"

PgxConsole::PgxConsole(Database *database)
{
    this->database = database;
    insertPlainText("");
    setViewportMargins(10, 0, 0, 0);
    setTabStopWidth(40);
    setUndoRedoEnabled(false);
    setWindowTitle(QApplication::translate("PgxConsole", "SQL console", 0));
    setStyleSheet("QPlainTextEdit{background-color: white; font: bold 14px 'Courier New';}");

    createActions();
    hit = 0;

    highlighter = new Highlighter(document());
    prompt = new Prompt(this);

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("pgxeditor");
    toolbar->setMovable(false);
    toolbar->addAction(newpgxconsole_action);
    toolbar->addAction(cut_action);
    toolbar->addAction(copy_action);
    toolbar->addAction(paste_action);
    toolbar->addSeparator();
    toolbar->addAction(clear_action);
    toolbar->addAction(find_action);

    pgxconsole_mainwin = new PgxConsoleMainWindow;
    pgxconsole_mainwin->addToolBar(toolbar);
    pgxconsole_mainwin->setCentralWidget(this);
    pgxconsole_mainwin->setAttribute(Qt::WA_DeleteOnClose);

    find_bar = new QLineEdit;
    find_bar->setPlaceholderText(tr("Find"));
    find_bar->setMaximumSize(100,find_bar->height());
    find_bar->setVisible(false);
    pgxconsole_mainwin->statusBar()->setSizeGripEnabled(false);
    pgxconsole_mainwin->statusBar()->addPermanentWidget(casesensitivity_button, 0);
    pgxconsole_mainwin->statusBar()->addPermanentWidget(wholeword_button, 0);
    //pgxconsole_mainwin->statusBar()->addPermanentWidget(backwards_button, 0);
    pgxconsole_mainwin->statusBar()->addPermanentWidget(find_bar);
    pgxconsole_mainwin->statusBar()->addPermanentWidget(find_previous_button, 0);
    pgxconsole_mainwin->statusBar()->addPermanentWidget(find_next_button, 0);

    connect(find_bar, SIGNAL(returnPressed()), SLOT(findText()));

    QShortcut *shortcut_paste = new QShortcut(QKeySequence::Paste, this);
    connect(shortcut_paste, SIGNAL(activated()), SLOT(pasteFromClipboard()));

    QShortcut *shortcut_single_paste = new QShortcut(QKeySequence("Ctrl+Shift+V"), this);
    connect(shortcut_single_paste, SIGNAL(activated()), SLOT(pasteAsSingleFromClipboard()));

    connect(this, SIGNAL(cursorPositionChanged()), SLOT(makePreviousBlocksReadonly()));

    //Tie command with QueryView creation.
    connect(this, SIGNAL(commandSignal(QString)), SLOT(showView(QString)));

    //Tie up and down keys with command history scolling.
    connect(this, SIGNAL(historyUp()), SLOT(historyUpCommand()));
    connect(this, SIGNAL(historyDown()), SLOT(historyDownCommand()));

    //Console updates.
    connect(this, SIGNAL(updateRequest(QRect,int)), SLOT(updatePrompt(QRect,int)));
    connect(pgxconsole_mainwin, SIGNAL(pgxconsoleClosing()), SLOT(pgxconsoleClosing()));
}

void PgxConsole::updatePrompt(const QRect &rect, int dy)
{
    if (dy)
        prompt->scroll(0, dy);
    else
        prompt->update(0, rect.y(), prompt->width(), rect.height());
}

void PgxConsole::highlightSearchedWord()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection border;
    QPen outline(Qt::darkGreen, 2, Qt::SolidLine);
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

void PgxConsole::removeSearchHighlighting()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
}

void PgxConsole::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    prompt->setGeometry(QRect(cr.left(), cr.top(), 10, cr.height()));
}

void PgxConsole::promptPaintEvent(QPaintEvent *event)
{
     QPainter painter(prompt);
     QTextBlock block = firstVisibleBlock();
     int blockNumber = block.blockNumber();
     int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
     int bottom = top + (int) blockBoundingRect(block).height();
     while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString pr = QLatin1String(">");
            QString number;
            if(block.previous().isVisible() && block.previous().isValid()) {
                number = QString::number(blockNumber + 1);
                if(block.previous().text().endsWith("\\"))
                    pr = QLatin1String(" ");
                else
                    pr = QLatin1String(">");
            }

            if(block.text().endsWith("\\")) {
                if(block.previous().text().endsWith("\\"))
                    pr = QLatin1String(" ");
                else
                    pr = QLatin1String(">");
            }
            painter.setPen(Qt::lightGray);
            painter.drawText(0, top, prompt->width(), fontMetrics().height(),
                             Qt::AlignCenter, pr);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void PgxConsole::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Up:
        emit historyUp();
        break;
    case Qt::Key_Down:
        emit historyDown();
        break;
    case Qt::Key_Left:
        if(!textCursor().atBlockStart())
            QPlainTextEdit::keyPressEvent(event);
        break;
    case Qt::Key_Backspace:
        if(!textCursor().atBlockStart())
            QPlainTextEdit::keyPressEvent(event);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit commandSignal(QString());
        break;
    case Qt::Key_Backslash:
        QPlainTextEdit::keyPressEvent(event);
        appendPlainText("");
        break;
    default:
        QPlainTextEdit::keyPressEvent(event);
    }
}

void PgxConsole::wheelEvent(QWheelEvent *wheelEvent)
{
    wheelEvent->accept();
    QFontMetrics df = fontMetrics();
}

void PgxConsole::showView(QString cmd)
{
    if(cmd.isEmpty()) {
        if(!textCursor().atEnd()) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
        }
        QTextBlock block = document()->end();
        if(!block.isValid())
            block = block.previous();
        cmd = block.text().trimmed();
        for (block = block.previous(); block.text().endsWith("\\"); block = block.previous())
            cmd.insert(0, block.text().trimmed().replace(QLatin1String("\\"), QLatin1String(" ")));
        // Do nothing on whitespace input.
        if(cmd.trimmed().isEmpty()) {
            appendPlainText("");
            return;
        }
    }
    // Save the command into the command history and
    // reassign the history iterator.
    history << cmd;
    hit = history.size();

    // 'clear' command to clear the console keeping the
    // history intact.
    if(cmd.compare("clear", Qt::CaseInsensitive) == 0)
        clear();
    // 'clearh' command to clear the history alone. Console
    // is not cleared.
    else if(cmd.compare("clearh", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit = 0;
        appendPlainText("");
    }
    // 'clearall' command to clear console and history
    else if(cmd.compare("clearall", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit = 0;
        clear();
    }
    // 'quit' command to clear console and history
    else if(cmd.compare("quit", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit = 0;
        clear();
        pgxconsole_mainwin->close();
        return;
    }
    // 'exit' command to clear console and history
    else if(cmd.compare("exit", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit = 0;
        clear();
        pgxconsole_mainwin->close();
    }
    else {
        // Reduce all groups of whitespace characters
        // to a single space between words.
        cmd = cmd.simplified();
        // Start a timer to count the seconds taken to display the
        // required output (used for queries and tables only).
        QTime t;
        t.start();
        emit showQueryView(database, cmd);
        appendPlainText("");
    }
}

void PgxConsole::makePreviousBlocksReadonly()
{
    if(textCursor().block().next().isValid())
        setReadOnly(true);
    else
        setReadOnly(false);
}

void PgxConsole::pasteFromClipboard()
{
    if(!textCursor().atEnd()) {
        if(textCursor().block().next().isValid()) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
        }
    }
    QClipboard *clipboard = QApplication::clipboard();
    QString clipboard_text = clipboard->text();
    QStringList clipboard_text_list = clipboard_text.trimmed().split("\n", QString::SkipEmptyParts);
    int clipboard_text_list_size = clipboard_text_list.size();
    for(int i = 0; i < clipboard_text_list_size; i++) {
        insertPlainText(clipboard_text_list.at(i));
        if(i != clipboard_text_list_size-1)
            showView(clipboard_text_list.at(i));
    }
}

void PgxConsole::pasteAsSingleFromClipboard()
{
    if(!textCursor().atEnd()) {
        if(textCursor().block().next().isValid()) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
        }
    }
    QClipboard *clipboard = QApplication::clipboard();
    QString clipboard_text = clipboard->text();
    QStringList clipboard_text_list = clipboard_text.trimmed().split("\n", QString::SkipEmptyParts);
    int clipboard_text_list_size = clipboard_text_list.size();
    for(int i = 0; i < clipboard_text_list_size; i++) {
        appendPlainText(clipboard_text_list.at(i));
        if(i != clipboard_text_list_size-1)
            insertPlainText(" \\");
    }
    //showView(clipboard_text_list.join(" "));
}

void PgxConsole::findText()
{
    QTextDocument::FindFlags find_flags;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;
    if(backwards_action->isChecked())
        find_flags |= QTextDocument::FindBackward;
    pgxconsole_mainwin->statusBar()->clearMessage();
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
            pgxconsole_mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
            find_cursor.movePosition(QTextCursor::Start);
        }
        else {
            pgxconsole_mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
            find_cursor.movePosition(QTextCursor::End);
        }
    }
    else {
        if(backwards_action->isChecked()) {
            find_cursor.movePosition(QTextCursor::StartOfWord);
        }
        highlightSearchedWord();
    }
}

void PgxConsole::findPrevious()
{
    QTextDocument::FindFlags find_flags;
    find_flags |= QTextDocument::FindBackward;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxconsole_mainwin->statusBar()->clearMessage();
    if(find_cursor.atStart()) {
        find_cursor.movePosition(QTextCursor::End);
        setTextCursor(find_cursor);
    }

    if(find(find_bar->text(), find_flags)) {
        find_cursor = textCursor();
        find_cursor.movePosition(QTextCursor::StartOfWord);
        highlightSearchedWord();
    }
    else {
        pgxconsole_mainwin->statusBar()->showMessage(tr("Reached the top. Continuing from the bottom."));
        find_cursor.movePosition(QTextCursor::Start);
    }
}

void PgxConsole::findNext()
{
    QTextDocument::FindFlags find_flags;
    if(casesensitivity_action->isChecked())
        find_flags |= QTextDocument::FindCaseSensitively;
    if(wholeword_action->isChecked())
        find_flags |= QTextDocument::FindWholeWords;

    pgxconsole_mainwin->statusBar()->clearMessage();
    if(find_cursor.atEnd()) {
        find_cursor.movePosition(QTextCursor::Start);
        setTextCursor(find_cursor);
    }

    if(find(find_bar->text(), find_flags)) {
        find_cursor = textCursor();
        highlightSearchedWord();
    }
    else {
        pgxconsole_mainwin->statusBar()->showMessage(tr("Reached the end. Continuing from the top."));
        find_cursor.movePosition(QTextCursor::End);
    }
}

void PgxConsole::toggleFindBar()
{
    if(find_bar->isVisible()) {
        if(find_bar->isActiveWindow()) {
            pgxconsole_mainwin->statusBar()->clearMessage();
            removeSearchHighlighting();
            casesensitivity_button->setVisible(false);
            wholeword_button->setVisible(false);
            //backwards_button->setVisible(false);
            find_bar->setVisible(false);
            find_next_button->setVisible(false);
            find_previous_button->setVisible(false);
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
        //backwards_button->setVisible(true);
        find_bar->setVisible(true);
        find_next_button->setVisible(true);
        find_previous_button->setVisible(true);
    }
}

void PgxConsole::insertFromMimeData(const QMimeData *)
{
    pasteFromClipboard();
}

void PgxConsole::historyUpCommand()
{
    if(!history.isEmpty() && hit > 0) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        cursor.select(QTextCursor::BlockUnderCursor);
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
            appendPlainText(history.at(--hit));
        }
        else
            insertPlainText(history.at(--hit));
    }
}

void PgxConsole::historyDownCommand()
{
    if(!history.isEmpty() && hit < history.size()-1) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        cursor.select(QTextCursor::BlockUnderCursor);
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
            appendPlainText(history.value(++hit));
        }
        else
            insertPlainText(history.value(++hit));
    }
    ensureCursorVisible();
}

void PgxConsole::__createWidgets()
{
    QStringList wordList;
    wordList << "alpha" << "omega" << "omicron" << "zeta";
    completer = new QCompleter(wordList, this);
    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWrapAround(true);
}

void PgxConsole::createActions()
{
    newpgxconsole_action = new QAction(QIcon(":/icons/console.png"), tr("New"), this);
    newpgxconsole_action->setShortcuts(QKeySequence::New);
    newpgxconsole_action->setStatusTip(tr("New console"));
    connect(newpgxconsole_action, SIGNAL(triggered()), this, SIGNAL(newPgxconsole()));

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
    paste_action->setStatusTip(tr("Paste"));
    connect(paste_action, SIGNAL(triggered()), SLOT(pasteAsSingleFromClipboard()));

    clear_action = new QAction(QIcon(":/icons/clear.png"), tr("&Clear"), this);
    clear_action->setStatusTip(tr("Clear the console"));
    connect(clear_action, SIGNAL(triggered()), SLOT(clear()));

    find_action = new QAction(QIcon(":/icons/search.png"), tr("Find"), this);
    find_action->setShortcuts(QKeySequence::Find);
    find_action->setStatusTip(tr("Find text"));
    find_action->setCheckable(true);
    connect(find_action, SIGNAL(triggered()), SLOT(toggleFindBar()));

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
}

void PgxConsole::setResizePos(QSize size, QPoint pos, QSize icon_size)
{
    pgxconsole_mainwin->resize(size);
    pgxconsole_mainwin->move(pos);
    toolbar->setIconSize(icon_size);
    pgxconsole_mainwin->show();
}

void PgxConsole::pgxconsoleClosing()
{
    emit pgxconsoleClosing(this);
}

void PgxConsole::closeMain()
{
    pgxconsole_mainwin->close();
}

void PgxConsole::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        newpgxconsole_action->setText(tr("New"));
        cut_action->setText(tr("Cut"));
        copy_action->setText(tr("Copy"));
        paste_action->setText(tr("Paste"));
        clear_action->setText(tr("&Clear"));
        find_action->setText(tr("Find"));

        newpgxconsole_action->setStatusTip(tr("New console"));
        cut_action->setStatusTip(tr("Cut selected text and copy to clipboard"));
        copy_action->setStatusTip(tr("Copy selected text to clipboard"));
        paste_action->setStatusTip(tr("Paste"));
        clear_action->setStatusTip(tr("Clear the console"));
        find_action->setStatusTip(tr("Find text"));
        casesensitivity_action->setToolTip(tr("Case sensitive"));
        wholeword_action->setToolTip(tr("Whole word"));
        backwards_action->setToolTip(tr("Backwards"));
    }
}

void PgxConsoleMainWindow::closeEvent(QCloseEvent *event)
{
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //TableView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    emit pgxconsoleClosing();

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("pgxconsole_maximized", true);
        showNormal();
    }
    else
        settings.setValue("pgxconsole_maximized", false);
    settings.setValue("pgxconsole_pos", pos());
    settings.setValue("pgxconsole_size", size());

    QMainWindow::closeEvent(event);
}

void PgxConsoleMainWindow::bringOnTop()
{
    activateWindow();
    raise();
}
