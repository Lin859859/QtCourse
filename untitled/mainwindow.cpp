#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QPainter>
#include <QTextBlock>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QTextCursor>


// ========== LineNumberArea 实现 ==========
LineNumberArea::LineNumberArea(QPlainTextEdit *edit, QWidget *parent)
    : QWidget(parent), m_edit(edit)
{
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#f0f0f0"));

    QTextBlock block = m_edit->document()->firstBlock();
    int blockNo = block.blockNumber();
    qreal top = m_edit->document()->documentLayout()->blockBoundingRect(block).top();
    qreal bottom;
    QRect viewRect = m_edit->viewport()->rect();

    while (block.isValid())
    {
        bottom = m_edit->document()->documentLayout()->blockBoundingRect(block).bottom();
        if (bottom > viewRect.bottom())
            break;
        if (top >= viewRect.top())
        {
            painter.drawText(0, top, width()-4, m_edit->fontMetrics().height(),
                             Qt::AlignRight, QString::number(blockNo+1));
        }
        block = block.next();
        blockNo = block.blockNumber();
        top = bottom;
    }
    QWidget::paintEvent(event);
}

// ========== MainWindow ==========
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化状态
    ui->actionStatusBar->setChecked(true);
    statusBar()->show();
    ui->actionToolBar->setChecked(true);
    ui->toolBar->show();

    // 工具栏控制
    connect(ui->actionToolBar, &QAction::toggled, this, [=](bool checked){
        ui->toolBar->setVisible(checked);
    });
    // 状态栏控制（修复点：show/hide）
    connect(ui->actionStatusBar, &QAction::toggled, this, [=](bool checked){
        if(checked)
        {
            statusBar()->show();
        }
        else
        {
            statusBar()->hide();
        }
    });

    // 状态栏文字更新
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, [=](){
        QTextCursor cur = ui->plainTextEdit->textCursor();
        int line = cur.blockNumber() + 1;
        int col  = cur.columnNumber() + 1;
        int totalLen = ui->plainTextEdit->toPlainText().size();
        int totalLines = ui->plainTextEdit->blockCount();
        statusBar()->showMessage(QString("length: %1   lines: %2   Ln: %3  Col: %4")
                                     .arg(totalLen).arg(totalLines).arg(line).arg(col));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


// ======== 行号辅助函数 ========
void MainWindow::updateLineNumberWidth()
{
    int digits = QString::number(ui->plainTextEdit->blockCount()).length();
    int width = 8 + ui->plainTextEdit->fontMetrics().horizontalAdvance('0') * digits;
    if(ui->actionLineNum->isChecked())
        lineNumberWidget->setFixedWidth(width);
    else
        lineNumberWidget->setFixedWidth(0);
}

void MainWindow::updateLineNumbers()
{
    QRect rect = ui->plainTextEdit->viewport()->rect();
    lineNumberWidget->setGeometry(0, rect.top(), lineNumberWidget->width(), rect.height());
    lineNumberWidget->update();
}

// ======== 自动换行 ========
void MainWindow::on_actionWrap_triggered()
{
    bool checked = ui->actionWrap->isChecked();
    if(checked)
    {
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    }
    else
    {
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
}

// ======== 显示行号开关 ========
void MainWindow::on_actionLineNum_triggered()
{
    updateLineNumberWidth();
    updateLineNumbers();
}

// ======== 查找 action_F (Ctrl+F) ========
void MainWindow::on_action_F_triggered()
{
    QDialog dlg(this);
    dlg.setWindowTitle("查找");
    dlg.resize(420, 180);

    QVBoxLayout *mainLay = new QVBoxLayout(&dlg);
    mainLay->setContentsMargins(20, 20, 20, 20);
    mainLay->setSpacing(10);

    // 查找目标行
    QHBoxLayout *layFind = new QHBoxLayout;
    QLabel *labFind = new QLabel("查找目标：");
    QLineEdit *edtFind = new QLineEdit();
    QPushButton *btnNext = new QPushButton("查找下一个(&F)");
    layFind->addWidget(labFind);
    layFind->addWidget(edtFind, 1);
    layFind->addWidget(btnNext);

    // 方向和区分大小写
    QHBoxLayout *layOption = new QHBoxLayout;

    QLabel *labDir = new QLabel("方向：");
    QRadioButton *radUp = new QRadioButton("向上(&U)");
    QRadioButton *radDown = new QRadioButton("向下(&D)");
    radDown->setChecked(true);

    QButtonGroup *dirGroup = new QButtonGroup(this);
    dirGroup->addButton(radUp);
    dirGroup->addButton(radDown);

    QCheckBox *chkCase = new QCheckBox("区分大小写(&C)");

    layOption->addWidget(labDir);
    layOption->addWidget(radUp);
    layOption->addWidget(radDown);
    layOption->addStretch();
    layOption->addWidget(chkCase);

    // 取消按钮
    QHBoxLayout *layBtn = new QHBoxLayout;
    layBtn->addStretch();
    QPushButton *btnCancel = new QPushButton("取消");
    layBtn->addWidget(btnCancel);

    mainLay->addLayout(layFind);
    mainLay->addLayout(layOption);
    mainLay->addLayout(layBtn);

    connect(btnNext, &QPushButton::clicked, this, [=](){
        QString target = edtFind->text();
        if(target.isEmpty()){
            QMessageBox::information(this, "提示", "请输入查找内容");
            return;
        }

        QTextDocument::FindFlags flags;
        if(chkCase->isChecked()){
            flags |= QTextDocument::FindCaseSensitively;
        }
        if(radUp->isChecked()){
            flags |= QTextDocument::FindBackward;
        }

        bool ok = ui->plainTextEdit->find(target, flags);
        if(!ok){
            QMessageBox::information(this, "提示", "没有找到");
        }
    });

    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    dlg.exec();
}


// ======== 替换 action_R (Ctrl+H) ========
void MainWindow::on_action_R_triggered()
{
    QDialog dlg(this);
    dlg.setWindowTitle("替换");
    dlg.resize(420, 230);

    QVBoxLayout *mainLay = new QVBoxLayout(&dlg);
    mainLay->setContentsMargins(20, 20, 20, 20);
    mainLay->setSpacing(10);

    // 查找目标
    QHBoxLayout *layFind = new QHBoxLayout;
    QLabel *labFind = new QLabel("查找目标：");
    QLineEdit *edtFind = new QLineEdit();
    QPushButton *btnNext = new QPushButton("查找下一个(&F)");
    layFind->addWidget(labFind);
    layFind->addWidget(edtFind, 1);
    layFind->addWidget(btnNext);

    // 替换为
    QHBoxLayout *layRep = new QHBoxLayout;
    QLabel *labRep = new QLabel("替换为：");
    QLineEdit *edtRep = new QLineEdit();
    layRep->addWidget(labRep);
    layRep->addWidget(edtRep, 1);

    // 方向和选项
    QHBoxLayout *layOption = new QHBoxLayout;

    QLabel *labDir = new QLabel("方向：");
    QRadioButton *radUp = new QRadioButton("向上(&U)");
    QRadioButton *radDown = new QRadioButton("向下(&D)");
    radDown->setChecked(true);

    QButtonGroup *dirGroup = new QButtonGroup(this);
    dirGroup->addButton(radUp);
    dirGroup->addButton(radDown);

    QCheckBox *chkCase = new QCheckBox("区分大小写(&C)");

    layOption->addWidget(labDir);
    layOption->addWidget(radUp);
    layOption->addWidget(radDown);
    layOption->addStretch();
    layOption->addWidget(chkCase);

    // 按钮：替换、全部替换、取消
    QHBoxLayout *layBtn = new QHBoxLayout;
    layBtn->addStretch();

    QPushButton *btnRep = new QPushButton("替换(&R)");
    QPushButton *btnAll = new QPushButton("全部替换(&A)");
    QPushButton *btnCancel = new QPushButton("取消");

    layBtn->addWidget(btnRep);
    layBtn->addWidget(btnAll);
    layBtn->addWidget(btnCancel);

    mainLay->addLayout(layFind);
    mainLay->addLayout(layRep);
    mainLay->addLayout(layOption);
    mainLay->addLayout(layBtn);

    // 查找下一个
    connect(btnNext, &QPushButton::clicked, this, [=](){
        QString target = edtFind->text();
        if(target.isEmpty()){
            QMessageBox::information(this, "提示", "请输入查找内容");
            return;
        }

        QTextDocument::FindFlags flags;
        if(chkCase->isChecked()){
            flags |= QTextDocument::FindCaseSensitively;
        }
        if(radUp->isChecked()){
            flags |= QTextDocument::FindBackward;
        }

        bool ok = ui->plainTextEdit->find(target, flags);
        if(!ok){
            QMessageBox::information(this, "提示", "没有找到");
        }
    });

    // 替换当前找到的一处
    connect(btnRep, &QPushButton::clicked, this, [=](){
        QString target = edtFind->text();
        QString replace = edtRep->text();

        if(target.isEmpty()){
            QMessageBox::information(this, "提示", "请输入查找内容");
            return;
        }

        QTextCursor cur = ui->plainTextEdit->textCursor();
        if(cur.hasSelection() && cur.selectedText() == target){
            cur.insertText(replace);
            ui->plainTextEdit->setTextCursor(cur);

            QTextDocument::FindFlags flags;
            if(chkCase->isChecked()){
                flags |= QTextDocument::FindCaseSensitively;
            }
            if(radUp->isChecked()){
                flags |= QTextDocument::FindBackward;
            }
            ui->plainTextEdit->find(target, flags);
        }else{
            QMessageBox::information(this, "提示", "没有可替换的匹配项，请先点击“查找下一个”");
        }
    });

    // 全部替换
    connect(btnAll, &QPushButton::clicked, this, [=](){
        QString target = edtFind->text();
        QString replace = edtRep->text();

        if(target.isEmpty()){
            QMessageBox::information(this, "提示", "请输入查找内容");
            return;
        }

        QString allText = ui->plainTextEdit->toPlainText();
        int count = 0;

        if(chkCase->isChecked()){
            int pos = 0;
            while((pos = allText.indexOf(target, pos)) != -1){
                allText.replace(pos, target.length(), replace);
                pos += replace.length();
                count++;
            }
        }else{
            int pos = 0;
            while((pos = allText.indexOf(target, pos, Qt::CaseInsensitive)) != -1){
                allText.replace(pos, target.length(), replace);
                pos += replace.length();
                count++;
            }
        }

        ui->plainTextEdit->setPlainText(allText);

        if(count > 0){
            QMessageBox::information(this, "完成", QString("已替换 %1 处").arg(count));
        }else{
            QMessageBox::information(this, "提示", "没有找到匹配内容");
        }
    });

    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    dlg.exec();
}


// ======== 工具栏、状态栏 ========
void MainWindow::on_actionToolBar_triggered()
{
    ui->toolBar->setVisible(ui->actionToolBar->isChecked());
}
void MainWindow::on_actionStatusBar_triggered()
{
    ui->statusbar->setVisible(ui->actionStatusBar->isChecked());
}

// ======== 文件菜单 ========
void MainWindow::on_actionNew_triggered()
{
    ui->plainTextEdit->clear();
    currentFilePath.clear();
    this->setWindowTitle("简易记事本 - 新建文档");
}
void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(this,"打开文件","","文本文件(*.txt);;所有文件(*.*)");
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"错误","打开失败");
        return;
    }
    QTextStream st(&f);
    ui->plainTextEdit->setPlainText(st.readAll());
    f.close();
    currentFilePath = path;
    this->setWindowTitle("简易记事本 - " + path);
}
void MainWindow::on_actionSave_triggered()
{
    if(currentFilePath.isEmpty())
    {
        on_actionSaveAs_triggered();
        return;
    }
    QFile f(currentFilePath);
    if(!f.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,"错误","保存失败");
        return;
    }
    QTextStream st(&f);
    st << ui->plainTextEdit->toPlainText();
    f.close();
    QMessageBox::information(this,"提示","保存成功");
}
void MainWindow::on_actionSaveAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this,"另存为","","文本文件(*.txt);;所有文件(*.*)");
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,"错误","保存失败");
        return;
    }
    QTextStream st(&f);
    st << ui->plainTextEdit->toPlainText();
    f.close();
    currentFilePath = path;
    this->setWindowTitle("简易记事本 - " + path);
    QMessageBox::information(this,"提示","另存成功");
}
void MainWindow::on_action_X_triggered()
{
    close();
}

