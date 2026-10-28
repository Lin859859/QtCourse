// 边界用例自动测试：直接驱动真实的 MainWindow（鼠标用 click()，键盘用 QKeyEvent）
// 编译方式见 _work_lab1/tests 说明
#include "mainwindow.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <cstdio>

static MainWindow *gWin = nullptr;
static QLineEdit  *gDisplay = nullptr;
static QLabel     *gExpression = nullptr;
static int gCase = 0;
static int gFail = 0;
static QString gSeq;

static void mouse(const char *objectName)
{
    QPushButton *button = gWin->findChild<QPushButton *>(QString::fromLatin1(objectName));
    if (!button) {
        gSeq += QStringLiteral("[缺少按钮%1]").arg(QString::fromLatin1(objectName));
        ++gFail;
        return;
    }
    gSeq += QStringLiteral("鼠标[%1] ").arg(QString::fromLatin1(objectName));
    button->click();
}

static void key(int code, const QString &text)
{
    gSeq += QStringLiteral("键盘[%1] ").arg(text.isEmpty() ? QStringLiteral("?") : text);
    QKeyEvent event(QEvent::KeyPress, code, Qt::NoModifier, text);
    QApplication::sendEvent(gWin, &event);
}

static void keyChar(const QString &text) { key(0, text); }
static void press(int code)              { key(code, QString()); }

static void checkValue(const char *title, const QString &actual, const QString &expected)
{
    ++gCase;
    const bool ok = (actual == expected);
    if (!ok) {
        ++gFail;
    }
    std::printf("%s  #%02d %s\r\n      操作: %s\r\n      期望: %s  实际: %s\r\n",
                ok ? "[通过]" : "[失败]", gCase, title,
                gSeq.toUtf8().constData(),
                expected.toUtf8().constData(),
                actual.toUtf8().constData());
    gSeq.clear();
}

/** 检查显示区（当前操作数 / 计算结果） */
static void check(const char *title, const QString &expected)
{
    checkValue(title, gDisplay->text(), expected);
}

