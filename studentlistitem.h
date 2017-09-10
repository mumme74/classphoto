#ifndef STUDENTLISTITEM_H
#define STUDENTLISTITEM_H

#include <QListWidgetItem>

class StudentBase;

class StudentListItem : public QListWidgetItem
{
    Q_OBJECT
public:
    StudentListItem(QWidget *parent, const QString picturePath);
    ~StudentListItem();

    bool isConnected() const { return reinterpret_cast<long>(m_student) != 0; }
    const QString picturePath() const { return m_picturePath; }
    const QString name() const { return text(); }

    void connectTo(StudentBase *student);
    void disconnect();
    void setName(const QString newName);

private slots:
    void onPicturePathChange(const QString newPicturePath);
    void onNameChange(const QString newName);
    void onStudentDestroyed();

private:

    QPixmap *m_pixmap;
    StudentBase *m_student;
    QString m_picturePath;
};

#endif // STUDENTLISTITEM_H
