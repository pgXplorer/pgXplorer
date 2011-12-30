#include "pgconsole.h"
#include "mainWin.h"
#include "queryview.h"

PgConsole::PgConsole()
{
    this->setCaption("SQL console");
    this->setGeometry(100,100,640,480);
    this->setFontPointSize(12);
    QObject::connect(this, SIGNAL(sqlcmd(QString)), this, SLOT(showView(QString)));
}

void PgConsole::keyPressEvent(QKeyEvent * e)
{
    QTextCursor cursor = this->textCursor();
    QStringList qsl;
    switch(e->key())
    {
    case Qt::Key_Escape:
        this->close();
        break;
    case Qt::Key_Up:
        break;
    case Qt::Key_Left:
        QTextEdit::keyPressEvent(e);
        break;
    case Qt::Key_Backspace:
        QTextEdit::keyPressEvent(e);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        qsl = this->toPlainText().split("\n\n");
        if(QString (qsl.last()).compare("") != 0)
        {
            this->insertPlainText("\n\n");
        }
        emit sqlcmd(qsl.last());
        break;
    default:
        QTextEdit::keyPressEvent(e);
    }
}

void PgConsole::showView(QString cmd)
{
    if(cmd.trimmed().compare("") == 0)
    {
        return;
    }
    QSqlQueryModel* model = new QSqlQueryModel;

    model->setQuery(cmd);
    //model->setEditStrategy(QSqlQueryModel::OnManualSubmit);
    //model->select();

    if (model->lastError().isValid())
    {
        QMessageBox::critical(this, qApp->tr("Query error"),
            model->lastError().text(), QMessageBox::Cancel);
        return;
    }

    QueryView* view = new QueryView(0, model, cmd, 0, Qt::WA_DeleteOnClose);
    view->show();
}
