#include "comboheader.h"

ComboHeader::ComboHeader(TableView *parent,
                         QStringList cols,
                         QStringList funcs,
                         QList<QStringList> funcs_list) :
             QHeaderView(Qt::Horizontal, parent)
{
    this->t = parent;
    this->cols = cols;
    this->funcs = funcs;
    this->funcs_list = funcs_list;

    connect(this, SIGNAL(sectionResized(int,int,int)), this,
            SLOT(handleSectionResized(int)));
    connect(this, SIGNAL(sectionMoved(int,int,int)), this,
            SLOT(handleSectionMoved(int,int,int)));
}

void ComboHeader::showEvent(QShowEvent *event)
{
    while (!boxes.isEmpty())
         delete boxes.takeFirst();
    for (int i=0; i<cols.length(); i++) {
        {
            QComboBox *box = new QComboBox(this);
            for(int j=0; j<funcs_list.at(i).length(); j++) {
                QString item = cols.at(i);
                box->addItem(item);
            }
            boxes.append(box);
        }
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 5, height());
        boxes[i]->show();
        connect(boxes[i], SIGNAL(currentIndexChanged(int)), t, SLOT(group()));
    }
    QHeaderView::showEvent(event);
}

void ComboHeader::handleSectionResized(int i)
{
    for(int j=visualIndex(i); j<count(); j++) {
        int logical = logicalIndex(j);
        boxes[logical]->setGeometry(sectionViewportPosition(logical), 0,
                                     sectionSize(logical) - 5, height());
    }
}

void ComboHeader::handleSectionMoved(int i, int oldidx, int newidx)
{
    for(int i=qMin(oldidx, newidx); i<count(); i++) {
        int logical = logicalIndex(i);
        boxes[logical]->setGeometry(sectionViewportPosition(logical), 0,
                                     sectionSize(logical) - 5, height());
    }
}

void ComboHeader::fixComboPositions()
{
    for(int i=0; i<count(); i++) {
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 5, height());
    }
}

void ComboHeader::refreshCombos(QList<QStringList> list)
{
    funcs_list = list;
    while (!boxes.isEmpty())
         delete boxes.takeFirst();
    for (int i=0; i<cols.length(); i++) {
        {
            QComboBox *box = new QComboBox(this);
            for(int j=0; j<funcs_list.at(i).length(); j++) {
                QString item = funcs_list.at(i).at(j) + "(" + cols.at(i) + ")";
                box->addItem(item);
            }
            boxes.append(box);
        }
        boxes[i]->setGeometry(sectionViewportPosition(i), 0,
                              sectionSize(i) - 5, height());
        boxes[i]->show();
        connect(boxes[i], SIGNAL(currentIndexChanged(QString)), t, SLOT(group()));
    }
}
