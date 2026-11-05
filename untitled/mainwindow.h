#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();

    void on_actionWrap_triggered();
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionBgColor_triggered();
    void on_actionEditBg_triggered();


    void updateStatusInfo();
    void onTextChanged();
    void updateEditActionState();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QString m_filePath;
    bool m_isModified;
    void updateWindowTitle();
};
#endif // MAINWINDOW_H
