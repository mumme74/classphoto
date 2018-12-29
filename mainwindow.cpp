#undef  QT_NO_PRINTER
#include <QPrinter>
#include <QtWidgets>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinterInfo>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "project.h"
#include "settings.h"
#include "editshowclassname.h"
#include "createclassdlg.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    project = new Project(this, ui->listView, ui->graphicsView);

    ui->editClassName->init(project);


    connectActions();
    ui->actionCloseClass->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionPrint->setEnabled(false);
    ui->actionPrintPdf->setEnabled(false);
    ui->actionPrintPreview->setEnabled(false);
    ui->actionToJpg->setEnabled(false);

    // hide these untill i find some time to implement
    ui->actionRedo->setVisible(false);
    ui->actionUndo->setVisible(false);


    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    settings = settingsFactory("MainWindow", this);

    if (!settings->value(LAST_OPENED_PROJECT_PATH).toString().isEmpty()) {
        project->openProjectFile(settings->value(LAST_OPENED_PROJECT_PATH).toString());
    }

    resize(settings->value("size", QSize(800, 400)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    ui->splitter->restoreState(settings->value("splitterSizes").toByteArray());
    project->setSnapToGrid(settings->value("snapToGrid", true).toBool());
    ui->actionSnapToGrid->setChecked(project->snapToGrid());

}

MainWindow::~MainWindow()
{
    settings->setValue("snapToGrid", project->snapToGrid());
    delete ui;

    delete project;

    settings->endGroup();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::connectActions()
{
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(onAboutQt()));
    connect(ui->actionAboutApplication, SIGNAL(triggered()), this, SLOT(onAboutApplication()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpenProject()));
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(onNewProject()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAs()));
    connect(ui->actionCloseClass, SIGNAL(triggered()), this, SLOT(onCloseClass()));
    connect(ui->actionClose, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionSnapToGrid, SIGNAL(toggled(bool)), project, SLOT(setSnapToGrid(bool)));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(onPrint()));
    connect(ui->actionPrintPdf, SIGNAL(triggered()), this, SLOT(onPrintPdf()));
    connect(ui->actionPrintPreview, SIGNAL(triggered()), this, SLOT(onPrintPreview()));
    connect(ui->actionToJpg, SIGNAL(triggered()), this, SLOT(onExportToJpg()));


    connect(project, SIGNAL(unSavedChanges(bool)), ui->actionSave, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionSaveAs, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionCloseClass, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionPrint, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionPrintPdf, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionPrintPreview, SLOT(setEnabled(bool)));
    connect(project, SIGNAL(openState(bool)), ui->actionToJpg, SLOT(setEnabled(bool)));

}



bool MainWindow::checkProjectOpen()
{
    if (project->isOpened()) {
        if (QMessageBox::Yes == QMessageBox::question(this, trUtf8("Vad vill du göra?"),
                                  trUtf8("Vill du stänga klassen %1?").arg(project->className()),
                                  QMessageBox::Yes, QMessageBox::No))
        {
            //close current project
            return project->saveProject();
        }

        // dont want to close
        return false;
    }
    return true;
}



// ---------------------------------------- event slots go down here --------------------------------

void MainWindow::closeEvent(QCloseEvent *)
{
    if (project->hasChanges() &&
        QMessageBox::Yes == QMessageBox::question(this, trUtf8("Spara?"),
                                                  trUtf8("Vill du spara ändringar?"),
                                                  QMessageBox::Yes, QMessageBox::No))
    {
        project->saveProject();
    }

    settings->setValue("size", size());
    settings->setValue("pos", pos());
    settings->setValue("splitterSizes", ui->splitter->saveState());
}


void MainWindow::onOpenProject()
{
    if (checkProjectOpen()) {
        QString projectPath = settings->value(LAST_OPENED_PROJECT_PATH, QDir::homePath()).toString();
        projectPath = QFileDialog::getOpenFileName(this, trUtf8("välj projektfil"),
                                                   projectPath, tr("klassfoto filer (*.xml)"));

        if (!projectPath.isEmpty()) {
            project->openProjectFile(projectPath);
            settings->setValue(LAST_OPENED_PROJECT_PATH, projectPath);
        }
    }
}

void MainWindow::onNewProject()
{
    if (checkProjectOpen()) {
        CreateClassDlg createDlg(settings);
        if (createDlg.exec() == QDialog::Accepted) {
            QString projectPath = createDlg.projectPath();
            project->newProject(projectPath);
            settings->setValue(LAST_OPENED_PROJECT_PATH, projectPath);
        }
    }
}

void MainWindow::onAboutApplication()
{
    QMessageBox::about(this, trUtf8("Om klassfoto"),
                    trUtf8("<h2>klassfoto 0.1</h2>"
                      "<p>Copyright &copy; 2010 Fredrik Johansson.</p>"
                      "<p>Klassfoto är ett enkelt litet program tänkt"
                      " att förenkla livet för en klassmentor när det är"
                      " dags för att göra en fotolista, så att man snabbt"
                      " kan lära sig namnen på de nya eleverna.</p>"
                      "<p>Detta program skapade jag i syfte att lära mig"
                      " lite C++ programmering, licensen för detta program är GPL</p>"));
}

void MainWindow::onAboutQt()
{
    qApp->aboutQt();
}


void MainWindow::onSave()
{
    if (project->isOpened())
        project->saveProject();
}