// ======== 编辑菜单 ========
void MainWindow::on_actionUndo_triggered(){ ui->plainTextEdit->undo(); }
void MainWindow::on_actionRedo_triggered(){ ui->plainTextEdit->redo(); }
void MainWindow::on_actionCut_triggered(){ ui->plainTextEdit->cut(); }
void MainWindow::on_actionCopy_triggered(){ ui->plainTextEdit->copy(); }
void MainWindow::on_actionPaste_triggered(){ ui->plainTextEdit->paste(); }
void MainWindow::on_actionSelectAll_triggered(){ ui->plainTextEdit->selectAll(); }

// ======== 格式菜单 ========
void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, ui->plainTextEdit->font(), this, "选择字体");
    if(ok) ui->plainTextEdit->setFont(f);
}
// 修改字体颜色
void MainWindow::on_actionFontColor_triggered()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    QTextCharFormat fmt = cursor.charFormat();
    QColor c = QColorDialog::getColor(fmt.foreground().color(), this, "字体颜色");
    if(c.isValid())
    {
        fmt.setForeground(c);
        cursor.setCharFormat(fmt);
        ui->plainTextEdit->setCurrentCharFormat(fmt);
    }
}

// 修改选中文字背景色（文字高亮底色）
void MainWindow::on_actionBgColor_triggered()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    QTextCharFormat fmt = cursor.charFormat();
    QColor c = QColorDialog::getColor(fmt.background().color(), this, "文字背景色");
    if(c.isValid())
    {
        fmt.setBackground(c);
        cursor.setCharFormat(fmt);
        ui->plainTextEdit->setCurrentCharFormat(fmt);
    }
}

