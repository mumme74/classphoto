#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QDomElement>
#include "mainwindow.h"


#define GRAPHICSVIEW_PIXMAP_WIDTH 130.0
#define GRAPHICSVIEW_PIXMAP_HEIGHT 112.0
#define GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN 20.0
#define GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN 20.0

class QObject;
class QFileSystemWatcher;
class QListView;
class Picture;
class StudentsView;
class ClassNameInGraphicsView;
class StudentInGraphicsView;


class Project : public QObject
{
    Q_OBJECT

public:
    Project(MainWindow *owner, QListView *listView, StudentsView *graphicsView);
    ~Project();

    bool openProjectFile(const QString projectPath);
    bool isOpened() const { return m_isOpen; }

    const QString projectPath() const { return m_projectPath; }
    const QString projectDir() const;
    bool saveProject();
    bool saveProjectToPath(const QString projectPath);
    const QString className() const { return m_className; }
    void setClassName(const QString className);
    bool newProject(const QString projectPath, const QString className = "");
    void closeProject();
    bool hasChanges() const { return m_dirty; }
    void setKnownName(const QString key, const QString name);
    const QString knownName(const QString key) const { return m_knownNames.value(key); }

    void setPixmapForKey(const QString key, const QPixmap pixmap);
    const QPixmap *pixmapForKey(const QString key) const;
    void updateStudentPosition(const QString key);
    void updateClassNamePosition();

    /// insert a student with no picture (placeholder)
    void insertPlaceHolder(const QString name, const QPointF &pos);

    Picture *pictureForKey(const QString key) const;
    void setPictureUpdated(const QString key);

    void setScaleFactor(qreal scaleFactor);
    qreal scaleFactor() const { return m_scaleFactor; }

    bool snapToGrid() const { return m_snapToGrid; }

    MainWindow *mainWindow() const { return m_mainWindow; }

    const QMap<QString, Picture *> *picturesInDir() const { return &m_picturesInDir; }
    const QMap<QString, QString> *knownNames() const { return &m_knownNames; }

    void addStudentToGraphicsView(const QString key, const QPointF point = QPointF(0.0, 0.0));
    void removeStudentFromGraphicsView(const QString key);
    void removeStudentFromGraphicsView(StudentInGraphicsView *student);

    void rebuildListView();
    void rebuildGraphicsView();
    void rebuild();




public slots:
    void onClassNameChange(const QString newClassName);
    void onFileChanged(const QString path);
    void onDirectoryChanged(const QString path);
    void onNameChange(const QString key, const QString newName);

    void setSnapToGrid(bool snapToGrid) { m_snapToGrid = snapToGrid; }

signals:
    void classNameChanged(const QString newClassName);
    void unSavedChanges(bool hasChanges);
    void picturesFilesChanged();
    void studentNameChanged(const QString key, const QString name);
    void openState(bool isOpen);

    void startProgress(int max);
    void progressStep(int step);
    void finishedProgress();

protected:
    void showEvent(QShowEvent *e);

private:
    void initProject(MainWindow *owner);
    void setUpFileWatcher();
    bool scanProjectDirForJpgFiles();
    bool placeStudentInView(const QString key, const QPointF point);
    bool removeStudentFromView(const QString key);
    bool parseClassName(QDomElement elem);
    bool parseScaleFactor(QDomElement elem);
    bool parseKnownNames(QDomElement elem);
    bool parsePicturesInDir(QDomElement elem);
    bool loadPictures();


    bool m_isOpen;
    bool m_dirty;
    qreal m_scaleFactor;
    QString m_className;
    QString m_projectPath;
    QMap<QString, Picture*> m_picturesInDir;
    QMap<QString, QString> m_knownNames;
    QMap<QString, StudentInGraphicsView*> m_graphicsStudents;

    QFileSystemWatcher *m_fileWatcher;
    StudentsView *m_graphicsView;
    QListView     *m_listView;
    ClassNameInGraphicsView *m_classEditor;

    QPointF m_classNamePos;
    bool m_snapToGrid;

    MainWindow *m_mainWindow;
};

#endif // PROJECT_H
