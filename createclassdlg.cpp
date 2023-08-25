#include "createclassdlg.h"
#include "ui_createclassdlg.h"
#include "settings.h"
#include <QKeyEvent>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QDir>

CreateClassDlg::CreateClassDlg(QSettings *setting, QDialog *parent) :
    QDialog(parent),
    m_settings(setting),
    ui(new Ui::CreateClassDlg),
    m_projectPathBlockAutoChange(false)
{
    ui->setupUi(this);

    qApp->installEventFilter(this);

    connect(ui->btnFindPicturesFolder, SIGNAL(clicked()),
            this, SLOT(picturesPathBrowse()));
    connect(ui->btnFindProjectFolder, SIGNAL(clicked()),
            this, SLOT(projectPathBrowse()));
    connect(ui->txtProjectPath, SIGNAL(textEdited(QString)),
            this, SLOT(projectPathTextEdited()));

    ui->txtClassName->setText("nytt_klass_foto");
    ui->txtPicturesFolder->setText(picturesPathDefault());
    ui->txtProjectPath->setText(projectPathDefault());
}

CreateClassDlg::~CreateClassDlg()
{
}

QString CreateClassDlg::projectPath()
{
    return ui->txtProjectPath->text();
}

QString CreateClassDlg::picturesPath()
{
    return ui->txtPicturesFolder->text();
}

QString CreateClassDlg::className()
{
    return ui->txtClassName->text();
}

void CreateClassDlg::classNameKeypressEvent(QKeyEvent *evt)
{
    QRegExp re(QStringLiteral("/^[a-z0-9_.@()-]+\\.[^.]+$/i"));
    if (re.exactMatch(ui->txtClassName->text() + evt->text()))
        ui->txtClassName->setStyleSheet("color:black");
    else
        ui->txtClassName->setStyleSheet("color:red");

    autoChangeProjectPath();
}

void CreateClassDlg::picturesPathBrowse()
{
    QString projectPath = ui->txtPicturesFolder->text();
    QFileInfo fi(projectPath);
    if (!(fi.exists() && fi.isDir() && fi.isWritable())) {
        QFileInfo path(m_settings->value(LAST_OPENED_PROJECT_PATH,
                                                          QDir::homePath()).toString());
        if (path.isFile())
            projectPath = path.absolutePath();
        else
            projectPath = path.absoluteFilePath();

    }

    projectPath = QFileDialog::getExistingDirectory(this,
                                      tr("Mapp med elevbilder"),
                                      projectPath,
                                      QFileDialog::DontResolveSymlinks); //tr("bilder (*.jpg, *.jpeg)")
    if (projectPath.isEmpty())
        return; // canceled
    ui->txtPicturesFolder->setText(projectPath);
    ui->txtPicturesFolder->setStyleSheet("color:black;");
    autoChangeProjectPath();
}

void CreateClassDlg::projectPathBrowse()
{
    QString projectPath = ui->txtProjectPath->text();
    QFileInfo fi(projectPath);
    if (!(fi.exists() && fi.isDir() && fi.isWritable())) {
        projectPath = projectPathDefault();
    }

    projectPath = QFileDialog::getSaveFileName(this, tr("Namn på klassen projektet (ta klassnamnet..)"),
                                               projectPath, tr("klassfoto filer (*.xml)"));
    if (projectPath.isEmpty())
        return; // canceled
    else if (projectPath.right(4).toLower() != ".xml") {
        projectPath += ".xml";
    }
    m_projectPathBlockAutoChange = true;
    ui->txtProjectPath->setText(projectPath);
}

void CreateClassDlg::projectPathTextEdited()
{
    m_projectPathBlockAutoChange = true;
}

bool CreateClassDlg::eventFilter(QObject *obj, QEvent *evt)
{
    if (evt->type() == QEvent::KeyPress) {
        if (obj == ui->txtClassName)
            classNameKeypressEvent(static_cast<QKeyEvent*>(evt));
        else if (obj == ui->txtPicturesFolder ||
                 obj == ui->txtProjectPath)
        {
            bool stopEvent = checkAndCompletePath(static_cast<QLineEdit*>(obj), static_cast<QKeyEvent*>(evt));
            autoChangeProjectPath();
            if (stopEvent)
                return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

// returns true when evetn should be stopped
bool CreateClassDlg::checkAndCompletePath(QLineEdit *edit, QKeyEvent *evt)
{
    bool ret = false;
    QString path = edit->text();
    if (evt->key() != Qt::Key_Tab)
        path += evt->text();
    else
        ret = true;

    QFileInfo fi(path);
    QStringList names = fi.dir().entryList(QDir::NoFilter ,QDir::Name);
    for (QString name : names) {
        if (name.left(fi.fileName().length()) == fi.fileName()) {
            if (evt->key() == Qt::Key_Tab) {
                edit->setText(fi.path() + QDir::separator() + name);
            }
            edit->setStyleSheet("color:black;");
            return ret;
        }
    }
    edit->setStyleSheet("color:red;");
    return ret;
}

QString CreateClassDlg::projectPathDefault()
{
    QFileInfo path(m_settings->value(LAST_OPENED_PROJECT_PATH,
                                     QDir::homePath()).toString());
    if (path.isFile())
        return path.absolutePath() + QDir::separator() + className() + ".xml";

    return path.absoluteFilePath() + QDir::separator() + className() + ".xml";
}

QString CreateClassDlg::picturesPathDefault()
{
    QFileInfo path(m_settings->value(LAST_OPENED_PROJECT_PATH,
                                     QDir::homePath()).toString());
    if (path.isFile())
        return path.absolutePath();

    return path.absoluteFilePath();
}

void CreateClassDlg::autoChangeProjectPath()
{
    if (!m_projectPathBlockAutoChange) {
        QFileInfo fi(picturesPath(), className() + ".xml");
        ui->txtProjectPath->setText(fi.path() + QDir::separator() + fi.fileName());
    }
}
