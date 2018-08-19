#include "project.h"
#include <QtWidgets>
#include <QDomDocument>
#include <QDomCDATASection>
#include <QDomElement>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlItem>


#include "availableitemsmodel.h"
#include "picture.h"
#include "studentlistitemdelegate.h"
#include "classnameingraphicsview.h"
#include "studentingraphicsview.h"
#include "studentsview.h"

Project::Project(MainWindow *owner, QListView *listView, StudentsView *graphicsView)
    : QObject(owner),
    m_graphicsView(graphicsView),
    m_listView(listView)
{
    m_graphicsView->init(this);

    QGraphicsScene *scene = new QGraphicsScene(m_graphicsView);
    scene->setSceneRect(QRectF(m_graphicsView->contentsRect()));
    m_graphicsView->setScene(scene);

    initProject(owner);

    //connect(m_mainWindow, SIGNAL(show()), m_graphicsView, SLOT(scaleViewToScene()));
}

Project::~Project()
{
    delete m_fileWatcher;
}

void Project::initProject(MainWindow *owner)
{
    m_dirty = false;
    m_isOpen = false;
    m_className = "";
    m_projectPath = "";
    m_mainWindow = owner;

    m_fileWatcher = 0;
    m_scaleFactor = 1.0;

    m_picturesInDir.clear();
    m_knownNames.clear();

    m_classNamePos.setX(-(m_graphicsView->width() / 2));
    m_classNamePos.setY(-(m_graphicsView->height() / 2));

    m_snapToGrid = true;

    emit unSavedChanges(m_dirty);
}

