#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <cmath>

namespace {
// 运算代码常量：与按钮上显示的文字解耦。
// 例如倒数按钮显示的是 ¹⁄ₓ，如果用按钮文字来比较运算符，很容易出错。
const QString OP_ADD     = QStringLiteral("+");
const QString OP_SUB     = QStringLiteral("-");
const QString OP_MUL     = QStringLiteral("×");
const QString OP_DIV     = QStringLiteral("÷");
const QString OP_SIGN    = QStringLiteral("±");
const QString OP_SQUARE  = QStringLiteral("x²");
const QString OP_SQRT    = QStringLiteral("√x");
const QString OP_PERCENT = QStringLiteral("%");
const QString OP_RECIP   = QStringLiteral("1/x");

const QChar CHAR_ZERO  = QLatin1Char('0');
const QChar CHAR_MINUS = QLatin1Char('-');
const QChar CHAR_DOT   = QLatin1Char('.');
const QChar CHAR_SPACE = QLatin1Char(' ');
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectButtons();     // 鼠标信号连接 + 控件初始化
    setupDisplay();       // 显示区属性
    applyStyleSheet();    // 用样式表美化界面
    syncExpression();     // 初始化表达式行

    // 焦点始终留在窗口上，保证 keyPressEvent 能收到键盘输入
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// 初始化
// ------------------------------------------------------------------

/**
 * 连接鼠标信号。
 * 同类按钮共用一个槽，槽内通过 sender() 判断具体是哪个按钮，
 * 避免为 20 多个按钮各写一个几乎相同的函数。
 */
void MainWindow::connectButtons()
{
    // 数字键 0~9：槽内取按钮文字作为输入的数字
    for (int i = 0; i <= 9; ++i) {
        QPushButton *button = findChild<QPushButton *>(QStringLiteral("btnNum%1").arg(i));
        if (button) {
            connect(button, &QPushButton::clicked, this, &MainWindow::onDigitButtonClicked);
        }
    }
    // 二元运算符按钮
    for (QPushButton *button : { ui->btn_add, ui->btn_sub, ui->btn_mul, ui->btn_div }) {
        connect(button, &QPushButton::clicked, this, &MainWindow::onBinaryOperatorClicked);
    }
    // 一元运算符按钮
    for (QPushButton *button : { ui->btn_sign, ui->btn_square, ui->btn_sqrt,
                                 ui->btn_percent, ui->btn_reciprocal }) {
        connect(button, &QPushButton::clicked, this, &MainWindow::onUnaryOperatorClicked);
    }
    // btn_dot、btn_backspace、btn_c、btn_ce、btn_equal 由 UI 文件的
    // connectSlotsByName 自动连接到 on_<控件名>_clicked 槽，这里不再重复连接。

    // 所有按钮都不接收焦点：焦点固定在窗口上，键盘事件不会被按钮拦截，
    // 也不会出现"回车触发上一次点击过的按钮"的问题。
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumSize(64, 52);
    }

    // 用动态属性给按钮分类，样式表按类别统一配色，避免为每个按钮写一条规则
    for (int i = 0; i <= 9; ++i) {
        QPushButton *button = findChild<QPushButton *>(QStringLiteral("btnNum%1").arg(i));
        if (button) {
            button->setProperty("role", "digit");
        }
    }
    ui->btn_dot->setProperty("role", "digit");
    for (QPushButton *button : { ui->btn_add, ui->btn_sub, ui->btn_mul, ui->btn_div }) {
        button->setProperty("role", "operator");
    }
    for (QPushButton *button : { ui->btn_sign, ui->btn_square, ui->btn_sqrt,
                                 ui->btn_percent, ui->btn_reciprocal }) {
        button->setProperty("role", "unary");
    }
    for (QPushButton *button : { ui->btn_c, ui->btn_ce, ui->btn_backspace }) {
        button->setProperty("role", "function");
    }
    ui->btn_equal->setProperty("role", "equal");
}

void MainWindow::setupDisplay()
{
    // 显示区只读且不接收焦点：
    // 1) 显示内容统一由状态机写入，用户无法直接编辑显示区，
    //    避免出现"显示内容和内部状态不一致"的问题；
    // 2) 焦点不会被显示区抢走，键盘输入始终由 keyPressEvent 统一处理。
    ui->display->setReadOnly(true);
    ui->display->setFocusPolicy(Qt::NoFocus);
    ui->display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->display->setText(QStringLiteral("0"));

    // 表达式行只做显示：横向策略设为 Ignored，表达式很长时也不会把窗口撑宽
    ui->expressionLabel->setText(QString());
    ui->expressionLabel->setFocusPolicy(Qt::NoFocus);
    QSizePolicy policy = ui->expressionLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    policy.setVerticalPolicy(QSizePolicy::Preferred);
    ui->expressionLabel->setSizePolicy(policy);
}

