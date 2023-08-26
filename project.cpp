#include "project.h"
#include "availableitemsmodel.h"
#include "picture.h"
#include "studentlistitemdelegate.h"
#include "classnameingraphicsview.h"
#include "studentingraphicsview.h"
#include "studentsview.h"

#undef QT_NO_FILESYSTEMWATCHER

#include <QtWidgets>
#include <QDomDocument>
#include <QDomCDATASection>
#include <QDomElement>
#include <QRegularExpression>
#include <QFileSystemWatcher>

static uint noPicCnt = 0;
static const QString invalidKeystr("..**invalid**..");

qreal stringToQReal(QString str, bool *ok)
{
    bool _ok;
    double scale = str.toDouble(&_ok);
    if (!_ok)
        scale = str.replace(QRegularExpression(","), ".").toDouble(ok);
    return scale;
}

void expectTag(QString fn, QString expected, QString got) {
    qDebug()
        << "wrong tagname in project file, parsing " << fn
        << "expected " << expected << ", got: " << got
        << Qt::endl;
}

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

    m_fileWatcher = nullptr;
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
        m_fileWatcher =nullptr;
    }

    QFile file(m_projectPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc("classphoto");

    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChildElement();
    bool res = true;
    while (!n.isNull() && res) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "className")
                res = parseClassName(e);
            else if (e.tagName() == "scaleFactor")
                res = parseScaleFactor(e);
            else if (e.tagName() == "knownNames")
                res = parseKnownNames(e);
            else if (e.tagName() == "picturesInDir")
                res = parsePicturesInDir(e);
            else {
                qDebug()
                    << "Unknown Xml tag i project file " << e.tagName()
                    << Qt::endl;
                res = false;
                break;
            }
        }
        n = n.nextSibling();
    }

    if (!res) return res;

    // load and display pictures
    loadPictures();

    m_dirty = scanProjectDirForJpgFiles();

    emit unSavedChanges(m_dirty);

    m_mainWindow->setCursor(Qt::ArrowCursor);

    m_isOpen = true;
    emit openState(m_isOpen);

    setUpFileWatcher();

    connect(m_fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    connect(m_fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));

    rebuild();

    return true;

}

bool Project::parseClassName(QDomElement elem) {
    QStringList keys; keys << "posX" << "posY";
    QMap<QString, int> values;
    for (auto key : keys) {
        if (!elem.hasAttribute(key))
            return false;
        bool ok = false;
        values[key] = static_cast<int>(elem.attribute(key).toFloat(&ok));
        if (!ok) return ok;
    }
    QString className = elem.text().trimmed();
    if (className.isEmpty()) return false;

    m_classNamePos.setX(values["posX"]);
    m_classNamePos.setY(values["posY"]);
    m_className = className;

    emit classNameChanged(m_className);
    return true;
}

bool Project::parseScaleFactor(QDomElement elem) {
    bool ok = false;
    qreal value = elem.text().trimmed().toFloat(&ok);
    if (!ok) return false;

    setScaleFactor(value);
    return true;
}

bool Project::parseKnownNames(QDomElement rootElem) {
    bool ok = true;

    auto keyNameLoop = [this, &ok](QDomElement knownName) {
        QDomElement child = knownName.firstChildElement();
        QString key, name;
        while (!child.isNull() && ok) {
            if (child.tagName() == "key")
                key = child.text().trimmed();
            else if (child.tagName() == "name")
                name = child.text().trimmed();
            else {
                expectTag("knownNames", "key or name", child.tagName());
                return false;
            }
            child = child.nextSiblingElement();
        }
        if (name.isEmpty() || key.isEmpty())
            return false;
        m_knownNames[key] = name;
        return true;
    };

    QDomElement elem = rootElem.firstChildElement();
    while (!elem.isNull() && ok) {
        if (elem.tagName() != "knownName") {
            expectTag("knownNames", "knownName", elem.tagName());
            ok = false;
        } else
            ok = keyNameLoop(elem);

        elem = elem.nextSiblingElement();
    }

    if (!ok) {
        m_knownNames.clear();
        return false;
    }
    return true;
}

