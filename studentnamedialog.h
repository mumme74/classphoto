#ifndef STUDENTNAMEDIALOG_H
#define STUDENTNAMEDIALOG_H

#include <QDialog>

namespace Ui {
    class StudentNameDialog;
}

class StudentNameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StudentNameDialog(QWidget *parent = nullptr);
    ~StudentNameDialog();

    void accept();

    const QString name() const;

signals:

public slots:


private:
    Ui::StudentNameDialog *ui;
};

#endif // STUDENTNAMEDIALOG_H
