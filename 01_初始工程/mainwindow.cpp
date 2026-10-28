#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //===== 数字按钮 btnNum0~btnNum9 绑定同一个槽 =====
    connect(ui->btnNum0, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum1, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum2, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum3, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum4, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum5, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum6, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum7, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum8, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum9, &QPushButton::clicked, this, &MainWindow::btnNumClicked);

    //===== 二元运算符 + - * / =====
    connect(ui->btn_add, &QPushButton::clicked, this, &MainWindow::btnBinaryOperatorClicked);
    connect(ui->btn_sub, &QPushButton::clicked, this, &MainWindow::btnBinaryOperatorClicked);
    connect(ui->btn_mul, &QPushButton::clicked, this, &MainWindow::btnBinaryOperatorClicked);
    connect(ui->btn_div, &QPushButton::clicked, this, &MainWindow::btnBinaryOperatorClicked);

    //===== 一元运算符：±、√x、x²、%、倒数1/x =====
    connect(ui->btn_sign, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btn_sqrt, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btn_square, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btn_percent, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btn_reciprocal, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 数字按钮点击
void MainWindow::btnNumClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(!btn) return;
    QString num = btn->text();
    ui->display->setText(ui->display->text() + num);
}

// 二元运算符 + - * /
void MainWindow::btnBinaryOperatorClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(!btn) return;
    QString op = btn->text();

    operands.push(ui->display->text());
    opcodes.push(op);
    ui->display->setText("");
}

// 一元运算符：±  √x  x²  %  1/x
void MainWindow::btnUnaryOperatorClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(!btn) return;
    QString op = btn->text();
    double val = ui->display->text().toDouble();
    double res = val;

    if(op == "±")
    {
        res = -val;
    }
    else if(op == "x²")
    {
        res = val * val;
    }
    else if(op == "√x")
    {
        if(val < 0)
        {
            ui->display->setText("Err");
            return;
        }
        res = sqrt(val);
    }
    else if(op == "%")
    {
        res = val / 100.0;
    }
    else if(op == "1/x") //倒数
    {
        if(val == 0)
        {
            ui->display->setText("Err");
            return;
        }
        res = 1.0 / val;
    }
    ui->display->setText(QString::number(res));
}

// 计算函数
QString MainWindow::calculation(bool *ok)
{
    *ok = true;
    if(operands.size() < 1 || opcodes.size() <1)
    {
        return ui->display->text();
    }
    double b = ui->display->text().toDouble();
    double a = operands.pop().toDouble();
    QString op = opcodes.pop();
    double ret = 0;

    if(op == "+")
    {
        ret = a + b;
    }
    else if(op == "-")
    {
        ret = a - b;
    }
    else if(op == "×" || op == "*")
    {
        ret = a * b;
    }
    else if(op == "÷" || op == "/")
    {
        if(b == 0)
        {
            *ok = false;
            return "Err";
        }
        ret = a / b;
    }
    return QString::number(ret);
}

// 小数点 btn_dot
void MainWindow::on_btn_dot_clicked()
{
    QString s = ui->display->text();
    if(!s.contains("."))
    {
        ui->display->setText(s + ".");
    }
}

// 退格 btn_backspace
void MainWindow::on_btn_backspace_clicked()
{
    QString text = ui->display->text();
    if(text.length() > 0)
    {
        text.chop(1);
        ui->display->setText(text);
    }
}

// C 全部清空：输入框+栈全部重置
void MainWindow::on_btn_c_clicked()
{
    ui->display->setText("");
    operands.clear();
    opcodes.clear();
}

// CE：只清空当前输入框，保留栈里的数据
void MainWindow::on_btn_ce_clicked()
{
    ui->display->setText("");
}

// 等于 btn_equal
void MainWindow::on_btn_equal_clicked()
{
    operands.push(ui->display->text());
    bool ok;
    QString res = calculation(&ok);
    ui->display->setText(res);
    operands.clear();
    opcodes.clear();
}
