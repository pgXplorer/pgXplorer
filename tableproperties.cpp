
#include "tableproperties.h"

TableProperties::TableProperties(QMainWindow *parent, QString table_name, QStringList table_names_list, bool oid, QString inherits, QString tablespce, int fill_factr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    setParent(parent);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);

    QFont title_font("Helvetica [Cronyx]", 14, QFont::Bold);
    QFont font("Helvetica [Cronyx]");

    title = new QLabel(this);
    title->setAlignment(Qt::AlignHCenter);
    title->setText(tr("Properties"));
    title->setFont(title_font);

    with_oid = new QCheckBox(this);
    with_oid->setChecked(oid);

    /*inherit_like = new QComboBox(this);
    inherit_like->setFont(font);
    inherit_like->addItem("Inherits");
    inherit_like->addItem("Like");*/

    parent_table = new QLineEdit(this);
    parent_table->setText(inherits);
    parent_table->setFont(font);

    completer = new QCompleter(table_names_list, this);

    parent_table->setCompleter(completer);

    tablespace = new QLineEdit(this);
    tablespace->setText(tablespce);
    tablespace->setFont(font);

    fill_factor = new QSpinBox(this);
    fill_factor->setMinimum(10);
    fill_factor->setMaximum(100);
    fill_factor->setSuffix("%");
    fill_factor->setValue(fill_factr);
    fill_factor->setFont(font);

    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button_box->setCenterButtons(true);
    button_box->setFont(font);
    connect(button_box, SIGNAL(accepted()), this, SLOT(okslot()));
    connect(button_box, SIGNAL(rejected()), this, SLOT(close()));

    form_layout = new QFormLayout;
    form_layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    form_layout->setContentsMargins(20,10,20,10);
    form_layout->setVerticalSpacing(20);
    form_layout->addRow(title);
    form_layout->addRow(tr("With oids"), with_oid);
    form_layout->labelForField(with_oid)->setFont(font);
    form_layout->addRow(QLatin1String("Inherits"), parent_table);
    form_layout->labelForField(parent_table)->setFont(font);
    form_layout->addRow(tr("Tablespace"), tablespace);
    form_layout->labelForField(tablespace)->setFont(font);
    form_layout->addRow(tr("Fill factor"), fill_factor);
    form_layout->labelForField(fill_factor)->setFont(font);
    form_layout->addRow(button_box);
    setLayout(form_layout);
}

void TableProperties::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        title->setText(tr("Properties"));
        (static_cast<QLabel*>(form_layout->labelForField(with_oid)))->setText(tr("With oids"));
        (static_cast<QLabel*>(form_layout->labelForField(tablespace)))->setText(tr("Tablespace"));
        (static_cast<QLabel*>(form_layout->labelForField(fill_factor)))->setText(tr("Fill factor"));
    }
}

void TableProperties::okslot()
{
    emit oksignal(with_oid->isChecked(), parent_table->text(), tablespace->text(), fill_factor->text().remove("%").toInt());
    close();
}
