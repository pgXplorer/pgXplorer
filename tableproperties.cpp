
#include "tableproperties.h"

TableProperties::TableProperties(QMainWindow *parent, QString table_name)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    setParent(parent);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);

    QFont title_font("Helvetica [Cronyx]", 14, QFont::Normal);
    QFont font("Helvetica [Cronyx]");

    title = new QLabel(this);
    title->setAlignment(Qt::AlignHCenter);
    title->setText(tr("Properties"));
    title->setFont(title_font);

    with_oid = new QCheckBox(this);

    inherit_like = new QComboBox(this);
    inherit_like->setFont(font);
    inherit_like->addItem("Inherit");
    inherit_like->addItem("Like");

    inherit_like_cb = new QCheckBox(this);

    tablespace = new QLineEdit(this);
    tablespace->setFont(font);

    fill_factor = new QSpinBox(this);
    fill_factor->setMinimum(10);
    fill_factor->setMaximum(100);
    fill_factor->setSuffix("%");
    fill_factor->setValue(100);
    fill_factor->setFont(font);

    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button_box->setCenterButtons(true);
    button_box->setFont(font);
    connect(button_box, SIGNAL(accepted()), this, SLOT(okslot()));
    connect(button_box, SIGNAL(rejected()), this, SLOT(close()));

    QFormLayout *form_layout = new QFormLayout;
    form_layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    form_layout->setContentsMargins(20,10,20,10);
    form_layout->setVerticalSpacing(20);
    form_layout->addRow(title);
    form_layout->addRow(tr("With oids"), with_oid);
    form_layout->labelForField(with_oid)->setFont(font);
    form_layout->addRow(inherit_like, inherit_like_cb);
    form_layout->addRow(tr("Tablespace"), tablespace);
    form_layout->addRow(tr("Fill factor"), fill_factor);
    form_layout->addRow(button_box);
    setLayout(form_layout);
}
