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

#include "comboheader.h"

ComboHeader::ComboHeader(TableView *parent) :
             QHeaderView(Qt::Horizontal, parent)
{
    this->tv = parent;

    connect(this, SIGNAL(sectionResized(int,int,int)), this,
            SLOT(handleSectionResized(int)));
    connect(this, SIGNAL(sectionMoved(int,int,int)), this,
            SLOT(handleSectionMoved(int,int,int)));
    connect(this, SIGNAL(changeGrouping(QStringList)), tv, SLOT(regroup(QStringList)));
}

void ComboHeader::showEvent(QShowEvent *event)
{
    while (!boxes.isEmpty()) {
         delete boxes.takeFirst();
    }

    for (int i=0; i<tv->columnNames().length(); i++) {
        QComboBox *box = new QComboBox(this);
        box->setEnabled(false);

        QString item(tv->columnNames().at(i));
        box->addItem(item.remove("\""));
        box->setItemData(0, Qt::AlignCenter, Qt::TextAlignmentRole);

        box->setEditable(true);
        box->setStyleSheet(QString("QComboBox, QComboBox::drop-down { border: 0px solid gray;\
                                   border-bottom: 1px solid lightgray;\
                                   color: black; \
                                   background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, \
                                                               stop: 0 #FFFFFF, stop: 1 #EEEEEE); \
                              } QComboBox::drop-down { border: 0px}"));
        box->lineEdit()->setReadOnly(true);
        box->lineEdit()->setAlignment(Qt::AlignCenter);

        if(tv->primaryKeys().indexOf(tv->columnNames().at(i)) != -1) {
            box->setItemIcon(0, key_icon);
        }

        boxes.append(box);
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 5, height());
        boxes[i]->show();
    }
    QHeaderView::showEvent(event);
}

void ComboHeader::handleSectionResized(int i)
{
    for(int j=visualIndex(i); j<count(); j++) {
        int logical = logicalIndex(j);
        boxes[logical]->setGeometry(sectionViewportPosition(logical), 0,
                                     sectionSize(logical) - 4, height());
    }
}

void ComboHeader::handleSectionMoved(int i, int oldidx, int newidx)
{
    for(int i=qMin(oldidx, newidx); i<count(); i++) {
        int logical = logicalIndex(i);
        boxes[logical]->setGeometry(sectionViewportPosition(logical), 0,
                                     sectionSize(logical) - 4, height());
    }
}

void ComboHeader::fixComboPositions()
{
    for(int i=0; i<count(); i++) {
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 4, height());
    }
}

void ComboHeader::refreshCombos()
{
    while (!boxes.isEmpty()) {
         delete boxes.takeFirst();
    }

    for (int i=0; i<tv->columnNames().length(); i++) {
        QComboBox *box = new QComboBox(this);
        box->setEditable(true);
        box->lineEdit()->setReadOnly(true);
        box->lineEdit()->setAlignment(Qt::AlignCenter);

        for(int j=0; j<tv->columnAggsList().at(i).length(); j++) {
            if(!tv->columnAggsList().at(i).at(j).isEmpty()) {
                QString item = tv->columnAggsList().at(i).at(j) + "(" + tv->columnNames().at(i) + ")";
                box->addItem(item);
                box->setStyleSheet(QString("QComboBox { border: 0px solid gray;\
                                           border-bottom: 1px solid lightgray;\
                                           color: black; \
                                           background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, \
                                                                       stop: 0 #FFFFFF, stop: 1 #EEEEEE); \
                                      } QComboBox::drop-down { border: 1px}\
                                    QComboBox::down-arrow {image: url(:/icons/down.png);}"));
            }
            else {
                QString item(tv->columnNames().at(i));
                box->addItem(item.remove("\""));
                box->setStyleSheet(QString("QComboBox { border: 0px solid gray;\
                                           border-bottom: 1px solid lightgray;\
                                           color: black; \
                                           background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, \
                                                                       stop: 0 #FFFFFF, stop: 1 #EEEEEE); \
                                      } QComboBox::drop-down { border: 0px}"));
                if(tv->primaryKeys().indexOf(tv->columnNames().at(i)) != -1) {
                    box->setItemIcon(0, key_icon);
                }
            }
            box->setItemData(j, Qt::AlignCenter, Qt::TextAlignmentRole);
        }

        connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(groupingChanged()));
        boxes.append(box);
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 4, height());
        boxes[i]->show();
    }
}

void ComboHeader::groupingChanged()
{
    QStringList aggs;
    for(int i=0; i<tv->columnNames().length(); i++) {
        int idx = boxes.at(i)->currentText().indexOf('(');
        if(idx != -1) {
            QString t(boxes.at(i)->currentText());
            t.truncate(idx);
            aggs.append(t);
        }
        else {
            aggs.append("");
        }
    }
    emit changeGrouping(aggs);
}
