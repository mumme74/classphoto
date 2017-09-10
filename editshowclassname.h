#ifndef EDITSHOWCLASSNAME_H
#define EDITSHOWCLASSNAME_H

#include <QWidget>

class Project;
class QLineEdit;
class QLabel;


class EditShowClassName : public QWidget
{
Q_OBJECT
public:
    explicit EditShowClassName(QWidget *parent = 0);
    ~EditShowClassName();


    void mouseDoubleClickEvent(QMouseEvent *);
    void init(Project *project);



signals:

public slots:
    void onClassNameChanged(const QString newName);
    void onEditingFinished();
    void onEditorLostFocus();

private:
    void toggleState();

    Project *m_project;
    QLineEdit *m_editor;
    QLabel *m_label;

    bool m_isEditing;

};


#endif // EDITSHOWCLASSNAME_H