void MainWindow::onSaveAs()
{
    if (checkProjectOpen()) {
        QFileInfo path(settings->value(LAST_OPENED_PROJECT_PATH,
                                                          QDir::homePath()).toString());
        QString projectPath;
        if (path.isFile())
            projectPath = path.absolutePath();
        else
            projectPath = path.absoluteFilePath();


        projectPath = QFileDialog::getSaveFileName(this, trUtf8("Nytt namn på projektet"), projectPath + "/nytt_klass_foto.xml", tr("klassfoto filer (*.xml)"));
        if (projectPath.right(4).toLower() != ".xml") {
            projectPath += ".xml";
        }

        project->saveProjectToPath(projectPath);
    }
}

void MainWindow::onCloseClass()
{
    if (project->hasChanges() &&
        QMessageBox::Yes == QMessageBox::question(this, trUtf8("Spara?"),
                                                  trUtf8("Vill du spara ändringar?"),
                                                  QMessageBox::Yes, QMessageBox::No))
    {
        project->saveProject();
    }

    project->closeProject();

    settings->remove(LAST_OPENED_PROJECT_PATH);
}

void MainWindow::printOnPrinterObject(QPrinter *printer)
{
    //QRect printerRect(printer->pageRect());
    qreal left, top, right, bottom;
    printer->getPageMargins(&left, &top, &right, &bottom, QPrinter::DevicePixel);


    QPainter painter(printer);
    QRectF pageRect = printer->pageRect();
    pageRect.setX(pageRect.x() + left);
    pageRect.setY(pageRect.y() + top);
    pageRect.setWidth(pageRect.width() - right);
    pageRect.setHeight(pageRect.height() - bottom);

    if (!printer->isValid()) {
        QMessageBox::warning(this, trUtf8("Fel vid utskrift"), trUtf8("Kunde inte skriva ut dokumentet "));
        printer->abort();
        return;
    }

    // try to center content
    QRect viewRect = ui->graphicsView->scene()->itemsBoundingRect().toRect(); //ui->graphicsView->viewport()->rect();
    //int minSide = qMin(viewRect.width(), viewRect.height());
    //int maxSide = qMax(viewRect.width(), viewRect.height());

    //qreal aspectRatio = static_cast<qreal>(maxSide) /
    //                     static_cast<qreal>(minSide);

    qreal factor = static_cast<qreal>(qMin(pageRect.width(), pageRect.height())) /
                   static_cast<qreal>(qMin(viewRect.width(), viewRect.height()));

    if ((viewRect.width() * factor + 10/*some space*/) < pageRect.width()) {
        int space = static_cast<int>(pageRect.width() - viewRect.width() * factor);
        pageRect.setX(pageRect.x() + (space / 2));
    }

    if ((viewRect.height() * factor + 10) < pageRect.height()) {
        int space = static_cast<int>(pageRect.height());
        pageRect.setY(pageRect.y() + (space / 2));
    }

    foreach (QGraphicsItem *item, ui->graphicsView->scene()->selectedItems()) {
        item->setSelected(false);
    }

    ui->graphicsView->scene()->render(&painter, pageRect, viewRect);

    painter.end();
}

void MainWindow::onPrintPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    printer.setPageMargins(20.0, 20.0, 20.0, 20.0, QPrinter::Millimeter);
    printer.setFullPage(true);

    printer.setPrinterName(settings->value("lastChoosenPrinter", QPrinterInfo::defaultPrinter().printerName()).toString());

    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
    dialog->setWindowTitle(trUtf8("Förhandsgranskning av %1").arg(project->className()));

    connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printOnPrinterObject(QPrinter*)));

    dialog->exec();

}

void MainWindow::onPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    QString printerName = settings->value("lastChoosenPrinter", QPrinterInfo::defaultPrinter().printerName()).toString();
    printer.setPrinterName(printerName);
    printer.setPageMargins(20.0, 20.0, 20.0, 20.0, QPrinter::Millimeter);

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(trUtf8("Skriv ut %1").arg(project->className()));

    if (dialog->exec() != QDialog::Accepted)
        return;

    printerName = printer.printerName();
    settings->setValue("lastChoosenPrinter", printerName);

    printOnPrinterObject(&printer);

}

void MainWindow::onPrintPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("Spara bild som"),
                                                    settings->value("lastSavedPdfPath",
                                                                    QDir::homePath()).toString()
                                                                    + '/' + project->className() + ".pdf",
                                                    "pdf filer (*.pdf)");
    if (!fileName.isEmpty()) {
        settings->setValue("lastSavedPdfPath", QFileInfo(fileName).absolutePath());

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFileName(fileName);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPaperSize(QPrinter::A4);
        printer.setOrientation(QPrinter::Landscape);

        printOnPrinterObject(&printer);

    }
}

void MainWindow::onExportToJpg()
{

    QString defaultPath = QDir::homePath();

    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("Spara bild som"),
                                                    settings->value("lastSavedPicturePath",
                                                                    defaultPath).toString()
                                                    + '/' + project->className() + ".jpg",
                                                    "jpg filer (*.jpg)");
    if (!fileName.isEmpty()) {
        settings->setValue("lastSavedPicturePath", QFileInfo(fileName).absolutePath());

        QImage img(2400, 1800, QImage::Format_RGB32);
        img.fill(0xFFFFFF);
        QPainter painter(&img);


        ui->graphicsView->render(&painter, QRect(GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN,
                                                          GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN,
                                                          static_cast<int>(img.width() - GRAPHICSVIEW_PIXMAP_HORIZONTAL_MARGIN * 4),
                                                          static_cast<int>(img.height() - GRAPHICSVIEW_PIXMAP_VERTICAL_MARGIN * 4)));

        img.save(fileName, "JPG", 100);
    }
}