void MainWindow::applyStyleSheet()
{
    // 用样式表统一界面风格：数字键、运算符、功能键、等号键用不同底色区分，
    // 圆角 + 留白让界面更接近系统计算器。
    setStyleSheet(QStringLiteral(R"QSS(
        QMainWindow, QWidget#centralwidget { background-color: #f2f4f7; }

        QLineEdit#display {
            background-color: #ffffff;
            border: 1px solid #d7dbe0;
            border-radius: 8px;
            padding: 4px 14px;
            font-size: 30px;
            font-weight: 600;
            color: #1f2937;
        }

        QLabel#expressionLabel {
            background-color: transparent;
            color: #6b7280;
            font-size: 15px;
            min-height: 22px;
            padding: 0px 14px;
        }

        QPushButton {
            border: 1px solid #e1e5ea;
            border-radius: 8px;
            background-color: #ffffff;
            font-size: 18px;
            color: #1f2937;
        }
        QPushButton:hover   { background-color: #eceff3; }
        QPushButton:pressed { background-color: #dfe4ea; }

        QPushButton[role="digit"]    { font-size: 20px; }
        QPushButton[role="unary"]    { background-color: #f7f8fa; color: #475569; font-size: 16px; }
        QPushButton[role="function"] { background-color: #f7f8fa; color: #b91c1c; font-size: 16px; }
        QPushButton[role="operator"] { background-color: #eaf0fb; color: #1d4ed8;
                                       font-size: 22px; font-weight: 600; }

        QPushButton[role="equal"] {
            background-color: #2563eb;
            border-color: #2563eb;
            color: #ffffff;
            font-size: 22px;
            font-weight: 600;
        }
        QPushButton[role="equal"]:hover   { background-color: #1d4ed8; }
        QPushButton[role="equal"]:pressed { background-color: #1e40af; }
    )QSS"));
}

// ------------------------------------------------------------------
// 键盘事件：只做"按键 -> 语义"的翻译，真正的处理复用鼠标那一套核心方法
// ------------------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const QString text = event->text();
    if (!text.isEmpty()) {
        const QChar ch = text.at(0);
        if (ch.isDigit())                                     { inputDigit(QString(ch));        return; }
        if (ch == CHAR_DOT)                                   { inputDot();                     return; }
        if (ch == QLatin1Char('+'))                           { inputBinaryOperator(OP_ADD);    return; }
        if (ch == CHAR_MINUS)                                 { inputBinaryOperator(OP_SUB);    return; }
        if (ch == QLatin1Char('*') || ch == QLatin1Char('x') || ch == QLatin1Char('X'))
                                                              { inputBinaryOperator(OP_MUL);    return; }
        if (ch == QLatin1Char('/'))                           { inputBinaryOperator(OP_DIV);    return; }
        if (ch == QLatin1Char('%'))                           { applyUnaryOperator(OP_PERCENT); return; }
        if (ch == QLatin1Char('='))                           { requestResult();                return; }
    }

    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Equal:
        requestResult();
        return;
    case Qt::Key_Backspace:
        backspace();
        return;
    case Qt::Key_Delete:
        clearEntry();          // Delete 与 CE 等价
        return;
    case Qt::Key_Escape:
        clearAll();            // Esc 与 C 等价
        return;
    case Qt::Key_Comma:
        inputDot();            // 小键盘上的逗号也按小数点处理
        return;
    default:
        break;
    }

    QMainWindow::keyPressEvent(event);   // 其余按键交回父类处理
}

// ------------------------------------------------------------------
// 核心操作：鼠标槽函数与键盘事件最终都调用这里
// ------------------------------------------------------------------

/** 输入一个数字字符 */
void MainWindow::inputDigit(const QString &digit)
{
    m_snapshot.valid = false;        // 有了新的输入，上一步的结果不再可回退
    if (m_state == State::Error) {
        clearAll();                  // 出错后按数字键：直接开始一次新的计算
    }
    beginOperandIfNeeded();

    QString text = displayText();
    if (text == QStringLiteral("0")) {
        text = digit;                            // 去掉多余的前导 0：0007 显示为 7
    } else if (text == QStringLiteral("-0")) {
        text = QStringLiteral("-") + digit;      // 负数：-0 之后输入 5 得到 -5
    } else {
        text += digit;
    }
    ui->display->setText(text);
    syncExpression();
}

/**
 * 输入小数点。
 * 避免同一操作数重复输入小数点的做法：先检查当前操作数的字符串中是否已经
 * 含有小数点，已经有就忽略本次输入（见下面的 contains 判断），
 * 因此连续点击小数点或连续敲键盘的 "." 都不会产生 "1..2" 这样的非法数据。
 */
void MainWindow::inputDot()
{
    m_snapshot.valid = false;
    if (m_state == State::Error) {
        clearAll();
    }
    beginOperandIfNeeded();

    const QString text = displayText();
    if (!text.contains(CHAR_DOT)) {
        ui->display->setText(text + CHAR_DOT);
    }
    syncExpression();
}

/** 输入二元运算符 */
void MainWindow::inputBinaryOperator(const QString &op)
{
    if (m_state == State::Error || op.isEmpty()) {
        return;                                  // 错误态下忽略运算符，需先按 C 或 CE
    }
    m_snapshot.valid = false;

    if (m_state == State::OperatorPending) {
        // 连续按下运算符：只替换待执行的运算符（表达式行同步替换），不产生新的操作数
        m_expressionPrefix.chop(2);              // 去掉末尾的“运算符 + 空格”
        m_expressionPrefix += op + CHAR_SPACE;
        m_pendingOperator = op;
        syncExpression();
        return;
    }

    // 把“当前操作数 运算符”追加到表达式行（此时显示区里还是当前操作数）
    if (m_state == State::ResultShown) {
        m_expressionPrefix = displayText() + CHAR_SPACE + op + CHAR_SPACE;   // 结果之后重新开始一个表达式
        m_resultExpression.clear();
    } else {
        m_expressionPrefix += displayText() + CHAR_SPACE + op + CHAR_SPACE;  // 第一个运算符 / 连续运算
    }

    if (m_state == State::EnteringSecond) {
        // 已经是"操作数1 运算符 操作数2"，先算出中间结果，实现连续运算
        evaluate();
        if (m_state == State::Error) {
            syncExpression();
            return;                              // 中间结果出错（如除数为 0）
        }
    }

    m_firstOperand    = currentValue();          // 第一个操作数 = 当前显示值
    m_pendingOperator = op;
    m_state           = State::OperatorPending;
    syncExpression();
}

/** 输入一元运算符 */
void MainWindow::applyUnaryOperator(const QString &op)
{
    if (m_state == State::Error || op.isEmpty()) {
        return;
    }
    m_snapshot.valid = false;

    if (op == OP_SIGN) {
        // ± 只改变当前操作数的符号，不结束当前的输入过程
        beginOperandIfNeeded();
        QString text = displayText();
        if (text.isEmpty() || text == QStringLiteral("0")) {
            text = QStringLiteral("-0");         // 便于接着输入负数
        } else if (text == QStringLiteral("-0")) {
            text = QStringLiteral("0");
        } else if (text.startsWith(CHAR_MINUS)) {
            text.remove(0, 1);
        } else {
            text.prepend(CHAR_MINUS);
        }
        ui->display->setText(text);
        syncExpression();
        return;
    }

    // 其余一元运算符立即作用于当前显示的操作数
    const bool inSecondPosition = (m_state == State::OperatorPending)
                                || (m_state == State::EnteringSecond);
    const double value  = currentValue();
    double       result = value;

    if (op == OP_SQUARE) {
        result = value * value;
    } else if (op == OP_SQRT) {
        if (value < 0.0) {
            showError(QStringLiteral("无效输入"));       // 负数不能开平方
            return;
        }
        result = std::sqrt(value);
    } else if (op == OP_PERCENT) {
        result = value / 100.0;
    } else if (op == OP_RECIP) {
        if (value == 0.0) {
            showError(QStringLiteral("不能除以零"));
            return;
        }
        result = 1.0 / value;
    } else {
        return;
    }

    if (!inSecondPosition) {
        // 独立计算（如 √9）：表达式行不再保留之前的算式
        m_expressionPrefix.clear();
        m_resultExpression.clear();
    }
    showNumber(result);
    // 结果仍留在原来的操作数位置上：处于第二个操作数时保持可继续输入的状态
    m_state = inSecondPosition ? State::EnteringSecond : State::ResultShown;
    syncExpression();
}

/** 执行一次二元运算（连续运算需要中间结果时调用，不记录快照） */
void MainWindow::evaluate()
{
    if (m_state == State::Error) {
        return;                                  // 错误态下按 = 不做任何事
    }

    if (m_pendingOperator.isEmpty()) {
        // 没有挂起的运算（例如只输入了数字就按 =）：当前显示值即为结果
        m_state = State::ResultShown;
        return;
    }

    const double  secondOperand = currentValue();
    const double  firstOperand  = m_firstOperand;
    const QString op            = m_pendingOperator;
    double        result        = 0.0;

    if (op == OP_ADD) {
        result = firstOperand + secondOperand;
    } else if (op == OP_SUB) {
        result = firstOperand - secondOperand;
    } else if (op == OP_MUL) {
        result = firstOperand * secondOperand;
    } else if (op == OP_DIV) {
        if (secondOperand == 0.0) {
            showError(QStringLiteral("不能除以零"));   // 除数为 0 的边界处理
            return;
        }
        result = firstOperand / secondOperand;
    } else {
        m_state = State::ResultShown;
        return;
    }

    m_pendingOperator.clear();                   // 一次运算结束后不再挂起运算符
    showNumber(result);
    m_state = State::ResultShown;
}

/** 按下 = ：先记录求值前的快照，再求值 */
void MainWindow::requestResult()
{
    if (m_state == State::Error) {
        return;                                  // 错误态下按 = 不做任何事
    }
    if (m_state == State::ResultShown && m_pendingOperator.isEmpty()) {
        return;                                  // 已经是结果，重复按 = 不再改变界面
    }

    m_snapshot.valid            = true;          // 供结果状态下按退格“返回上一步”
    m_snapshot.state            = m_state;
    m_snapshot.firstOperand     = m_firstOperand;
    m_snapshot.pendingOperator  = m_pendingOperator;
    m_snapshot.display          = displayText();
    m_snapshot.expressionPrefix = m_expressionPrefix;

    m_resultExpression = m_expressionPrefix + displayText();   // 例如 “12 + 3”
    evaluate();
    if (m_state != State::ResultShown) {
        m_snapshot.valid = false;                // 出错时不提供回退
    }
    syncExpression();
}

/** 退格：结果状态下“返回上一步”，输入过程中删除一个字符 */
void MainWindow::backspace()
{
    if (m_state == State::Error) {
        clearAll();
        return;
    }
    if (m_state == State::ResultShown) {
        undoLastEvaluation();                    // 返回上一步：撤销刚做的 = 运算
        return;
    }
    m_snapshot.valid = false;

    if (!isEnteringOperand()) {
        return;                                  // 刚按下运算符时退格无意义，直接忽略
    }

    QString text = displayText();
    text.chop(1);
    if (text.isEmpty() || text == QStringLiteral("-")) {
        text = QStringLiteral("0");              // 删空后回到 0
    }
    ui->display->setText(text);
    syncExpression();
}

/**
 * 返回上一步：把状态恢复到按下 = 之前。
 * 恢复后显示区里是第二个操作数，可以继续用退格修改，或者重新按 = 计算。
 */
void MainWindow::undoLastEvaluation()
{
    if (!m_snapshot.valid) {
        return;                                  // 没有可回退的运算（例如结果来自一元运算）
    }

    m_state            = m_snapshot.state;
    m_firstOperand     = m_snapshot.firstOperand;
    m_pendingOperator  = m_snapshot.pendingOperator;
    m_expressionPrefix = m_snapshot.expressionPrefix;
    m_resultExpression.clear();
    ui->display->setText(m_snapshot.display);
    m_snapshot.valid   = false;                  // 只允许回退一步
    syncExpression();
}

/** C：清空显示与全部运算状态 */
void MainWindow::clearAll()
{
    m_snapshot.valid = false;
    m_state = State::EnteringFirst;
    m_firstOperand = 0.0;
    m_pendingOperator.clear();
    m_expressionPrefix.clear();
    m_resultExpression.clear();
    ui->display->setText(QStringLiteral("0"));
    syncExpression();
}

/** CE：只清当前输入的操作数，保留已输入的第一个操作数和运算符 */
void MainWindow::clearEntry()
{
    if (m_state == State::Error) {
        clearAll();
        return;
    }
    m_snapshot.valid = false;
    if (m_state == State::ResultShown) {
        // 结果态按下 CE：结果作废，回到可输入的状态
        clearAll();
        return;
    }

    ui->display->setText(QStringLiteral("0"));
    if (m_state == State::EnteringSecond) {
        m_state = State::OperatorPending;        // 回到"等待输入第二个操作数"
    }
    syncExpression();
}

// ------------------------------------------------------------------
// 状态与显示辅助
// ------------------------------------------------------------------

/** 按钮 -> 运算代码 */
QString MainWindow::operatorCodeOf(const QPushButton *button) const
{
    if (button == ui->btn_add)        return OP_ADD;
    if (button == ui->btn_sub)        return OP_SUB;
    if (button == ui->btn_mul)        return OP_MUL;
    if (button == ui->btn_div)        return OP_DIV;
    if (button == ui->btn_sign)       return OP_SIGN;
    if (button == ui->btn_square)     return OP_SQUARE;
    if (button == ui->btn_sqrt)       return OP_SQRT;
    if (button == ui->btn_percent)    return OP_PERCENT;
    if (button == ui->btn_reciprocal) return OP_RECIP;
    return QString();
}

/** 显示区是否处于可继续追加字符的输入态 */
bool MainWindow::isEnteringOperand() const
{
    return m_state == State::EnteringFirst || m_state == State::EnteringSecond;
}

/**
 * 必要时切换到"输入新的操作数"的状态。
 * - 刚按下运算符：清空显示区，开始输入第二个操作数；
 * - 刚显示结果：不复用旧结果，开始一次全新的计算。
 */
void MainWindow::beginOperandIfNeeded()
{
    if (m_state == State::OperatorPending) {
        ui->display->setText(QStringLiteral("0"));
        m_state = State::EnteringSecond;
        return;
    }
    if (m_state == State::ResultShown) {
        ui->display->setText(QStringLiteral("0"));
        m_pendingOperator.clear();
        m_expressionPrefix.clear();
        m_resultExpression.clear();
        m_state = State::EnteringFirst;
    }
}

QString MainWindow::displayText() const
{
    return ui->display->text();
}

double MainWindow::currentValue() const
{
    return displayText().toDouble();
}

/** 把计算结果写入显示区 */
void MainWindow::showNumber(double value)
{
    // 用 15 位有效数字显示，避免出现 0.30000000000000004 这类浮点尾数
    QString text = QString::number(value, 'g', 15);
    if (text == QStringLiteral("-0")) {
        text = QStringLiteral("0");
    }
    ui->display->setText(text);
}

/** 进入错误态：提示信息写入显示区，并终止挂起的运算 */
void MainWindow::showError(const QString &message)
{
    m_pendingOperator.clear();
    m_state = State::Error;
    ui->display->setText(message);
    syncExpression();
}

/**
 * 生成表达式行的文本：
 * - 输入第二个操作数时显示 “12 + 3”；
 * - 按下 = 之后显示 “12 + 3 =”；
 * - 刚按下运算符时显示 “12 + ”。
 */
QString MainWindow::expressionText() const
{
    switch (m_state) {
    case State::EnteringSecond:
        return m_expressionPrefix + displayText();
    case State::ResultShown:
        return m_resultExpression.isEmpty()
                   ? QString()
                   : (m_resultExpression + QStringLiteral(" ="));
    case State::Error:
        // 出错时保留完整算式（不显示等号），例如 “8 ÷ 0”
        return m_resultExpression.isEmpty() ? m_expressionPrefix : m_resultExpression;
    case State::EnteringFirst:
    case State::OperatorPending:
    default:
        return m_expressionPrefix;
    }
}

/** 把表达式写入表达式行 */
void MainWindow::syncExpression()
{
    ui->expressionLabel->setText(expressionText());
}

// ------------------------------------------------------------------
// 鼠标槽函数：只负责"取出按钮语义"，然后调用核心方法
// ------------------------------------------------------------------

void MainWindow::onDigitButtonClicked()
{
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }
    inputDigit(button->text());
}

void MainWindow::onBinaryOperatorClicked()
{
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }
    inputBinaryOperator(operatorCodeOf(button));
}

void MainWindow::onUnaryOperatorClicked()
{
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }
    applyUnaryOperator(operatorCodeOf(button));
}

void MainWindow::on_btn_dot_clicked()       { inputDot(); }
void MainWindow::on_btn_backspace_clicked() { backspace(); }
void MainWindow::on_btn_c_clicked()         { clearAll(); }
void MainWindow::on_btn_ce_clicked()        { clearEntry(); }
void MainWindow::on_btn_equal_clicked()     { requestResult(); }
