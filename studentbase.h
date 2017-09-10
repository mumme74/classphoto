#ifndef STUDENTBASE_H
#define STUDENTBASE_H

//#include <QGlobal>
#include <QObject>
#include <QString>
#include <QRect>

class Project;
class QPixmap;

class StudentBase : public QObject
{
    Q_OBJECT
public:
    StudentBase(Project *project);
    ~StudentBase();

    const QString name() const { return m_name; }
    const QString picturePath() const { return m_picturePath; }
    const QRect visibleRect() const { return m_visibleRect; }
    qreal scaleFactor() const { return m_scaleFactor; }

    bool setName(const QString newName);
    bool setPicturePath(const QString newPicturePath);
    bool savePictureAt(const QString fileName);


signals:
    void nameChanged(const QString newName);
    void picturePathChanged(const QString newPicturePath);
    void close(const QString pictureFileName);

protected:
    Project *m_project;
    QString  m_name;
    QString  m_picturePath;
    QRect    m_visibleRect;
    qreal    m_scaleFactor;
    QPixmap *m_pixmap;
};

#endif // STUDENTBASE_H
