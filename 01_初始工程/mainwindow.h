#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStack<QString> operands;
    QStack<QString> opcodes;
    QString calculation(bool *ok = NULL);

private slots:
    void btnNumClicked();
    void btnBinaryOperatorClicked();
    void btnUnaryOperatorClicked();

    void on_btn_dot_clicked();
    void on_btn_backspace_clicked();
    void on_btn_c_clicked();
    void on_btn_ce_clicked();
    void on_btn_equal_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
