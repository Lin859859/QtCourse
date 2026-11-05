#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QDebug>

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

    // 工具栏 官方toggleViewAction方案
    connect(ui->actionToolBar, &QAction::toggled, this, [=](bool checked){
        ui->toolBar->toggleViewAction()->setChecked(checked);
    });
    // 状态栏
    connect(ui->actionStatusBar, &QAction::toggled, this, [=](bool checked){
        statusBar()->setVisible(checked);
    });

    // 初始化
    ui->actionToolBar->setChecked(true);
    ui->toolBar->toggleViewAction()->setChecked(true);
    ui->actionStatusBar->setChecked(true);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateWindowTitle()
{
    QString title = "简易记事本";
    if(!m_filePath.isEmpty()){
        title = m_filePath;
    }
    if(m_isModified){
        title.prepend("* ");
    }
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
    bool hasSelect = ui->plainTextEdit->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelect);
    ui->actionCopy->setEnabled(hasSelect);
}

void MainWindow::on_actionNew_triggered()
{
    if(m_isModified){
        auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        if(ret == QMessageBox::Yes){
            on_actionSave_triggered();
        }
    }
    ui->plainTextEdit->clear();
    m_filePath = "";
    m_isModified = false;
    updateWindowTitle();
}

void MainWindow::on_actionOpen_triggered()
{
    if(m_isModified){
        auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel) return;
        if(ret == QMessageBox::Yes){
            on_actionSave_triggered();
        }
    }
    QString path = QFileDialog::getOpenFileName(this, "打开文件", "", "文本文件 (*.txt);;所有文件 (*)");
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "错误", "打开失败");
        return;
    }
    QTextStream st(&f);
    ui->plainTextEdit->setPlainText(st.readAll());
    f.close();
    m_filePath = path;
    m_isModified = false;
    updateWindowTitle();
}

bool saveFile(const QString &path, const QString &text)
{
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly)) return false;
    QTextStream st(&f);
    st << text;
    f.close();
    return true;
}

void MainWindow::on_actionSave_triggered()
{
    if(m_filePath.isEmpty()){
        on_actionSaveAs_triggered();
        return;
    }
    bool ok = saveFile(m_filePath, ui->plainTextEdit->toPlainText());
    if(ok){
        m_isModified = false;
        updateWindowTitle();
    }else{
        QMessageBox::warning(this, "错误", "保存失败");
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", "", "文本文件 (*.txt);;所有文件 (*)");
    if(path.isEmpty()) return;
    bool ok = saveFile(path, ui->plainTextEdit->toPlainText());
    if(ok){
        m_filePath = path;
        m_isModified = false;
        updateWindowTitle();
    }else{
        QMessageBox::warning(this, "错误", "保存失败");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_isModified){
        auto ret = QMessageBox::question(this, "提示", "文件已修改，是否保存？",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(ret == QMessageBox::Cancel){
            event->ignore();
            return;
        }
        if(ret == QMessageBox::Yes){
            on_actionSave_triggered();
        }
    }
    event->accept();
}

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

void MainWindow::on_actionWrap_triggered()
{
    bool checked = ui->actionWrap->isChecked();
    if(checked){
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    }else{
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, ui->plainTextEdit->font(), this, "选择字体");
    if(ok){
        ui->plainTextEdit->setFont(f);
    }
}

void MainWindow::on_actionFontColor_triggered()
{
    QColor c = QColorDialog::getColor(ui->plainTextEdit->palette().text().color(), this, "字体颜色");
    if(c.isValid()){
        auto pal = ui->plainTextEdit->palette();
        pal.setColor(QPalette::Text, c);
        ui->plainTextEdit->setPalette(pal);
    }
}

void MainWindow::on_actionBgColor_triggered()
{
    QColor c = QColorDialog::getColor(Qt::yellow, this, "选中高亮颜色");
    if(c.isValid()){
        ui->plainTextEdit->setStyleSheet(QString("QPlainTextEdit{ selection-background-color:%1; }").arg(c.name()));
    }
}

void MainWindow::on_actionEditBg_triggered()
{
    QColor c = QColorDialog::getColor(Qt::white, this, "编辑器背景");
    if(c.isValid()){
        ui->plainTextEdit->setStyleSheet(QString("QPlainTextEdit{ background-color:%1; }").arg(c.name()));
    }
}



