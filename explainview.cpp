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

#include "explainview.h"

ExplainView::ExplainView(QSqlQueryModel *model, QString const name, Qt::WidgetAttribute f)
{
    this->setMod(model);
    this->setWindowTitle(name);
    this->setStyleSheet("QTableView {font-weight: 400;}");
    QShortcut *shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    this->setGeometry(110,110,640,480);
    this->show();
}

void ExplainView::closeEvent(QCloseEvent *event)
{
    if(this->getMod())
        delete this->getMod();
    QWidget::closeEvent(event);
}
