#ifndef CREATECLASSDLG_H
#define CREATECLASSDLG_H

#include <QDialog>
#include <QLineEdit>

namespace Ui {
    class CreateClassDlg;
}

class QSettings;
class CreateClassDlg : public QDialog
{
    Q_OBJECT
public:
    explicit CreateClassDlg(QSettings *setting, QDialog *parent = nullptr);
    virtual ~CreateClassDlg();

    QString projectPath();
    QString picturesPath();
    QString className();

    //void accept();

signals:

private slots:
    void classNameKeypressEvent(QKeyEvent *evt);
    void picturesPathBrowse();
    void projectPathBrowse();
    void projectPathTextEdited();

protected:
    bool eventFilter(QObject *obj, QEvent *evt);

private:
    bool checkAndCompletePath(QLineEdit *edit, QKeyEvent *evt);
    QString projectPathDefault();
    QString picturesPathDefault();
    void autoChangeProjectPath();
    QSettings *m_settings;
    Ui::CreateClassDlg *ui;
    bool m_projectPathBlockAutoChange;

};

#endif // CREATECLASSDLG_H
