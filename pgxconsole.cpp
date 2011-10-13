#include "pgxconsole.h"
#include "queryview.h"
#include "explainview.h"
#include <QSqlQueryModel>
#include <QSqlQuery>

PgxConsole::PgxConsole(QWidget *parent) : QPlainTextEdit(parent)
{
    setViewportMargins(10, 0, 0, 0);
    setTabStopWidth(40);
    setUndoRedoEnabled(false);
    setCaption(QApplication::translate("PgxConsole", "SQL console", 0, QApplication::UnicodeUTF8));
    setGeometry(100,100,640,480);
    setStyleSheet("QPlainTextEdit{background-color: white; font: bold 14px;}");
    highlighter = new Highlighter(document());
    prompt = new Prompt(this);
    QShortcut *shortcut_paste = new QShortcut(QKeySequence::Paste, this);
    connect(shortcut_paste, SIGNAL(activated()), this, SLOT(paste_cmd()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(curChanged()));
    connect(this, SIGNAL(cmd(QKeyEvent *)), this, SLOT(showView(QKeyEvent *)));
    connect(this, SIGNAL(histUp()), this, SLOT(histUpCmd()));
    connect(this, SIGNAL(histDn()), this, SLOT(histDnCmd()));
    //connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updatePromptWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updatePrompt(QRect,int)));
    //updatePromptWidth(0);
}

/*
void PgxConsole::updatePromptWidth(int)
{
    setViewportMargins(10, 0, 0, 0);
}
*/

void PgxConsole::updatePrompt(const QRect &rect, int dy)
{
    if (dy)
        prompt->scroll(0, dy);
    else
        prompt->update(0, rect.y(), prompt->width(), rect.height());
    //if (rect.contains(viewport()->rect()))
    //    updatePromptWidth(0);
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
     //painter.fillRect(event->rect(), Qt::white);
     QTextBlock block = firstVisibleBlock();
     int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
     int bottom = top + (int) blockBoundingRect(block).height();
     while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
             QString pr;
             if(block.text().startsWith("ERROR")) {
                 pr = QString("x");
                 painter.setPen(QColor(255,127,127));
             }
             else if(block.text().startsWith("Driver not loaded")) {
                 pr = QString("x");
                 painter.setPen(QColor(255,127,127));
             }
             else if(block.text().startsWith("WARNING")) {
                 pr = QString(" ");
                 painter.setPen(QColor(255,127,127));
             }
             else if(block.text().startsWith("MESSAGE")) {
                 pr = QString(" ");
                 painter.setPen(QColor(255,127,127));
             }
             else// if(block == firstVisibleBlock()) {
             {
                 pr = QString(">");
                 painter.setPen(Qt::lightGray);
             }
             painter.drawText(0, top, prompt->width(), fontMetrics().height(),
                              Qt::AlignCenter, pr);
        }
        QTextBlock prevBlock = block.previous();
        block = block.next();

        //if(prevBlock.isVisible() && prevBlock.isValid())
        //{
        //    if (prevBlock.text().endsWith("\\")) {
        //        painter.setPen(Qt::lightGray);
        //        painter.drawText(0, top, prompt->width(), fontMetrics().height(),
        //                         Qt::AlignRight, "+");
        //    }
        //    else if(prevBlock.text().startsWith("ERROR"))
        //    {
        //        painter.setPen(Qt::red);
        //        painter.drawText(0, top, prompt->width(), fontMetrics().height(),
        //                         Qt::AlignRight, "x");
        //    }
        //    else
        //    {
        //        painter.setPen(Qt::lightGray);
        //        painter.drawText(0, top, prompt->width(), fontMetrics().height(),
        //                         Qt::AlignRight, ">");
        //    }
        //}

        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
    }
}

void PgxConsole::keyPressEvent(QKeyEvent * e)
{
    switch(e->key()) {
    case Qt::Key_Up:
        emit histUp();
        break;
    case Qt::Key_Down:
        emit histDn();
        break;
    case Qt::Key_Left:
        if(!textCursor().atBlockStart())
            QPlainTextEdit::keyPressEvent(e);
        break;
    case Qt::Key_Backspace:
        if(!textCursor().atBlockStart())
            QPlainTextEdit::keyPressEvent(e);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit cmd(e);
        //QPlainTextEdit::keyPressEvent(e);
        break;
    case Qt::Key_Backslash:
        QPlainTextEdit::keyPressEvent(e);
        appendPlainText("");
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
    }
}

