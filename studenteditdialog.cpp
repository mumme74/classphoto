#include "studenteditdialog.h"
#include "ui_studenteditdialog.h"
#include "croppicture.h"

#include <QGraphicsScene>
#include <QDebug>
#include <math.h>

#include "project.h"
#include "picture.h"

StudentEditDialog::StudentEditDialog(Project *project, const QString key, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudentEditDialog),
    m_project(project),
    m_pic(0),
    m_crop(0),
    m_key(key)
{
    ui->setupUi(this);

    m_pic = m_project->pictureForKey(key);
    ui->brightnessSlider->setValue(static_cast<int>(m_pic->brightness() * 100));
    qreal scaleFactor = (m_pic->scaleFactor() - 1.0) * 100;
    ui->scaleSlider->setValue(static_cast<int>(scaleFactor));

    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(onBrightnessChange()));
    connect(ui->scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(onScaleChange()));
    connect(ui->rotation, SIGNAL(valueChanged(int)), this, SLOT(onRotationChange()));
    connect(ui->rotationSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onRotationSpinBoxChange()));


    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);

    m_crop = new CropPicture(m_pic);
    scene->addItem(m_crop);


    ui->graphicsView->setScene(scene);
    //ui->graphicsView->centerOn(m_pixmapItem);
    //m_crop->update();
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //const QPixmap *pix = m_pic->originalPixmap();


    QString name = m_project->knownName(key);
    if (!name.isEmpty())
        ui->nameEdit->setText(name);


    int rotation = m_pic->rotation() + 180;
    while(rotation > 360) rotation -= 360;
    while(rotation < 0) rotation += 360;
    ui->rotationSpinBox->setValue(rotation);



    m_crop->setBrightness(m_pic->brightness());
    m_crop->doBrightness();
    onScaleChange();
}

StudentEditDialog::~StudentEditDialog()
{
    delete ui;
}

void StudentEditDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void StudentEditDialog::accept()
{
    if (ui->nameEdit->text() != m_project->knownName(m_key))
        m_project->setKnownName(m_key, ui->nameEdit->text());

    bool changedPic = false;

    int rotation = ui->rotationSpinBox->value() - 180;
    while(rotation > 360) rotation -= 360;
    while(rotation < 0) rotation += 360;
    if (rotation != m_pic->rotation()) {
        m_pic->setRotation(rotation);
        changedPic = true;
    }

    qreal brightness = static_cast<qreal>(ui->brightnessSlider->value()) / 100.0;
    if (brightness != m_pic->brightness()) {
        m_pic->setBrightness(brightness);
        changedPic = true;
    }

    qreal scale = 1.0 + (static_cast<qreal>(ui->scaleSlider->value()) / 100.0);
    if (scale != m_pic->scaleFactor()) {
        m_pic->setScale(scale);
        changedPic = true;
    }

    QRect rect = m_crop->cropRect();
    if (m_pic->viewPort() != rect && rect.width() && rect.height()) {
        m_pic->setViewPort(rect);
        changedPic = true;
    }

    QPointF rotationPoint(m_crop->rotationPoint());
    if (m_pic->rotationPoint() != rotationPoint) {
        m_pic->setRotationPoint(rotationPoint);
        changedPic = true;
    }

    if (changedPic)
        m_project->setPictureUpdated(m_key);

    QDialog::accept();
}

void StudentEditDialog::showEvent(QShowEvent *)
{
    //ui->graphicsView->fitInView(m_crop, Qt::KeepAspectRatio);
    qreal scaleFactor = ui->graphicsView->contentsRect().height();
    if (scaleFactor)
       scaleFactor /= static_cast<qreal>(m_crop->boundingRect().height());
    ui->graphicsView->scale(scaleFactor, scaleFactor);
    m_crop->centerView();
    m_crop->setRotationPoint(m_pic->rotationPoint());
    m_crop->update();
}

/*
void StudentEditDialog::rotate()
{
    m_crop->findLargestVisible();

    int rotation = ui->rotationSpinBox->value() - 180;
    while(rotation > 360) rotation -= 360;
    while(rotation < 0) rotation += 360;
    if (static_cast<int>(m_pixmapItem->rotation()) != rotation) {
        m_pixmapItem->setTransformOriginPoint(m_pixmapItem->pixmap().width() / 2.0,
                                              m_pixmapItem->pixmap().height() / 2.0);
        m_pixmapItem->setRotation(rotation);
    }
}
*/

// --------------------------- event slots down here ------------------------
/*void StudentEditDialog::doBrightness()
{
    if (m_brightnessTimer) {
        m_brightnessTimer->stop();
        delete m_brightnessTimer;
        m_brightnessTimer = 0;
    }

    QImage img = m_pic->originalPixmap()->toImage();
    qreal brightness = static_cast<qreal>(ui->brightnessSlider->value()) / 100.0;
    Picture::doBrightness(&img, brightness);
    m_pixmapItem->setPixmap(QPixmap::fromImage(img));
}
*/

void StudentEditDialog::onRotationSpinBoxChange()
{
    int rotation = ui->rotationSpinBox->value();
    ui->rotation->setValue(rotation);
    if (m_crop->rotation() != rotation)
        m_crop->setRotation(rotation);
}

void StudentEditDialog::onBrightnessChange()
{


    qreal brightness = static_cast<qreal>(ui->brightnessSlider->value()) / 100.0;
    m_crop->setBrightness(brightness);

    /*
    if (m_brightnessTimer) {
        if (m_brightnessTimer->isActive()) {
            m_brightnessTimer->stop();
        }
        delete m_brightnessTimer;
        m_brightnessTimer = 0;
    }

    m_brightnessTimer = new QTimer(this);
    connect(m_brightnessTimer, SIGNAL(timeout()), this, SLOT(doBrightness()));
    m_brightnessTimer->start(200);
    */
}

void StudentEditDialog::onRotationChange()
{
    int rotation = ui->rotation->value();
    if (ui->rotationSpinBox->value() != rotation)
        ui->rotationSpinBox->setValue(rotation);

    if (m_crop->rotation() != rotation)
        m_crop->setRotation(rotation);
}

void StudentEditDialog::onScaleChange()
{
    int scale = ui->scaleSlider->value();
    m_crop->setScale(scale);

    /*
    scale -= m_scale;
    m_scale += scale;

    if (scale != 0) {
        qreal scaleFactor = 1.0 + (static_cast<qreal>(scale) / (101.0 - m_scale));
        m_crop->setScale(scaleFactor);
        ui->graphicsView->centerOn(m_pixmapItem);
    }
    */
}
