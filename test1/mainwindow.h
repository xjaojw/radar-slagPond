#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QPushButton *frontButton;
    QPushButton *backButton;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QPushButton *topButton;
    QPushButton *bottomButton;
    QLineEdit *ipEdit;
    QSpinBox *portSpinBox;
};
#endif // MAINWINDOW_H