void PgxConsole::wheelEvent(QWheelEvent *wheelEvent)
{
    // BEGIN TODO 2
    wheelEvent->accept();
    QFontMetrics df = fontMetrics();
    // END TODO 2
}

void PgxConsole::showView(QKeyEvent * e)
{
    if(!textCursor().atEnd()) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }
    QTextBlock block = document()->end();
    if(!block.isValid())
        block = block.previous();
    QString cmd = block.text().trimmed();
    for (block = block.previous(); block.text().endsWith("\\"); block = block.previous())
        cmd.insert(0, block.text().trimmed().replace(QString("\\"), QString(" ")));
    // Do nothing on whitespace input.
    if(cmd.trimmed().isEmpty()) {
        appendPlainText("");
        return;
    }
    // Save the command into the command history and
    // reassign the history iterator.
    history << cmd;
    hit = history.size();
    // 'clear' command to clear the console keeping the
    // history intact.
    if(cmd.compare("clear", Qt::CaseInsensitive) == 0) {
        clear();
        return;
    }
    // 'clear' command to clear the history alone. Console
    // is not cleared.
    else if(cmd.compare("clearh", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit = 0;
        appendPlainText("");
        return;
    }
    // 'clearall' command to clear console and history
    else if(cmd.compare("clearall", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit =0;
        clear();
        return;
    }
    // 'quit' command to clear console and history
    else if(cmd.compare("quit", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit =0;
        clear();
        this->close();
    }
    // 'exit' command to clear console and history
    else if(cmd.compare("exit", Qt::CaseInsensitive) == 0) {
        history.clear();
        hit =0;
        clear();
        this->close();
    }
    // Reduce all groups of whitespace characters
    // to a single space between words.
    cmd = cmd.simplified();
    // Start a timer to count the seconds taken to display the
    // required output (used for queries and tables only).
    QTime t;
    t.start();
    // Don't execute SELECT INTO queries.
    // Suggest CREATE TABLE AS construct.
    QRegExp rx("\\bINTO\\b\\s+[\\w]*\\s+\\bFROM\\b", Qt::CaseInsensitive);
    if(cmd.startsWith("select", Qt::CaseInsensitive) &&
       cmd.contains(rx)) {
        appendPlainText(QApplication::translate(
                "PgxConsole", "WARNING: SELECT INTO deprecated. Use CREATE TABLE AS construct instead.\n",
                0, QApplication::UnicodeUTF8));
        return;
    }
    // EXPLAIN queries display.
    else if(cmd.startsWith("explain", Qt::CaseInsensitive))
    {
        /*!
           @todo EXPLAIN ANALYZE on DML statements.
         * Explain analyze executes the DML thereby altering data.
         * This can be potentially hazardous when the intention is
         * to only check the query run time, rows affected, etc.
         * The proper way to implement this is to enclose the DML
         * in a transaction block like so:
         * @code
         * BEGIN;
         *   {DML};
         * ROLLBACK;
         * @endcode
         * Yet to be implemented. EXPLAIN ANALYZE on
         * DML are disabled for now.
         */
        if(cmd.startsWith("explain analyze update", Qt::CaseInsensitive) ||
           cmd.startsWith("explain analyze insert", Qt::CaseInsensitive) ||
           cmd.startsWith("explain analyze delete", Qt::CaseInsensitive) ||
           cmd.startsWith("explain analyze verbose update", Qt::CaseInsensitive) ||
           cmd.startsWith("explain analyze verbose insert", Qt::CaseInsensitive) ||
           cmd.startsWith("explain analyze verbose delete", Qt::CaseInsensitive))
        {
            appendPlainText(QApplication::translate("PgxConsole", "WARNING: EXPLAIN ANAYZE for DML not supported at this time.\n", 0, QApplication::UnicodeUTF8));
            return;
        }

        QSqlQueryModel* model = new QSqlQueryModel;
        model->setQuery(cmd);
        if (model->lastError().isValid()) {
            appendHtml(model->lastError().databaseText());
            appendPlainText("");
            return;
        }
        ExplainView* eview = new ExplainView(model, cmd, Qt::WA_DeleteOnClose);
        int width = eview->verticalHeader()->width();
        if(eview->verticalScrollBar()->isVisible()) {
            width += eview->verticalScrollBar()->width();
        }
        eview->setColumnWidth(0, eview->width() - width);
        eview->resizeRowsToContents();
        eview->show();
    }
    // DDL, DML queries display.
    else
    {
        if((cmd.startsWith("update", Qt::CaseInsensitive) ||
           cmd.startsWith("insert", Qt::CaseInsensitive) ||
           cmd.startsWith("delete", Qt::CaseInsensitive) ||
           cmd.startsWith("truncate", Qt::CaseInsensitive)) &&
           !cmd.contains("returning", Qt::CaseInsensitive))
        {
            QSqlQuery* sqlqry = new QSqlQuery;
            sqlqry->exec(cmd);
            if (sqlqry->lastError().isValid()) {
                appendHtml(sqlqry->lastError().databaseText());
                appendPlainText("");
                return;
            }
            QString mesg = QApplication::translate("PgxConsole", "MESSAGE: Rows affected = ", 0, QApplication::UnicodeUTF8);
            appendHtml(mesg.append(QString::number(sqlqry->numRowsAffected())));
            appendPlainText("");
            return;
        }
        else
        {
            QSqlQueryModel* model = new QSqlQueryModel;
            model->setQuery(cmd);
            if (model->lastError().isValid()) {
                appendHtml(model->lastError().databaseText());
                appendPlainText("");
                return;
            }
            if(model->rowCount() == 0) {
                appendPlainText(QApplication::translate("PgxConsole", "MESSAGE: No rows to display.\n", 0, QApplication::UnicodeUTF8));
                return;
            }
            QueryView* qview = new QueryView(0, model, cmd, t.elapsed(), model->rowCount(),
                                               model->columnCount(), Qt::WA_DeleteOnClose);
            qview->show();
        }
    }
    appendPlainText("");
}

void PgxConsole::curChanged()
{
    if(textCursor().block().next().isValid())
        setReadOnly(true);
    else
        setReadOnly(false);
}

void PgxConsole::paste_cmd()
{
    QTextCursor cursor = textCursor();
    if(textCursor().block().next().isValid()) {
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);

        paste();
    }
    else {
        /*QClipboard *clipboard = QApplication::clipboard();
        QString cb = clipboard->text();
        QStringList cbl = cb.split("\n");
        siz = cbl.size();
        for(int i = 0; i < siz; i++) {
            appendPlainText(cbl.at(i));
        }*/
        paste();
    }
}

void PgxConsole::histUpCmd()
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
        else {
            insertPlainText(history.at(--hit));
        }
    }
}

