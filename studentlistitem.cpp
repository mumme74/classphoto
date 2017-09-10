#include "studentlistitem.h"
#include "studentbase.h"

StudentListItem::StudentListItem(QWidget *parent, const QString picturePath)
    : QListWidgetItem(parent, QListWidgetItem::UserType),
    m_pixmap(0),
    m_student(0),
    m_picturePath("")
{

}

StudentListItem::~StudentListItem()
{
    delete m_pixmap;
    m_pixmap = 0;
}

void StudentListItem::connectTo(StudentBase *student)
{
    if (isConnected())
        disconnect();

    connect(student, SIGNAL(nameChanged(QString)), this, SLOT(onNameChange(QString)));
    connect(student, SIGNAL(picturePathChanged(QString)), this, SLOT(onPicturePathChange(QString)));
    connect(student, SIGNAL(destroyed()), this, SLOT(onStudentDestroyed()));

    m_student = student;
    this->setHidden(true);
}

void StudentListItem::disconnect()
{
    disconnect();
    m_student = 0;
    setHidden(false);
}

void StudentListItem::setName(const QString newName)
{
    // the connet system will take care of changing name
    if (isConnected())
        m_student->setName(newName);
    else
        setText(newName);
}


// ------------------------------- event slots goes down here ------------------
void StudentListItem::onNameChange(const QString newName)
{
    setText(newName);
}

void StudentListItem::onPicturePathChange(QString newPicturePath)
{
    if (!m_pixmap)
        m_pixmap = new QPixmap;

    m_pixmap->load(newPicturePath);
    setText(text());// trigger update
}