bool Project::parsePicturesInDir(QDomElement rootElem) {
    bool ok = true;

    auto props = [this, &ok](QDomElement prop) {
        Picture *pic = nullptr;

        qreal scale = stringToQReal(prop.attribute("scaleFactor"), &ok);
        if (!ok) return pic;
        bool isPlaced = prop.attribute("isPlaced").toInt(&ok) != 0;
        if (!ok) return pic;
        qreal posX = stringToQReal(prop.attribute("posX"), &ok);
        if (!ok) return pic;
        qreal posY = stringToQReal(prop.attribute("posY"), &ok);
        if (!ok) return pic;
        QPoint pos(posX, posY);
        int viewPortX = prop.attribute("viewPortX").toInt(&ok);
        if (!ok) return pic;
        int viewPortY = prop.attribute("viewPortY").toInt(&ok);
        if (!ok) return pic;
        int viewPortWidth = prop.attribute("viewPortWidth").toInt(&ok);
        if (!ok) return pic;
        int viewPortHeight = prop.attribute("viewPortHeight").toInt(&ok);
        if (!ok) return pic;
        QRect viewPortRect(viewPortX, viewPortY, viewPortWidth, viewPortHeight);
        qreal brightness = stringToQReal(prop.attribute("brightness"), &ok);
        if (!ok) return pic;
        qreal rotation = prop.attribute("rotation").toInt(&ok);
        if (!ok) return pic;
        qreal rotX = stringToQReal(prop.attribute("rotationPointX"), &ok);
        if (!ok) return pic;
        qreal rotY = stringToQReal(prop.attribute("rotationPointY"), &ok);
        if (!ok) return pic;
        QPointF rotPoint(rotX, rotY);

        // create a new picture
        pic = new Picture(this);
        pic->setViewPort(viewPortRect);
        pic->setRotationPoint(rotPoint);
        pic->setRotation(rotation);
        pic->setBrightness(brightness);
        pic->setPlaced(isPlaced);
        pic->setScale(scale);
        pic->setPos(pos);
        return pic;
    };

    auto keyPropLoop = [this, &ok, &props](QDomElement picElem) {
        QString key;
        Picture *pic = nullptr;

        QDomElement elem = picElem.firstChildElement();
        while (!elem.isNull() && ok) {
            if (elem.tagName() == "key")
                key = elem.text().trimmed();
            else if (elem.tagName() == "properties")
                pic = props(elem);
            else {
                expectTag("picturesInDir", "key or properties", elem.tagName());
                return false;
            }
            if (pic && !key.isEmpty()) {
                this->m_picturesInDir[key] = pic;
                key.clear(); pic = nullptr; // reset for next elem
            }
            elem = elem.nextSiblingElement();
        }
        return ok;
    };

    QDomElement elem = rootElem.firstChildElement();
    while (!elem.isNull() && ok) {
        if (elem.tagName() != "pic") {
            expectTag("picturesInDir", "pic", elem.tagName());
            ok = false;
        } else
            ok = keyPropLoop(elem);

        elem = elem.nextSiblingElement();
    }

    if (!ok) {
        m_picturesInDir.clear();
        return false;
    }

    return true;
}

bool Project::loadPictures() {
    QString dirPath = QFileInfo(m_projectPath).absolutePath() + '/';
    QDir dir(dirPath);
    QStringList files = dir.entryList(
        QStringList() << "*.jpg" << "*.jpeg" << "*.JPG", QDir::Files);


    // notify progressbar how many files we are loading
    emit startProgress(files.count());

    int i = 0;

    for (auto file : files) {
        emit progressStep(i++);
        QCoreApplication::processEvents();

        auto fn = QFileInfo(file).fileName();
        if (!m_picturesInDir.keys().contains(fn))
            m_picturesInDir[fn] = new Picture(this);

        auto pic = m_picturesInDir[fn];
        QPixmap pixmap(QPixmap(dirPath + fn));
        if (!pixmap.isNull())
            pic->setPixmap(pixmap);
    }

    // notify that we are finished loading our pictures
    emit finishedProgress();

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

    QByteArray xml = doc.toByteArray(2);

    QFile file(m_projectPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) || xml.length() < 10)
        return false;
    QTextStream out(&file);
    out << xml;
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

    m_mainWindow->setCursor(Qt::ArrowCursor);

    m_isOpen = true;
    emit openState(m_isOpen);

    setClassName(QFileInfo(m_projectPath).baseName());
    rebuild();

    return saveProject();
}

