#include "editshowclassname.h"

#include "project.h"
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>

EditShowClassName::EditShowClassName(QWidget *parent) :
    QWidget(parent),
    m_project(0),
    m_isEditing(false)
{
}


EditShowClassName::~EditShowClassName()
{
    delete m_editor;
    m_editor = 0;
    delete m_label;
    m_label = 0;
}

void EditShowClassName::init(Project *project)
{
    m_project = project;

    m_editor = new QLineEdit(m_project->className(), this);
    m_label = new QLabel(m_project->className(), this);
    m_label->setMargin(5);

    QFont myFont;
    myFont.setStyleHint(QFont::Decorative);
    myFont.setStyle(QFont::StyleItalic);
    myFont.setWeight(QFont::Bold);
    myFont.setBold(true);

    m_label->setFont(myFont);
    m_editor->setFont(myFont);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_editor);
    layout->addWidget(m_label);
    this->setLayout(layout);

    m_editor->setVisible(false);
    m_label->setVisible(true);

    connect(m_editor, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_project, SIGNAL(classNameChanged(QString)), this, SLOT(onClassNameChanged(QString)));
    connect(m_editor, SIGNAL(lostFocus()), this, SLOT(onEditorLostFocus()));
}


void EditShowClassName::toggleState()
{
    m_isEditing = !m_isEditing;
    m_editor->setVisible(m_isEditing);
    m_label->setVisible(!m_isEditing);
    if (m_isEditing) {
        m_editor->setFocus();
        QFontMetrics fm(m_editor->font());
        m_editor->resize(m_editor->width() + 30, m_editor->height()); // add some space to type in

    }
}

void EditShowClassName::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    toggleState();
}


// --------------------- event slots goes down here -----------------------
void EditShowClassName::onClassNameChanged(QString newName)
{
    m_editor->setText(newName);
    m_label->setText(newName);
}


void EditShowClassName::onEditingFinished()
{
    if (m_editor->isModified()) {
        m_project->setClassName(m_editor->text());
        m_editor->setModified(false);
    }

    if (m_isEditing)
        toggleState();
}

void EditShowClassName::onEditorLostFocus()
{
    if (m_isEditing)
        toggleState();
}
