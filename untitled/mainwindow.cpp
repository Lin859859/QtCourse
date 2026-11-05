#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 绑定编辑菜单自带功能：撤销、重做、剪切、复制、粘贴、全选
    connect(ui->actionUndo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::selectAll);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 新建
void MainWindow::on_actionNew_triggered()
{
    ui->plainTextEdit->clear();
    this->setWindowTitle("无标题 - 文本编辑器");
}

// 打开文件
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,"打开文件",".","文本文件(*.txt);;所有文件(*.*)");
    if(fileName.isEmpty()) return;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"警告","文件打开失败！");
        return;
    }
    QTextStream in(&file);
    ui->plainTextEdit->setPlainText(in.readAll());
    file.close();
    this->setWindowTitle(fileName);
}

// 保存
void MainWindow::on_actionSave_triggered()
{
    QString fileName = this->windowTitle();
    if(fileName == "无标题 - 文本编辑器"){
        on_actionSaveAs_triggered();
        return;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"警告","保存失败");
        return;
    }
    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
}

// 另存为
void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,"另存为",".","文本文件(*.txt);;所有文件(*.*)");
    if(fileName.isEmpty()) return;
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"警告","另存为失败");
        return;
    }
    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
    this->setWindowTitle(fileName);
}
