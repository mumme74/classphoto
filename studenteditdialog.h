#ifndef STUDENTEDITDIALOG_H
#define STUDENTEDITDIALOG_H

#include <QDialog>

class Project;
class QGraphicsPixmapItem;
class Picture;
class CropPicture;

namespace Ui {
    class StudentEditDialog;
}

class StudentEditDialog : public QDialog {
    Q_OBJECT
public:
    StudentEditDialog(Project *project, const QString key, QWidget *parent = 0);
    ~StudentEditDialog();
    void accept();




protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *);

private slots:
    void onRotationSpinBoxChange();
    void onRotationChange();
    void onBrightnessChange();
    void onScaleChange();
    //void doBrightness();

private:
    void rotate();

    Ui::StudentEditDialog *ui;
    Project *m_project;
    //int m_rotation;
    //int m_scale;
    //QGraphicsPixmapItem *m_pixmapItem;
    Picture *m_pic;
    //QTimer *m_brightnessTimer;
    CropPicture *m_crop;

    QString m_key;

};

#endif // STUDENTEDITDIALOG_H