bool Project::openProjectFile(const QString projectPath)
{
    m_projectPath = projectPath;
    m_mainWindow->setCursor(Qt::WaitCursor);

    if (m_fileWatcher) {
        delete m_fileWatcher;
        m_fileWatcher = 0;
    }

    QFile file(m_projectPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QXmlQuery query;
    QString value; QString key;
    QStringList values; QStringList keys;

    query.bindVariable("inputDocument", &file);

    QString x, y;
    query.setQuery("doc($inputDocument)/class/className/@posX/string()");
    query.evaluateTo(&x);
    query.setQuery("doc($inputDocument)/class/className/@posY/string()");
    query.evaluateTo(&y);
    m_classNamePos.setX(QVariant(x).toFloat());
    m_classNamePos.setY(QVariant(y).toFloat());

    query.setQuery("doc($inputDocument)/class/className/text()");
    query.evaluateTo(&value);
    m_className = value.trimmed();
    emit classNameChanged(m_className);


    query.setQuery("doc($inputDocument)/class/scaleFactor/value/string()");
    query.evaluateTo(&value);
    if (!value.trimmed().isEmpty())
        setScaleFactor(value.trimmed().toFloat());


    query.setQuery("doc($inputDocument)/class/knownNames/knownName/key/string()");
    query.evaluateTo(&keys);
    query.setQuery("doc($inputDocument)/class/knownNames/knownName/name/string()");
    query.evaluateTo(&values);

    qDebug() << key <<endl;

    for(int i = 0; i < keys.count(); ++i) {
        if (values.count() > i)
            m_knownNames[keys.at(i)] = values.at(i);
    }

    keys.clear();
    values.clear();

    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/key/string()");
    query.evaluateTo(&keys);

    QStringList rotations;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@rotation/string()");
    query.evaluateTo(&rotations);

    QStringList scaleFactors;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@scaleFactor/string()");
    query.evaluateTo(&scaleFactors);

    QStringList brightness;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@brightness/string()");
    query.evaluateTo(&brightness);

    QStringList viewPortXs;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@viewPortX/string()");
    query.evaluateTo(&viewPortXs);

    QStringList viewPortYs;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@viewPortY/string()");
    query.evaluateTo(&viewPortYs);

    QStringList viewPortWidths;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@viewPortWidth/string()");
    query.evaluateTo(&viewPortWidths);

    QStringList viewPortHeights;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@viewPortHeight/string()");
    query.evaluateTo(&viewPortHeights);

    QStringList placed;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@isPlaced/string()");
    query.evaluateTo(&placed);

    QStringList posX;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@posX/string()");
    query.evaluateTo(&posX);

    QStringList posY;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@posY/string()");
    query.evaluateTo(&posY);

    QStringList rotationPointX;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@rotationPointX/string()");
    query.evaluateTo(&rotationPointX);

    QStringList rotationPointY;
    query.setQuery("doc($inputDocument)/class/picturesInDir/pic/properties/@rotationPointY/string()");
    query.evaluateTo(&rotationPointY);

    QString dir = QFileInfo(m_projectPath).absolutePath() + '/';
    for (int i = 0; i < keys.count(); ++i) {
        Picture *pic = new Picture(this);
        QPixmap pixmap(QPixmap(dir + keys.at(i)));
        pic->setPixmap(pixmap);
        if (rotations.count() > i)
            pic->setRotation(rotations.at(i).toInt());

        if (scaleFactors.count() > i)
            pic->setScale(scaleFactors.at(i).toFloat());

        if (brightness.count() > i)
            pic->setBrightness(brightness.at(i).toFloat());

        int x = 0; int y = 0; int w = 0; int h = 0;

        if (viewPortXs.count() > i)
            x = viewPortXs.at(i).toInt();

        if (viewPortYs.count() > i)
            y = viewPortYs.at(i).toInt();

        if (viewPortWidths.count() > i)
            w = viewPortWidths.at(i).toInt();

        if (viewPortHeights.count() > i)
            h = viewPortHeights.at(i).toInt();

        if (x && y && w && h) {
            QRect rect(x, y, w, h);
            pic->setViewPort(rect);
        }

        if (placed.count() > i)
            pic->setPlaced(static_cast<bool>(placed.at(i).toInt()));

        if (posX.count() > i && posY.count() > i) {
            pic->setPos(QPointF(posX.at(i).toFloat(), posY.at(i).toFloat()));
        }

        if (rotationPointX.count() > i && rotationPointY.count() > i) {
            pic->setRotationPoint(QPointF(rotationPointX.at(i).toFloat(), rotationPointY.at(i).toFloat()));
        }
        m_picturesInDir[keys.at(i)] = pic;
    }

    m_dirty = scanProjectDirForJpgFiles();

    emit unSavedChanges(m_dirty);

    m_mainWindow->setCursor(Qt::ArrowCursor);

    m_isOpen = true;
    emit openState(m_isOpen);

    m_fileWatcher = new QFileSystemWatcher(this);
    m_fileWatcher->addPath(dir);
    foreach(key, keys) {
        m_fileWatcher->addPath(dir + key);
    }

    connect(m_fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    connect(m_fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));

    rebuild();

    return true;
}


bool Project::saveProject()
{
    QDomDocument doc("classPhoto");
    QDomElement root = doc.createElement("class");
    QDomElement className = doc.createElement("className");
    className.setAttribute("posX", QVariant(m_classNamePos.x()).toString());
    className.setAttribute("posY", QVariant(m_classNamePos.y()).toString());
    QDomCDATASection nameValue = doc.createCDATASection(m_className);
    className.appendChild(nameValue);
    root.appendChild(className);
    doc.appendChild(root);

    //save scalefactor
    QDomElement scaleFactor = doc.createElement("scaleFactor");
    scaleFactor.setAttribute("value", m_scaleFactor);

    // save the known names
    QDomElement knownNames = doc.createElement("knownNames");
    root.appendChild(knownNames);

    for (QMap<QString, QString>::const_iterator it = m_knownNames.constBegin();
         it != m_knownNames.constEnd(); ++it)
    {
        QDomElement item = doc.createElement("knownName");
        knownNames.appendChild(item);

        QDomElement key = doc.createElement("key");
        QDomCDATASection keyValue = doc.createCDATASection(it.key());
        key.appendChild(keyValue);
        item.appendChild(key);

        QDomElement name = doc.createElement("name");
        QDomCDATASection nameValue = doc.createCDATASection(it.value());
        name.appendChild(nameValue);
        item.appendChild(name);
    }

    //Save the pictures
    QDomElement picturesInDir = doc.createElement("picturesInDir");
    root.appendChild(picturesInDir);

    for (QMap<QString, Picture*>::const_iterator it = m_picturesInDir.constBegin();
         it != m_picturesInDir.constEnd(); ++it)
    {
        QDomElement pic = doc.createElement("pic");
        picturesInDir.appendChild(pic);

        QDomElement key = doc.createElement("key");
        QDomCDATASection keyValue = doc.createCDATASection(it.key());
        key.appendChild(keyValue);
        pic.appendChild(key);

        Picture *picture = it.value();

        QDomElement properties = doc.createElement("properties");
        properties.setAttribute("scaleFactor", picture->scaleFactor());
        properties.setAttribute("brightness", picture->brightness());
        properties.setAttribute("rotation", picture->rotation());

        QRect rect = picture->viewPort();

        properties.setAttribute("viewPortX", rect.x());
        properties.setAttribute("viewPortY", rect.y());
        properties.setAttribute("viewPortWidth", rect.width());
        properties.setAttribute("viewPortHeight", rect.height());

        properties.setAttribute("isPlaced", picture->isPlaced());
        properties.setAttribute("posX", picture->pos().x());
        properties.setAttribute("posY", picture->pos().y());

        properties.setAttribute("rotationPointX", picture->rotationPoint().x());
        properties.setAttribute("rotationPointY", picture->rotationPoint().y());

        pic.appendChild(properties);

    }

    QFile file(m_projectPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << doc.toString(2);
    file.close();

    m_dirty = false;
    emit unSavedChanges(m_dirty);

    return true;
}

bool Project::saveProjectToPath(const QString projectPath)
{
    m_projectPath = projectPath;
    return saveProject();
}



bool Project::newProject(QString projectPath, QString className)
{
    if (!className.isEmpty())
        m_className = className;

    m_dirty = true;

    scanProjectDirForJpgFiles();

    m_projectPath = projectPath;

    emit unSavedChanges(m_dirty);

    m_isOpen = true;
    emit openState(m_isOpen);

    setClassName(QFileInfo(m_projectPath).baseName());
    rebuild();

    return saveProject();
}

void Project::closeProject()
{
    m_graphicsView->scene()->clear();

    if (m_fileWatcher)
           delete m_fileWatcher;

    m_picturesInDir.clear();
    initProject(m_mainWindow);

    m_dirty = false;
    emit unSavedChanges(m_dirty);

    m_isOpen = false;
    emit openState(m_isOpen);

    rebuild();
}

void Project::setKnownName(const QString key, const QString name)
{
    m_knownNames[key] = name;
    m_dirty = true;
    qobject_cast<AvailableItemsModel *>(m_listView->model())->updateRow(key);
    if (m_graphicsStudents.contains(key))
        m_graphicsStudents[key]->studentNameChanged(name);

    emit studentNameChanged(key, name);
    emit unSavedChanges(m_dirty);
}

void Project::setClassName(QString className)
{
    m_className = className;
    m_dirty = true;
    emit classNameChanged(className);
    emit unSavedChanges(m_dirty);
}

const QPixmap *Project::pixmapForKey(QString key) const
{
    return m_picturesInDir.value(key)->pixmap();
}

void Project::setPixmapForKey(QString key, const QPixmap pixmap)
{
    if (!m_picturesInDir.contains(key)) {
        Picture *pic = new Picture(this);
        pic->setPixmap(pixmap);
        m_picturesInDir[key] = pic;
        rebuildListView();
    } else {
        m_picturesInDir[key]->setPixmap(pixmap);
        qobject_cast<AvailableItemsModel *>(m_listView->model())->updateRow(key);

        if (m_graphicsStudents.contains(key)) {
            m_graphicsStudents[key]->studentPixmapChanged(new QPixmap(pixmap));
            m_dirty = true;
            emit unSavedChanges(m_dirty);
        }
    }
}

void Project::updateStudentPosition(QString key)
{
    if (m_picturesInDir.contains(key) && m_graphicsStudents.contains(key)) {
        StudentInGraphicsView *student = m_graphicsStudents.value(key);

        if (m_snapToGrid) {
            qreal x = student->pos().x();
            qreal y = student->pos().y();
            qreal newX = 0;
            qreal newY = 0;
            int stepX = GRAPHICSVIEW_PIXMAP_WIDTH / 2 +
                       GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN / 2;
            int stepY =  GRAPHICSVIEW_PIXMAP_HEIGHT / 2 +
                         GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN / 2;
            if (x >= 0) {
                while ((newX - stepX < x) && (newX + stepX < x)) {
                    newX += stepX * 2;
                }
            } else {
                while ((newX - stepX > x) && (newX + stepX > x)) {
                    newX -= stepX * 2;
                }
            }

            if (y >= 0) {
                while ((newY - stepY < y) && (newY + stepY < y)) {
                    newY += stepY * 2;
                }
            } else {
                while ((newY - stepY > y) && (newY + stepY > y)) {
                    newY -= stepY * 2;
                }
            }

            // is there already a student at this position?
            foreach(StudentInGraphicsView *placedStudent, m_graphicsStudents) {
                if (static_cast<int>(placedStudent->pos().x()) == newX &&
                    static_cast<int>(placedStudent->pos().y()) == newY)
                {
                    newX -= student->boundingRect().width() / 10;
                    newY -= student->boundingRect().height() / 10;
                    break;
                }
            }

            student->setPos(newX, newY);

        }

        m_graphicsView->scaleViewToScene();

        m_picturesInDir.value(key)->setPos(student->scenePos());

        m_dirty = true;
        emit unSavedChanges(m_dirty);
    }
}

void Project::updateClassNamePosition()
{
    m_classNamePos.setX(m_classEditor->pos().x());
    m_classNamePos.setY(m_classEditor->pos().y());

    m_graphicsView->scaleViewToScene();

    m_dirty = true;
    emit unSavedChanges(m_dirty);
}

bool Project::placeStudentInView(QString key, QPointF point)
{
    if (m_isOpen && !m_graphicsStudents.contains(key)) {
        StudentInGraphicsView *student = new StudentInGraphicsView(this, key);
        m_graphicsView->scene()->addItem(student);
        student->setPos(point);
        m_graphicsStudents[key] = student;

        Picture *pic = m_picturesInDir.value(key);
        pic->setPos(student->scenePos());
        pic->setPlaced(true);

        qobject_cast<AvailableItemsModel *>(m_listView->model())->setVisible(key, false);
        return true;
    }
    return false;
}

void Project::addStudentToGraphicsView(const QString key, const QPointF point)
{
    if (placeStudentInView(key, point)) {
        if (m_snapToGrid)
            updateStudentPosition(key);
        m_dirty = true;
        emit unSavedChanges(m_dirty);
    }
}

bool Project::removeStudentFromView(const QString key)
{
    if (m_isOpen && m_graphicsStudents.contains(key)) {
        StudentInGraphicsView *student = m_graphicsStudents[key];
        m_graphicsStudents.remove(key);
        m_graphicsView->scene()->removeItem(student);
        student->deleteLater();

        m_picturesInDir.value(key)->setPlaced(false);
        m_picturesInDir.value(key)->setDefaultProperties();

        qobject_cast<AvailableItemsModel *>(m_listView->model())->setVisible(key, true);
        return true;
    }
    return false;
}

void Project::removeStudentFromGraphicsView(QString key)
{
    if (removeStudentFromView(key)) {
        m_dirty = true;
        emit unSavedChanges(m_dirty);
    }
}

void Project::setScaleFactor(qreal scaleFactor)
{
    m_scaleFactor = scaleFactor;
    m_classEditor->setScale(scaleFactor);

    foreach(StudentInGraphicsView *student, m_graphicsStudents) {
        student->setScale(scaleFactor);
    }


    m_dirty = true;
    emit unSavedChanges(m_dirty);
}

Picture *Project::pictureForKey(QString key) const
{
    return m_picturesInDir.value(key);
}

void Project::setPictureUpdated(QString key)
{
    qobject_cast<AvailableItemsModel *>(m_listView->model())->updateRow(key);

    if (m_graphicsStudents.contains(key)) {
        m_graphicsStudents[key]->studentPixmapChanged(m_picturesInDir.value(key)->pixmap());
    }

    m_dirty = true;
    emit unSavedChanges(m_dirty);
}

bool Project::scanProjectDirForJpgFiles()
{
    bool hasChanges = false;

    if (!m_projectPath.isEmpty()) {
        QDir dir = QFileInfo(m_projectPath).absoluteDir();
        dir.setFilter(QDir::Files | QDir::Readable);

        QList<QString> searchedFiles;

        QFileInfoList list = dir.entryInfoList();
        foreach(QFileInfo file, list) {
            if (file.suffix().toLower() == "jpg" || file.suffix().toLower() == "jpeg") {
                searchedFiles.append(file.fileName());
                if (!m_picturesInDir.contains(file.fileName())) {
                    Picture *pix = new Picture(this);
                    QPixmap pixmap(file.absoluteFilePath());
                    pix->setPixmap(pixmap);

                    m_picturesInDir[QString(file.fileName())] = pix;
                    hasChanges = true;
                }
            }
        }

        // are there any deleted files?
        if (searchedFiles.count() < m_picturesInDir.count()) {
            foreach(QString key, m_picturesInDir.keys()) {
                if (!searchedFiles.contains(key))
                    m_picturesInDir.remove(key);
            }
            hasChanges = true;
        }
    }

    if (hasChanges) {
        rebuildListView();
        emit picturesFilesChanged();
    }

    return hasChanges;
}


void Project::rebuild()
{
    rebuildListView();
    rebuildGraphicsView();

    // place the stored ones
    foreach (QString key, m_picturesInDir.keys()) {
        Picture *pic = m_picturesInDir.value(key);
        if (pic->isPlaced())
            placeStudentInView(key, pic->pos());
    }
}

void Project::rebuildListView()
{

    // rebuild the ListViewModel
    if (m_listView->itemDelegate())
        m_listView->itemDelegate()->deleteLater();

    if (m_listView->model())
       m_listView->model()->deleteLater();

    AvailableItemsModel *model = new AvailableItemsModel(this, m_mainWindow);
    model->insertRows(0, m_picturesInDir.keys());

    StudentListItemDelegate *delegate = new StudentListItemDelegate(this, m_listView);

    m_listView->setItemDelegate(delegate);
    m_listView->setModel(model);
}

void Project::rebuildGraphicsView()
{
    if (m_graphicsView->scene() && m_graphicsView->scene()->items().count()) {
        m_graphicsView->scene()->clear();
    }
    m_graphicsView->setAcceptDrops(true);

    if (isOpened()) {
        if (!m_graphicsView->scene()) {
            QGraphicsScene *scene = new QGraphicsScene(m_graphicsView);
            scene->setSceneRect(QRectF(m_graphicsView->contentsRect()));
            m_graphicsView->setScene(scene);
        }

        m_classEditor = new ClassNameInGraphicsView(this);
        m_graphicsView->scene()->addItem(m_classEditor);

        m_classEditor->setPos(m_classNamePos);

        m_graphicsView->scaleViewToScene();
    }
}


// --------------------------------- event slots below here -----------------------
void Project::onClassNameChange(QString newClassName)
{
    setClassName(newClassName);
}

void Project::onDirectoryChanged(QString path)
{
    Q_UNUSED(path);
    scanProjectDirForJpgFiles();
}

void Project::onFileChanged(QString path)
{
    QFileInfo file(path);
    if (!file.exists() && m_picturesInDir.contains(file.fileName())) {
        if (QMessageBox::information(m_mainWindow, trUtf8("Filen är borta"),
                                 trUtf8("filen %1 är flyttad eller bortagen av en annan process<br/><br/>"
                                        "Vill du spara filen du har i minnet på sagda plats?").arg(file.fileName()),
                                 QMessageBox::Yes, QMessageBox::No))
        {
            Picture *pic = m_picturesInDir.value(file.fileName());
            if (pic && pic->originalPixmap())
                pic->originalPixmap()->save(path);
        }
    }
}

void Project::onNameChange(const QString key, const QString newName)
{
    QFileInfo file(m_projectPath);
    if (QFileInfo(file.absolutePath() + '/' + key).exists())
        m_knownNames[key] = QString(newName);

}