void Project::setUpFileWatcher() {
    QString dirPath = QFileInfo(m_projectPath).absolutePath() + '/';
    m_fileWatcher = new QFileSystemWatcher(this);
    m_fileWatcher->addPath(dirPath);
    for (auto key : m_picturesInDir.keys()) {
        m_fileWatcher->addPath(dirPath + key);
    }
}

void Project::closeProject()
{
    // m_graphicsView deletes m_graphicStudents
    m_graphicsView->scene()->clear();
    // clear our storage
    m_graphicsStudents.clear();

    if (m_fileWatcher)
           delete m_fileWatcher;

    // memory clean and cleanup
    auto picIt = m_picturesInDir.begin();
    while(picIt != m_picturesInDir.end()) {
        auto delIt = picIt++;
        delete *delIt;
    }
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
            int stepX = static_cast<int>(GRAPHICSVIEW_PIXMAP_WIDTH / 2.0 +
                                         GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN / 2.0);
            int stepY = static_cast<int>(GRAPHICSVIEW_PIXMAP_HEIGHT / 2.0 +
                                         GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN / 2.0);
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
                // dont directly compare doubles
                if (placedStudent->pos().x() > newX -0.1 &&
                    placedStudent->pos().x() < newX +0.1 &&
                    placedStudent->pos().y() > newY -0.1 &&
                    placedStudent->pos().y() < newY +0.1)
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

void Project::insertPlaceHolder(const QString name, const QPointF &pos)
{
    QString key = QString("%1%2").arg(invalidKeystr).arg(noPicCnt++);
    m_knownNames[key] = name;

    Picture *pic = new Picture(this);
    m_picturesInDir[key] = pic;

    addStudentToGraphicsView(key, pos);
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

        if (key.mid(0, invalidKeystr.length()) == invalidKeystr) {
            m_picturesInDir.remove(key);
            m_knownNames.remove(key);
        } else {
            m_picturesInDir.value(key)->setPlaced(false);
            m_picturesInDir.value(key)->setDefaultProperties();

            qobject_cast<AvailableItemsModel *>(m_listView->model())->setVisible(key, true);
        }
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

void Project::removeStudentFromGraphicsView(StudentInGraphicsView *student)
{
    if (!m_isOpen)
        return;

    for (auto &key : m_graphicsStudents.keys()) {
        if (m_graphicsStudents[key] == student)
            removeStudentFromGraphicsView(key);
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
        emit startProgress(list.size());
        int i = 0;
        for(QFileInfo file : list) {
            emit progressStep(i++);
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

        emit finishedProgress();
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

    m_graphicsView->scaleViewToScene();
}

void Project::rebuildListView()
{

    // rebuild the ListViewModel
    if (m_listView->itemDelegate())
        m_listView->itemDelegate()->deleteLater();

    if (m_listView->model())
       m_listView->model()->deleteLater();

    AvailableItemsModel *model = new AvailableItemsModel(this, m_mainWindow);

    // filter out no name placehlder pictures
    QStringList keys;
    for (auto &key : m_picturesInDir.keys()) {
        if (key.left(invalidKeystr.length()) != invalidKeystr)
            keys.append(key);
    }
    if (keys.isEmpty())
        return;

    model->insertRowsFromList(0, keys);

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
    Q_UNUSED(path)
    scanProjectDirForJpgFiles();
}

void Project::onFileChanged(QString path)
{
    QFileInfo file(path);
    if (!file.exists() && m_picturesInDir.contains(file.fileName())) {
        if (QMessageBox::information(m_mainWindow, tr("Filen är borta"),
                                 tr("filen %1 är flyttad eller bortagen av en annan process<br/><br/>"
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

void Project::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    m_graphicsView->scaleViewToScene();
}
