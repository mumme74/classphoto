#include "studentbase.h"
#include "project.h"
#include <QtWidgets>

StudentBase::StudentBase(Project *project)
    : QObject(project),
    m_name(""),
    m_picturePath(""),
    m_visibleRect(0, 0, 0, 0),
    m_scaleFactor(1.0),
    m_pixmap(nullptr)
{
    m_project = project;
}

StudentBase::~StudentBase()
{
    QString fileName = "";
    if (!m_picturePath.isEmpty()) {
        fileName = QFileInfo(m_picturePath).fileName();
    }
    emit close(fileName);

    delete m_pixmap;
    m_pixmap = nullptr;
}

bool StudentBase::setName(QString newName)
{
    m_name = newName;
    emit nameChanged(newName);
    return true;
}

bool StudentBase::setPicturePath(QString newPicturePath)
{
    if (!QFileInfo(newPicturePath).exists()) {
        QMessageBox::information(m_project->mainWindow(),
                                 tr("Bild finns ej"), tr("Bilden %1 finns inte.")
                                    .arg(newPicturePath),
                                 QMessageBox::Ok);
        return false;
    }

    emit picturePathChanged(newPicturePath);

    m_picturePath = newPicturePath;
    if (!m_pixmap)
        m_pixmap = new QPixmap;

    if (!m_pixmap->load(newPicturePath)) {
        delete m_pixmap;
        m_pixmap = nullptr;
        return false;
    }

    return true;
}

// this method should almost never be used, only when the originating file is lost in the file system.
bool StudentBase::savePictureAt(QString fileName)
{
    if (m_pixmap) {
        m_pixmap->save(fileName);
        return true;
    }
    return false;
}
