#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 行号区域类
class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(QPlainTextEdit *edit, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPlainTextEdit *m_edit;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 查找、替换
    void on_action_F_triggered();
    void on_action_R_triggered();
    // 自动换行、行号
    void on_actionWrap_triggered();
    void on_actionLineNum_triggered();
    // 工具栏、状态栏
    void on_actionToolBar_triggered();
    void on_actionStatusBar_triggered();

    // 文件
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_action_X_triggered();

    // 编辑
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();

    // 格式
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionBgColor_triggered();
    void on_actionEditBg_triggered();

    // 关于
    void on_actionAbout_triggered();

    // 行号更新槽
    void updateLineNumberWidth();
    void updateLineNumbers();



private:
    Ui::MainWindow *ui;
    LineNumberArea *lineNumberWidget;
    QString currentFilePath; // 当前打开文件路径
};

#endif // MAINWINDOW_H