/** 检查表达式行（输入的数据 + 运算符 + 结果） */
static void checkExpr(const char *title, const QString &expected)
{
    checkValue(title, gExpression->text(), expected);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    gWin = &window;
    gDisplay = window.findChild<QLineEdit *>(QStringLiteral("display"));
    gExpression = window.findChild<QLabel *>(QStringLiteral("expressionLabel"));
    if (!gDisplay || !gExpression) {
        std::printf("找不到 display / expressionLabel 控件\r\n");
        return 2;
    }

    // ---------------- 基本运算 ----------------
    mouse("btn_c");
    mouse("btnNum1"); mouse("btnNum2"); mouse("btn_add");
    mouse("btnNum3"); mouse("btn_add"); mouse("btnNum4"); mouse("btn_equal");
    check("鼠标连续运算 12+3+4", QStringLiteral("19"));

    mouse("btn_c");
    mouse("btnNum0"); mouse("btn_dot"); mouse("btnNum5");
    mouse("btn_add");
    mouse("btnNum0"); mouse("btn_dot"); mouse("btnNum2"); mouse("btnNum5");
    mouse("btn_equal");
    check("小数运算 0.5+0.25", QStringLiteral("0.75"));

    mouse("btn_c");
    mouse("btnNum0"); mouse("btn_dot"); mouse("btnNum1");
    mouse("btn_add");
    mouse("btnNum0"); mouse("btn_dot"); mouse("btnNum2");
    mouse("btn_equal");
    check("浮点精度 0.1+0.2", QStringLiteral("0.3"));

    // ---------------- 异常输入 ----------------
    mouse("btn_c");
    mouse("btnNum1"); mouse("btn_dot"); mouse("btn_dot"); mouse("btn_dot"); mouse("btnNum2");
    check("连续小数点 1...2", QStringLiteral("1.2"));
    mouse("btnNum3");
    check("小数点后再输入 1.2+3", QStringLiteral("1.23"));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btn_add"); mouse("btn_sub"); mouse("btn_mul");
    mouse("btnNum3"); mouse("btn_equal");
    check("连续运算符 5+-*3=（最后一个运算符生效）", QStringLiteral("15"));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btn_add"); mouse("btn_sub");
    mouse("btnNum3"); mouse("btn_equal");
    check("连续运算符 5+-3=（最后一个运算符生效）", QStringLiteral("2"));

    mouse("btn_c");
    mouse("btnNum8"); mouse("btn_div"); mouse("btnNum0"); mouse("btn_equal");
    check("除数为 0  8÷0", QStringLiteral("不能除以零"));
    checkExpr("除数为 0 时表达式行 8 ÷ 0", QStringLiteral("8 ÷ 0"));

    mouse("btnNum5");
    check("出错后按数字 5", QStringLiteral("5"));
    mouse("btn_add"); mouse("btnNum3"); mouse("btn_equal");
    check("出错后重新计算 5+3", QStringLiteral("8"));

    // ---------------- 结果后继续输入 / 清除 ----------------
    mouse("btn_c");
    mouse("btnNum2"); mouse("btn_add"); mouse("btnNum3"); mouse("btn_equal");
    check("2+3=", QStringLiteral("5"));
    mouse("btnNum7");
    check("结果后继续输入 7", QStringLiteral("7"));
    mouse("btn_add"); mouse("btnNum4"); mouse("btn_equal");
    check("结果后继续运算 7+4", QStringLiteral("11"));

    mouse("btn_c");
    mouse("btnNum1"); mouse("btnNum2"); mouse("btnNum3"); mouse("btn_backspace");
    check("退格 123→12", QStringLiteral("12"));
    mouse("btn_backspace"); mouse("btn_backspace");
    check("退格到空 12→0", QStringLiteral("0"));

    mouse("btn_c");
    mouse("btnNum1"); mouse("btnNum2"); mouse("btn_add"); mouse("btn_backspace");
    check("运算符后退格不影响第一个操作数", QStringLiteral("12"));
    mouse("btnNum3"); mouse("btn_equal");
    check("继续输入并计算 12+3", QStringLiteral("15"));

    mouse("btn_c");
    mouse("btnNum9"); mouse("btnNum9"); mouse("btnNum9");
    check("输入 999", QStringLiteral("999"));
    mouse("btn_c");
    check("C 清空", QStringLiteral("0"));
    mouse("btnNum1"); mouse("btn_add"); mouse("btnNum1"); mouse("btn_equal");
    check("清除后重新计算 1+1", QStringLiteral("2"));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btn_add"); mouse("btnNum3"); mouse("btn_ce"); mouse("btnNum2");
    mouse("btn_equal");
    check("CE 只清当前输入 5+3(CE)2=", QStringLiteral("7"));

    // ---------------- 键盘 ----------------
    mouse("btn_c");
    keyChar(QStringLiteral("1")); keyChar(QStringLiteral("2"));
    keyChar(QStringLiteral("+"));
    keyChar(QStringLiteral("3"));
    keyChar(QStringLiteral("="));
    check("键盘计算 12+3", QStringLiteral("15"));

    mouse("btn_c");
    keyChar(QStringLiteral("6")); keyChar(QStringLiteral("*")); keyChar(QStringLiteral("7"));
    press(Qt::Key_Return);
    check("键盘 6*7 回车", QStringLiteral("42"));

    mouse("btn_c");
    mouse("btnNum1"); keyChar(QStringLiteral("2")); keyChar(QStringLiteral("+"));
    mouse("btnNum3"); keyChar(QStringLiteral("="));
    check("鼠标键盘混合 12+3", QStringLiteral("15"));

    mouse("btn_c");
    keyChar(QStringLiteral("1")); keyChar(QStringLiteral("2")); keyChar(QStringLiteral("3"));
    press(Qt::Key_Backspace);
    check("键盘退格 123→12", QStringLiteral("12"));
    press(Qt::Key_Escape);
    check("Esc 等同 C", QStringLiteral("0"));

    mouse("btn_c");
    keyChar(QStringLiteral(".")); keyChar(QStringLiteral("5"));
    check("键盘输入小数 .5", QStringLiteral("0.5"));

    // ---------------- 一元运算符 ----------------
    mouse("btn_c");
    mouse("btnNum9"); mouse("btn_sqrt");
    check("√9", QStringLiteral("3"));
    mouse("btnNum5"); mouse("btn_sign");
    check("5 取负 ±", QStringLiteral("-5"));
    mouse("btn_square");
    check("(-5)²", QStringLiteral("25"));

    mouse("btn_c");
    mouse("btnNum0"); mouse("btn_reciprocal");
    check("0 的倒数", QStringLiteral("不能除以零"));
    mouse("btn_c");
    mouse("btnNum9"); mouse("btn_sign"); mouse("btn_sqrt");
    check("负数开平方", QStringLiteral("无效输入"));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btnNum0"); mouse("btn_percent");
    check("50%", QStringLiteral("0.5"));

    // ---------------- 表达式行（输入的数据 + 运算符 + 结果）----------------
    mouse("btn_c");
    mouse("btnNum1"); mouse("btnNum2");
    checkExpr("表达式行：只输入第一个操作数时为空", QString());
    mouse("btn_add");
    checkExpr("表达式行 12 +", QStringLiteral("12 + "));
    check("按下运算符后显示区保留第一个操作数", QStringLiteral("12"));
    mouse("btnNum3");
    checkExpr("表达式行 12 + 3", QStringLiteral("12 + 3"));
    mouse("btn_equal");
    checkExpr("表达式行 12 + 3 =", QStringLiteral("12 + 3 ="));
    check("12+3 的结果", QStringLiteral("15"));

    mouse("btn_c");
    mouse("btnNum1"); mouse("btn_add"); mouse("btnNum2");
    mouse("btn_add"); mouse("btnNum3"); mouse("btn_equal");
    checkExpr("连续运算表达式 1 + 2 + 3 =", QStringLiteral("1 + 2 + 3 ="));
    check("连续运算结果 1+2+3", QStringLiteral("6"));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btn_add"); mouse("btn_sub"); mouse("btn_mul");
    checkExpr("连续运算符后表达式 5 ×", QStringLiteral("5 × "));

    mouse("btn_c");
    mouse("btnNum1"); mouse("btnNum2"); mouse("btn_add"); mouse("btnNum3"); mouse("btn_equal");
    mouse("btn_add");
    checkExpr("结果后继续运算的表达式 15 +", QStringLiteral("15 + "));

    mouse("btn_c");
    mouse("btnNum5"); mouse("btnNum0"); mouse("btn_percent");
    checkExpr("一元运算后表达式行不残留旧算式", QString());
    mouse("btn_c");

    // ---------------- 结果状态按退格返回上一步 ----------------
    mouse("btnNum1"); mouse("btnNum2"); mouse("btn_add"); mouse("btnNum3"); mouse("btn_equal");
    check("12+3=", QStringLiteral("15"));
    checkExpr("12+3= 的表达式行", QStringLiteral("12 + 3 ="));
    mouse("btn_backspace");
    check("退格返回上一步：显示区回到第二个操作数", QStringLiteral("3"));
    checkExpr("退格返回上一步：表达式行去掉 =（回到 12 + 3）", QStringLiteral("12 + 3"));
    mouse("btn_backspace");
    check("返回上一步后继续退格", QStringLiteral("0"));
    mouse("btnNum4"); mouse("btn_equal");
    check("修改后重新计算 12+4", QStringLiteral("16"));
    checkExpr("重新计算的表达式 12 + 4 =", QStringLiteral("12 + 4 ="));

    mouse("btn_c");
    keyChar(QStringLiteral("1")); keyChar(QStringLiteral("+"));
    keyChar(QStringLiteral("2")); keyChar(QStringLiteral("="));
    check("键盘 1+2=", QStringLiteral("3"));
    press(Qt::Key_Backspace);
    check("键盘退格也能返回上一步", QStringLiteral("2"));
    checkExpr("键盘退格后的表达式行（去掉 =）", QStringLiteral("1 + 2"));
    press(Qt::Key_Escape);
    checkExpr("C 之后表达式行清空", QString());

    std::printf("\r\n==== 共 %d 个用例，失败 %d 个 ====\r\n", gCase, gFail);
    return gFail == 0 ? 0 : 1;
}