// 修改整个编辑器的背景（底色）


void MainWindow::on_actionEditBg_triggered()
{
    QColor c = QColorDialog::getColor(ui->plainTextEdit->palette().color(QPalette::Base), this, "编辑器背景");
    if(c.isValid())
    {
        QPalette pal = ui->plainTextEdit->palette();
        pal.setColor(QPalette::Base, c);
        ui->plainTextEdit->setPalette(pal);
    }
}

// ======== 关于 ========
void MainWindow::on_actionAbout_triggered()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Dialog");
    dlg.resize(420, 300);

    QVBoxLayout *mainLay = new QVBoxLayout(&dlg);
    mainLay->setContentsMargins(30, 30, 30, 30);
    mainLay->setSpacing(15);

    // 标题行：图标 + 标题
    QHBoxLayout *titleLay = new QHBoxLayout;
    QLabel *iconLab = new QLabel;
    iconLab->setPixmap(QPixmap(":/icons/notebook.png")); // 没有图标可先删掉这行
    QLabel *titleLab = new QLabel("小牛文本编辑");
    titleLab->setFont(QFont("微软雅黑", 24, QFont::Bold));
    titleLay->addWidget(iconLab);
    titleLay->addWidget(titleLab);
    titleLay->addStretch();

    // 信息文本
    QLabel *infoLab = new QLabel(
        "开发者：林泳恩<br>"
        "学号：2024414290219<br>"
        "email：linyongen2024@example.com"
        );
    infoLab->setFont(QFont("微软雅黑", 11));

    // 底部：版本号 + OK按钮
    QHBoxLayout *bottomLay = new QHBoxLayout;
    QLabel *verLab = new QLabel("V1.0 build 20241108");
    QPushButton *btnOk = new QPushButton("OK");
    btnOk->setMinimumWidth(80);
    bottomLay->addWidget(verLab);
    bottomLay->addStretch();
    bottomLay->addWidget(btnOk);

    mainLay->addLayout(titleLay);
    mainLay->addWidget(infoLab);
    mainLay->addStretch();
    mainLay->addLayout(bottomLay);

    connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

