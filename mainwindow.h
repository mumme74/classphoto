#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class Project;
class QSettings;
class QGraphicsProxyWidget;
class QPrinter;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool autoPlace() const { return m_autoPlace; }

public slots:
    void printOnPrinterObject(QPrinter *printer);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *);

private slots:
    void onAboutApplication();
    void onAboutQt();
    void onOpenProject();
    void onNewProject();
    void onSave();
    void onSaveAs();
    void onCloseClass();
    void onPrintPreview();
    void onPrint();
    void onPrintPdf();
    void onExportToJpg();

private:
    void connectActions();
    bool checkProjectOpen();
    void rebuildListView();
    void rebuildGraphicsView();
    void rebuild();

    Ui::MainWindow *ui;

    Project *project;
    bool m_autoPlace;

    QSettings *settings;
};

#endif // MAINWINDOW_H
