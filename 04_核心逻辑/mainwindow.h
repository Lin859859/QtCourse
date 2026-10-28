#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QKeyEvent;
class QPushButton;

/**
 * 带键盘事件的计算器。
 *
 * 设计要点：
 * 1) 所有输入动作都收敛到 inputDigit() / inputDot() / inputBinaryOperator() /
 *    applyUnaryOperator() 等核心方法上，鼠标（按钮 clicked 信号）与键盘
 *    （keyPressEvent）走完全相同的处理逻辑，保证两种输入方式行为一致，
 *    也避免维护两套重复代码。
 * 2) 用枚举 State 统一描述“当前正在输入第几个操作数”，而不是用栈保存中间结果。
 *    状态机的方式可以自然地覆盖连续运算、连续运算符、结果后继续输入、
 *    除数为 0、错误恢复等边界情况。
 * 3) 界面分两行显示（与手机计算器一致）：
 *    - 上方 expressionLabel 显示已输入的数据与运算符，例如 “12 + 3 =”；
 *    - 下方 display 显示当前输入的操作数或计算结果。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // 键盘事件入口：把按键翻译成与鼠标相同的核心操作
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onDigitButtonClicked();        // 数字键 0~9 共用一个槽
    void onBinaryOperatorClicked();     // + - × ÷ 共用一个槽
    void onUnaryOperatorClicked();      // ± √x x² % 1/x 共用一个槽
    void on_btn_dot_clicked();
    void on_btn_backspace_clicked();
    void on_btn_c_clicked();
    void on_btn_ce_clicked();
    void on_btn_equal_clicked();

private:
    // 计算器的输入状态
    enum class State {
        EnteringFirst,    // 正在输入第一个操作数
        OperatorPending,  // 刚按下运算符，等待输入第二个操作数
        EnteringSecond,   // 正在输入第二个操作数
        ResultShown,      // 显示区是计算结果（按下 = 或一元运算之后）
        Error             // 错误态（除数为 0、负数开平方等）
    };

    // 求值前的状态快照，供“结果状态下按退格返回上一步”使用
    struct Snapshot {
        bool    valid = false;
        State   state = State::EnteringFirst;
        double  firstOperand = 0.0;
        QString pendingOperator;
        QString display;
        QString expressionPrefix;
    };

    // ---- 核心操作：鼠标与键盘共用 ----
    void inputDigit(const QString &digit);
    void inputDot();
    void inputBinaryOperator(const QString &op);
    void applyUnaryOperator(const QString &op);
    void evaluate();                        // 执行一次二元运算（连续运算取中间结果时也调用）
    void requestResult();                   // 按下 = ：记录快照后求值
    void backspace();
    void undoLastEvaluation();               // 结果状态下退格：返回上一步
    void clearAll();                        // C ：全部复位
    void clearEntry();                      // CE：只清当前输入的操作数

    // ---- 状态与显示辅助 ----
    QString operatorCodeOf(const QPushButton *button) const;
    void beginOperandIfNeeded();            // 必要时结束上一个操作数，开始输入新的操作数
    bool isEnteringOperand() const;         // 显示区是否处于可继续追加字符的输入态
    QString displayText() const;
    double currentValue() const;
    void showNumber(double value);
    void showError(const QString &message);

    QString expressionText() const;         // 生成表达式行的文本，如 “12 + 3 =”
    void syncExpression();                  // 把表达式写入 expressionLabel

    void connectButtons();                  // 鼠标信号连接与控件初始化
    void setupDisplay();                    // 显示区属性
    void applyStyleSheet();                 // 样式表

    Ui::MainWindow *ui;

    State   m_state = State::EnteringFirst;
    double  m_firstOperand = 0.0;           // 第一个操作数
    QString m_pendingOperator;              // 待执行的运算符，空串表示没有挂起的运算

    QString m_expressionPrefix;             // 表达式行的已输入部分，如 “12 + ”
    QString m_resultExpression;             // 按下 = 时的完整表达式，如 “12 + 3”

    Snapshot m_snapshot;                    // 上一次 = 运算之前的快照
};

#endif // MAINWINDOW_H
