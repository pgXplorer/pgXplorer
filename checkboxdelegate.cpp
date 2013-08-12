#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
  : QStyledItemDelegate(parent) {
}

void CheckBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

  QStyleOptionButton check_box_style_option;
  check_box_style_option.state |= QStyle::State_Enabled;
  if (checked) {
      check_box_style_option.state |= QStyle::State_On;
  } else {
      check_box_style_option.state |= QStyle::State_Off;
  }

  QRect rect = option.rect;
  
  if(option.state & QStyle::State_Selected) {
      QLinearGradient selection_gradient(0, 0, 1.0, 0.25);
      selection_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
      selection_gradient.setColorAt(0, QColor::fromRgb(0x5F, 0x5F, 0x7F));
      selection_gradient.setColorAt(0, QColor::fromRgb(0x7F, 0x7F, 0x9F));
      
      QBrush selection(selection_gradient);
      painter->setBrush(selection);
      painter->drawRect(rect);
  }
  
  check_box_style_option.rect = CheckBoxRect(option);

  QApplication::style()->drawControl(QStyle::CE_CheckBox,
                                     &check_box_style_option,
                                     painter);
}

bool CheckBoxDelegate::editorEvent(QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) {
  if ((event->type() == QEvent::MouseButtonRelease) ||
      (event->type() == QEvent::MouseButtonDblClick)) {
    QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
    if (mouse_event->button() != Qt::LeftButton ||
        !CheckBoxRect(option).contains(mouse_event->pos())) {
      return false;
    }
    if (event->type() == QEvent::MouseButtonDblClick) {
      return true;
    }
  } else if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
        static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
      return false;
    }
  } else {
    return false;
  }

  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
  return model->setData(index, !checked, Qt::EditRole);
}
