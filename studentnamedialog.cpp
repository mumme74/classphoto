#include "studentnamedialog.h"
#include "ui_studentnamedialog.h"

StudentNameDialog::StudentNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudentNameDialog)
{
    ui->setupUi(this);
    ui->txtName->setFocus();
}

StudentNameDialog::~StudentNameDialog()
{
    delete ui;
}

void StudentNameDialog::accept()
{
    QDialog::accept();
}

const QString StudentNameDialog::name() const
{
    return ui->txtName->text();
}
