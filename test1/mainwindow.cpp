#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("水渣池毫米波雷达探测系统 V1.0");

    // 创建菜单栏
    // QMenuBar *menuBar = new QMenuBar(this);
    // QMenu *settingsMenu = menuBar->addMenu("设置");
    // menuBar->addAction("刷新");

    // 创建工具栏
    QToolBar *leftToolBar = new QToolBar("主工具栏", this);
    addToolBar(Qt::LeftToolBarArea, leftToolBar);

    // 添加动作
    QAction *act1 = new QAction("默", this);
    act1->setShortcut(QKeySequence::New);
    act1->setStatusTip("默认视角");
    leftToolBar->addAction(act1);

    // 创建中央控件和布局
    QPushButton *centralWidget = new QPushButton("mainButton", this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 设备启动按钮 Front, Back, Left, Right, Top, Bottom
    // QPushButton *startButton = new QPushButton("启动采集", this);
    // startButton->setFixedSize(100, 30);

    // frontButton = new QPushButton("前视", this);
    // frontButton->setFixedSize(100, 30);

    // backButton = new QPushButton("后视", this);
    // backButton->setFixedSize(100, 30);

    // leftButton = new QPushButton("左视", this);
    // leftButton->setFixedSize(100, 30);

    // rightButton = new QPushButton("右视", this);
    // rightButton->setFixedSize(100, 30);

    // topButton = new QPushButton("俯视", this);
    // topButton->setFixedSize(100, 30);

    // bottomButton = new QPushButton("仰视", this);
    // bottomButton->setFixedSize(100, 30);

    // IP地址设置
    // QLabel *ipLabel = new QLabel("IP地址:", this);
    // ipEdit = new QLineEdit("192.168.30.91", this);
    // ipEdit->setFixedWidth(120);

    // 端口设置
    // QLabel *portLabel = new QLabel("端口:", this);
    // portSpinBox = new QSpinBox(this);
    // portSpinBox->setRange(1, 65535);
    // portSpinBox->setValue(8866);
    // portSpinBox->setFixedWidth(80);

    // 将控件添加到工具栏
    // leftToolBar->addWidget(startButton);
    // leftToolBar->addSeparator();
    // leftToolBar->addWidget(ipLabel);
    // leftToolBar->addWidget(ipEdit);
    // leftToolBar->addWidget(portLabel);
    // leftToolBar->addWidget(portSpinBox);
    // leftToolBar->addSeparator();
    // leftToolBar->addWidget(frontButton);
    // leftToolBar->addWidget(backButton);
    // leftToolBar->addWidget(leftButton);
    // leftToolBar->addWidget(rightButton);
    // leftToolBar->addWidget(topButton);
    // leftToolBar->addWidget(bottomButton);

}

MainWindow::~MainWindow()
{
    delete ui;
}
