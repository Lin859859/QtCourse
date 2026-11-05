#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_filePath = "";
    m_isModified = false;

    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusInfo);
    connect(ui->plainTextEdit, &QPlainTextEdit::selectionChanged, this, &MainWindow::updateEditActionState);

    updateWindowTitle();
    updateStatusInfo();
    updateEditActionState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateWindowTitle()
{
    QString title = "简易记事本";
    if (!m_filePath.isEmpty())
        title = m_filePath;
    if (m_isModified)
        title = "* " + title;
    setWindowTitle(title);
}

void MainWindow::onTextChanged()
{
    m_isModified = true;
    updateWindowTitle();
}

void MainWindow::updateStatusInfo()
{
    auto cursor = ui->plainTextEdit->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock() + 1;
    statusBar()->showMessage(QString("行：%1  列：%2").arg(line).arg(col));
}

void MainWindow::updateEditActionState()
{
    bool hasSelection = ui->plainTextEdit->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    QClipboard *clip = QApplication::clipboard();
    ui->actionPaste->setEnabled(!clip->text().isEmpty());
}

void MainWindow::on_actionNew_triggered()
{
    if (m_isModified)
    {
        auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
            on_actionSave_triggered();
        else if (ret == QMessageBox::Cancel)
            return;
    }
    ui->plainTextEdit->clear();
    m_filePath = "";
    m_isModified = false;
    updateWindowTitle();
}

void MainWindow::on_actionOpen_triggered()
{
    if (m_isModified)
    {
        auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
            on_actionSave_triggered();
        else if (ret == QMessageBox::Cancel)
            return;
    }

    QString path = QFileDialog::getOpenFileName(this, "打开文件", "", "文本文件(*.txt);;所有文件(*.*)");
    if (path.isEmpty()) return;

    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "错误", "打开失败：" + f.errorString());
        return;
    }
    QTextStream in(&f);
    ui->plainTextEdit->setPlainText(in.readAll());
    f.close();

    m_filePath = path;
    m_isModified = false;
    updateWindowTitle();
}

bool saveFile(const QString &path, const QString &content)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text))
        return false;
    QTextStream out(&f);
    out << content;
    f.close();
    return true;
}

void MainWindow::on_actionSave_triggered()
{
    if (m_filePath.isEmpty())
    {
        on_actionSaveAs_triggered();
        return;
    }
    bool ok = saveFile(m_filePath, ui->plainTextEdit->toPlainText());
    if (!ok)
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件");
        return;
    }
    m_isModified = false;
    updateWindowTitle();
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", "", "文本文件(*.txt);;所有文件(*.*)");
    if (path.isEmpty()) return;
    bool ok = saveFile(path, ui->plainTextEdit->toPlainText());
    if (!ok)
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件");
        return;
    }
    m_filePath = path;
    m_isModified = false;
    updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isModified)
    {
        event->accept();
        return;
    }
    auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (ret == QMessageBox::Yes)
    {
        on_actionSave_triggered();
        event->accept();
    }
    else if (ret == QMessageBox::No)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

// ===== 编辑功能槽函数 =====
void MainWindow::on_actionUndo_triggered()
{
    ui->plainTextEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->plainTextEdit->redo();
}

void MainWindow::on_actionCut_triggered()
{
    ui->plainTextEdit->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    ui->plainTextEdit->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->plainTextEdit->paste();
}

void MainWindow::on_actionSelectAll_triggered()
{
    ui->plainTextEdit->selectAll();
}