void PgxConsole::histDnCmd()
{
    if(!history.isEmpty() && hit < history.size()) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        cursor.select(QTextCursor::BlockUnderCursor);
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
            appendPlainText(history.value(++hit));
        }
        else {
            insertPlainText(history.value(++hit));
        }
    }
    ensureCursorVisible();
}

void PgxConsole::createWidgets()
{
    QStringList wordList;
    wordList << "alpha" << "omega" << "omicron" << "zeta";
    completer = new QCompleter(wordList, this);
    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWrapAround(true);
}

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\b[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);
    keywordFormat.setForeground(Qt::darkCyan);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bselect\\b" << "\\bupdate\\b" << "\\bdelete\\b"
                    << "\\btruncate\\b" << "\\bunion\\b" << "\\ball\\b"
                    << "\\bintersect\\b" << "\\bexcept\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    keywordFormat2.setForeground(Qt::darkGreen);
    keywordFormat2.setFontItalic(true);
    QStringList keywordPatterns2;
    keywordPatterns2 << "\\bfrom\\b" << "\\bin\\b" << "\\bwith\\b"
                     << "\\bjoin\\b" << "\\bon\\b" << "\\bgroup by\\b"
                     << "\\bleft\\b" << "\\right\\b" << "\\bfull\\b" << "\\bcross\\b"
                     << "\\binner\\b" << "\\bouter\\b" << "\\bnatural\\b"
                     << "\\border by\\b" << "\\blimit\\b" << "\\bfetch\\b"
                     << "\\bhaving\\b" << "\\bwindow\\b" << "\\boffset\\b";
    foreach (const QString &pattern, keywordPatterns2) {
        rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
        rule.format = keywordFormat2;
        highlightingRules.append(rule);
    }
    singleQuotFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\'([^\']*)\'");
    rule.format = singleQuotFormat;
    highlightingRules.append(rule);
    doubleQuotFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegExp("\"([^\"]*)\"");
    rule.format = doubleQuotFormat;
    highlightingRules.append(rule);
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}