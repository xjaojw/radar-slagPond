#include "SlagPondWidget.h"
#include "./ui_SlagPondWidget.h"

#include <QTreeWidgetItem>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

SlagPondWidget::SlagPondWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SlagPondWidget)
    , m_udpSocket(new QUdpSocket(this))
    , m_currentPort(0)
    , m_isBound(false)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("水渣池毫米波雷达探测系统 V1.0");
    setMinimumSize(1400, 800);

    // 设置黑色背景
    setStyleSheet("background-color: black; color: white;");

    setupLeftToolbar();
    setup3DViewers();
    setupRightPanel();
    setupBottomControls();

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // 左侧工具栏
    mainLayout->addWidget(m_toolList);

    // 中间3D显示区域
    QVBoxLayout *viewerLayout = new QVBoxLayout();
    viewerLayout->addWidget(m_heightViewer, 1);
    viewerLayout->addWidget(m_bottomControls);
    viewerLayout->setSpacing(5);

    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addWidget(m_heightViewer);
    centerLayout->addWidget(m_distributionViewer);
    centerLayout->setSpacing(5);

    QVBoxLayout *centerMainLayout = new QVBoxLayout();
    centerMainLayout->addLayout(centerLayout, 1);
    centerMainLayout->addWidget(m_bottomControls);
    centerMainLayout->setSpacing(5);

    QWidget *centerWidget = new QWidget();
    centerWidget->setLayout(centerMainLayout);
    mainLayout->addWidget(centerWidget, 1);

    // 右侧控制面板
    mainLayout->addWidget(m_rightWidget);

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &SlagPondWidget::onSocketReadyRead);
}

SlagPondWidget::~SlagPondWidget()
{
    delete ui;
}

void SlagPondWidget::selectFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,                           // 父窗口
        tr("选择文件"),                 // 对话框标题
        "D:/scanData",              // 初始目录
        tr("文本文件 (*.csv);;")      // 文件过滤器
        );

    if (!fileName.isEmpty()) {
        //QFileInfo fileInfo(fileName);
        qDebug() << "选择了文件：" << fileName;
    }
    loadCSV(fileName);
}

void SlagPondWidget::setupLeftToolbar()
{

    m_toolList = new QListWidget(this);
    m_toolList->setFixedWidth(40);
    m_toolList->setStyleSheet(
        "QListWidget { background-color: #2b2b2b; border: 1px solid #555; }"
        "QListWidget::item { height: 35px; text-align: center; color: white; }"
        "QListWidget::item:selected { background-color: #0078d4; }"
        "QToolTip {"
        "   background-color: #2E2E2E;"
        "   color: #FF0000;"
        "   border: 1px solid #555555;"
        "   border-radius: 3px;"
        "   padding: 5px;"
        "}"
        );

    QStringList toolItems = {"默", "俯", "仰", "左", "右", "前", "后",
                             "清", "轴", "背", "粗", "选", "距", "区"};
    QStringList toolTips = {"默认视角", "俯视图", "仰视图", "左视图", "右视图",
                            "前视图", "后视图", "清除", "坐标轴", "背景",
                            "粗细", "选择", "距离", "区域"};

    for (int i = 0; i < toolItems.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(toolItems[i]);
        item->setToolTip(toolTips[i]);
        item->setTextAlignment(Qt::AlignCenter);
        m_toolList->addItem(item);
    }
}

void SlagPondWidget::setup3DViewers()
{
    // 料堆实时高度图
    m_heightViewer = new SlagPondViewWidget(this);
    m_heightViewer->setMinimumSize(500, 500);
    m_heightViewer->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1a1a, stop:1 #2d2d2d);"
        "border: 2px solid #555; border-radius: 3px;"
        );

    QLabel *heightTitle = new QLabel("料堆实时高度图");
    heightTitle->setStyleSheet("color: #00ff00; font-size: 30px; font-weight: bold; background: transparent;");
    heightTitle->setAlignment(Qt::AlignCenter);

    QVBoxLayout *heightLayout = new QVBoxLayout(m_heightViewer);
    heightLayout->addWidget(heightTitle);
    heightLayout->setAlignment(heightTitle, Qt::AlignTop);

    // 料堆实时水渣分布图
    m_distributionViewer = new SlagPondViewWidget(this);
    m_distributionViewer->setMinimumSize(500, 500);
    m_distributionViewer->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1a1a, stop:1 #2d2d2d);"
        "border: 2px solid #555; border-radius: 3px;"
        );

    QLabel *distTitle = new QLabel("料堆实时水渣分布图");
    distTitle->setStyleSheet("color: #00ffff; font-size: 30px; font-weight: bold; background: transparent;");
    distTitle->setAlignment(Qt::AlignCenter);

    QVBoxLayout *distLayout = new QVBoxLayout(m_distributionViewer);
    distLayout->addWidget(distTitle);
    distLayout->setAlignment(distTitle, Qt::AlignTop);
}

void SlagPondWidget::setupRightPanel()
{
    m_rightWidget = new QWidget(this);
    m_rightWidget->setFixedWidth(300);
    m_rightWidget->setStyleSheet("background-color: #f5f5f5; color: black;");

    // 右侧QWidget的布局rightLayout
    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightWidget);

    // 选择-连接Qwidget
    QWidget *selectConnectWidget = new QWidget();
    QVBoxLayout *selectLayout = new QVBoxLayout(selectConnectWidget);

    //QPushButton *load = new QPushButton("更新");

    // 选择按钮
    QGridLayout *optionGrid = new QGridLayout();
    QPushButton *slagPond = new QPushButton("渣池选择");
    QPushButton *equipment = new QPushButton("设备选择");
    QPushButton *point = new QPushButton("点位选择");
    QPushButton *display = new QPushButton("显示模式");

    QString btnStyle = "QPushButton { background-color: #0078d4; color: white; border: none; padding: 8px; }"
                       "QPushButton:hover { background-color: #106ebe; }"
                       "QPushButton:pressed { background-color: #005a9e; }";

    slagPond->setStyleSheet(btnStyle);
    equipment->setStyleSheet(btnStyle);
    point->setStyleSheet(btnStyle);
    display->setStyleSheet(btnStyle);

    optionGrid->addWidget(slagPond, 0, 0);
    optionGrid->addWidget(equipment, 0, 1);
    optionGrid->addWidget(point, 1, 0);
    optionGrid->addWidget(display, 1, 1);

    // 连接设置
    QHBoxLayout *ipLayout = new QHBoxLayout();
    QLabel *ipLabel = new QLabel("连接");
    QLineEdit *ipEdit = new QLineEdit("127.0.0.1");
    QLabel *portLabel = new QLabel(":");
    QLineEdit *portEdit = new QLineEdit("7998");
    ipEdit->setStyleSheet("background: white; color: black;");
    portEdit->setStyleSheet("background: white; color: black;");
    // 圆形状态图标
    QPushButton *connectStatusBtn = new QPushButton();
    connectStatusBtn->setFixedSize(20, 20);
    connectStatusBtn->setStyleSheet(
        "QPushButton {"
        "    border-radius: 10px;"  // 半径设为宽度/高度的一半
        "    background-color: red;"
        "}"
        "QPushButton:hover {"
        "    background-color: darkred;"
        "}"
        );
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipEdit);
    ipLayout->addWidget(portLabel);
    ipLayout->addWidget(portEdit);
    ipLayout->addWidget(connectStatusBtn);

    // 控制按钮
    QGridLayout *btnGrid = new QGridLayout();
    QPushButton *connectBtn = new QPushButton("连接设备");
    QPushButton *disconnectBtn = new QPushButton("断开设备");
    QPushButton *startScanBtn = new QPushButton("开始扫描");
    QPushButton *stopScanBtn = new QPushButton("结束扫描");

    connectBtn->setStyleSheet(btnStyle);
    disconnectBtn->setStyleSheet(btnStyle);
    startScanBtn->setStyleSheet(btnStyle);
    stopScanBtn->setStyleSheet(btnStyle);

    btnGrid->addWidget(connectBtn, 0, 0);
    btnGrid->addWidget(disconnectBtn, 0, 1);
    btnGrid->addWidget(startScanBtn, 1, 0);
    btnGrid->addWidget(stopScanBtn, 1, 1);

    // 设备状态
    QHBoxLayout *deviceStatusLayout = new QHBoxLayout();
    QPushButton *realTtime_pointCloud = new QPushButton("实时点云");
    QPushButton *realTtime_network = new QPushButton("实时网络");
    QPushButton *historicalData = new QPushButton("历史数据");
    deviceStatusLayout->addWidget(realTtime_pointCloud);
    deviceStatusLayout->addWidget(realTtime_network);
    deviceStatusLayout->addWidget(historicalData);

    //selectLayout->addWidget(load);
    selectLayout->addLayout(optionGrid);
    selectLayout->addLayout(ipLayout);
    selectLayout->addLayout(btnGrid);
    selectLayout->addLayout(deviceStatusLayout);

    // 设备状态组
    m_statusGroup = new QGroupBox("设备状态", this);
    m_statusGroup->setStyleSheet(
        "QGroupBox { color: #ff9900; font-weight: bold; border: 2px solid #555; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }"
        );

    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusGroup);

    QLabel *statusLabel = new QLabel("工作状态: 空闲");
    // 渣池状态
    QGridLayout *slagPondLabelLayout = new QGridLayout();
    QLabel *slagPondLabel1 = new QLabel("1号渣池: 正常");
    QLabel *slagPondLabel2 = new QLabel("2号渣池: 正常");
    QLabel *slagPondLabel3 = new QLabel("3号渣池: 正常");
    QLabel *slagPondLabel4 = new QLabel("4号渣池: 正常");
    slagPondLabelLayout->addWidget(slagPondLabel1, 0, 0);
    slagPondLabelLayout->addWidget(slagPondLabel2, 1, 0);
    slagPondLabelLayout->addWidget(slagPondLabel3, 0, 1);
    slagPondLabelLayout->addWidget(slagPondLabel4, 1, 1);

    statusLayout->addWidget(statusLabel);
    statusLayout->addLayout(slagPondLabelLayout);

    // 检测结果组
    m_resultGroup = new QGroupBox("检测结果", this);
    m_resultGroup->setStyleSheet(m_statusGroup->styleSheet());

    QVBoxLayout *resultLayout = new QVBoxLayout(m_resultGroup);

    // 创建表格显示结果
    QTreeWidget *resultTreeWidget = new QTreeWidget;
    resultTreeWidget->setColumnCount(4);
    // 创建表头项并设置表头居中
    QTreeWidgetItem *headerItem = new QTreeWidgetItem();
    for (int i = 0; i < 4; ++i) {
        headerItem->setTextAlignment(i, Qt::AlignCenter);
    }
    headerItem->setText(0, "生成时间");
    headerItem->setText(1, "渣池");
    headerItem->setText(2, "高度");
    headerItem->setText(3, "最高点");
    resultTreeWidget->setHeaderItem(headerItem);

    // 设置QTreeWidget固定列宽
    resultTreeWidget->setColumnWidth(0, 80);
    resultTreeWidget->setColumnWidth(1, 40);
    resultTreeWidget->setColumnWidth(2, 50);
    resultTreeWidget->setColumnWidth(3, 50);

    // 父节点1
    QTreeWidgetItem *parent1 = new QTreeWidgetItem();
    parent1->setText(0, "16:28");
    parent1->setTextAlignment(0, Qt::AlignCenter);  // 第1列文本居中
    resultTreeWidget->addTopLevelItem(parent1);

    // 子节点
    QTreeWidgetItem *child11 = new QTreeWidgetItem(parent1);
    child11->setText(0, "16:28");
    child11->setText(1, "1号");
    child11->setText(2, "8.85");
    child11->setText(3, "5.32,7.53");

    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child11->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child12 = new QTreeWidgetItem(parent1);
    child12->setText(0, "16:28");
    child12->setText(1, "2号");
    child12->setText(2, "6.72");
    child12->setText(3, "3.43,6.82");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child12->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child13 = new QTreeWidgetItem(parent1);
    child13->setText(0, "16:28");
    child13->setText(1, "3号");
    child13->setText(2, "8.07");
    child13->setText(3, "7.23,4.19");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child13->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child14 = new QTreeWidgetItem(parent1);
    child14->setText(0, "16:28");
    child14->setText(1, "4号");
    child14->setText(2, "4.28");
    child14->setText(3, "4.19,5.79");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child14->setTextAlignment(i, Qt::AlignCenter);
    }

    // 父节点2
    QTreeWidgetItem *parent2 = new QTreeWidgetItem();
    parent2->setText(0, "16:29");
    parent2->setTextAlignment(0, Qt::AlignCenter);  // 第1列文本居中
    resultTreeWidget->addTopLevelItem(parent2);


    // 子节点
    QTreeWidgetItem *child21 = new QTreeWidgetItem(parent2);
    child21->setText(0, "16:29");
    child21->setText(1, "1号");
    child21->setText(2, "8.87");
    child21->setText(3, "5.88,7.81");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child21->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child22 = new QTreeWidgetItem(parent2);
    child22->setText(0, "16:29");
    child22->setText(1, "2号");
    child22->setText(2, "6.73");
    child22->setText(3, "3.39,6.91");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child22->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child23 = new QTreeWidgetItem(parent2);
    child23->setText(0, "16:29");
    child23->setText(1, "3号");
    child23->setText(2, "8.09");
    child23->setText(3, "7.20,4.23");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child23->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child24 = new QTreeWidgetItem(parent2);
    child24->setText(0, "16:29");
    child24->setText(1, "4号");
    child24->setText(2, "4.30");
    child24->setText(3, "4.25,5.60");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child24->setTextAlignment(i, Qt::AlignCenter);
    }

    // 父节点3
    QTreeWidgetItem *parent3 = new QTreeWidgetItem();
    parent3->setText(0, "16:30");
    parent3->setTextAlignment(0, Qt::AlignCenter);  // 第1列文本居中
    resultTreeWidget->addTopLevelItem(parent3);

    // 子节点
    QTreeWidgetItem *child31 = new QTreeWidgetItem(parent3);
    child31->setText(0, "16:30");
    child31->setText(1, "1号");
    child31->setText(2, "8.90");
    child31->setText(3, "5.67,7.71");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child31->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child32 = new QTreeWidgetItem(parent3);
    child32->setText(0, "16:30");
    child32->setText(1, "2号");
    child32->setText(2, "6.75");
    child32->setText(3, "3.32,6.82");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child32->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child33 = new QTreeWidgetItem(parent3);
    child33->setText(0, "16:30");
    child33->setText(1, "3号");
    child33->setText(2, "8.11");
    child33->setText(3, "7.12,4.31");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child33->setTextAlignment(i, Qt::AlignCenter);
    }

    QTreeWidgetItem *child34 = new QTreeWidgetItem(parent3);
    child34->setText(0, "16:30");
    child34->setText(1, "4号");
    child34->setText(2, "4.32");
    child34->setText(3, "4.22,5.63");
    // 每一列文本居中
    for(int i = 0; i < 4; ++i)
    {
        child34->setTextAlignment(i, Qt::AlignCenter);
    }

    // 展开所有节点
    resultTreeWidget->expandAll();

    resultLayout->addWidget(resultTreeWidget);

    // 右侧面板外层布局
    rightLayout->addWidget(selectConnectWidget);
    rightLayout->addWidget(m_statusGroup);
    rightLayout->addWidget(m_resultGroup);

    connect(connectBtn, &QPushButton::clicked, [=]{
        if (m_isBound) {
            m_udpSocket->close();
            m_isBound = false;
        }

        QString ip = ipEdit->text();
        int port = portEdit->text().toInt();
        int localPort = 1234;
        if (m_udpSocket->bind(QHostAddress::Any, localPort)) {
            m_currentPort = localPort;
            m_isBound = true;
            qDebug() << QString("已绑定到本地端口: %1").arg(localPort);
            m_udpSocket->connectToHost(ip, port);
            qDebug() << QString("UDP已连接，IP：%1  端口： %2").arg(ip).arg(port);
            qDebug() << m_udpSocket->isOpen();
            connectStatusBtn->setStyleSheet(
                "QPushButton {"
                "    border-radius: 10px;"  // 半径设为宽度/高度的一半
                "    background-color: green;"
                "}"
                );
            QString message = "我来了~";
            QByteArray datagram = message.toUtf8();
            sendDatagram(datagram, QHostAddress(ipEdit->text()), portEdit->text().toInt());
        } else {
            qDebug() << QString("绑定端口 %1 失败: %2").arg(port).arg(m_udpSocket->errorString());
        }
    });
    connect(disconnectBtn, &QPushButton::clicked, [=]{
        QString message = "我下了~";
        QByteArray datagram = message.toUtf8();
        sendDatagram(datagram, QHostAddress(ipEdit->text()), portEdit->text().toInt());
        if (m_udpSocket && m_udpSocket->isOpen()) {
            m_udpSocket->close();
            m_isBound = false;
            qDebug() << "UDP socket 已关闭";
            connectStatusBtn->setStyleSheet(
                "QPushButton {"
                "    border-radius: 10px;"  // 半径设为宽度/高度的一半
                "    background-color: red;"
                "}"
                );
        }
    });
    connect(startScanBtn, &QPushButton::clicked, [ipEdit, portEdit, this]{
        QString message = "请求开始扫描";
        QByteArray datagram = message.toUtf8();
        sendDatagram(datagram, QHostAddress(ipEdit->text()), portEdit->text().toInt());
    });
    connect(stopScanBtn, &QPushButton::clicked, [ipEdit, portEdit, this]{
        QString message = "请求停止扫描";
        QByteArray datagram = message.toUtf8();
        sendDatagram(datagram, QHostAddress(ipEdit->text()), portEdit->text().toInt());
    });
    // connect(historicalData, &QPushButton::clicked, this, &SlagPondWidget::selectFile);
    connect(historicalData, &QPushButton::clicked, historicalData, [this, child11]{
        this->selectFile();
        // 通过父对象查找子项，避免悬空指针
        QTreeWidget* resultTreeWidget = m_resultGroup->findChild<QTreeWidget*>();
        if (resultTreeWidget) {
            QTreeWidgetItem* firstParent = resultTreeWidget->topLevelItem(0);
            if (firstParent && firstParent->childCount() > 0) {
                QTreeWidgetItem* firstChild = firstParent->child(0);
                firstChild->setText(2, QString::number(m_maxHeight, 'f', 2));
                firstChild->setText(3, QString(QString::number(m_maxHeight_x, 'f', 2) + "," + QString::number(m_maxHeight_y, 'f', 2)));
            }
        }
    });

}

void SlagPondWidget::setupBottomControls()
{
    m_bottomControls = new QWidget(this);
    m_bottomControls->setFixedHeight(80);
    m_bottomControls->setStyleSheet("background: #2b2b2b; border: 1px solid #555;");

    QHBoxLayout *bottomLayout = new QHBoxLayout(m_bottomControls);
    //bottomLayout ->setSpacing(5);

    // 左侧按钮的Widget
    QWidget *leftBtnWidget = new QWidget();
    QHBoxLayout *leftBottomLayout = new QHBoxLayout(leftBtnWidget);

    // 3D操作按钮
    QPushButton *rotateBtn = new QPushButton();
    rotateBtn->setFixedSize(50, 50);
    rotateBtn->setIcon(QIcon("D:/pushButton_picture/rotateBtn.png"));  // 设置图标
    rotateBtn->setIconSize(QSize(50, 50));

    QPushButton *panBtn = new QPushButton();
    panBtn->setFixedSize(50, 50);
    panBtn->setIcon(QIcon("D:/pushButton_picture/panBtn.png"));
    panBtn->setIconSize(QSize(50, 50));

    QPushButton *enlargeBtn = new QPushButton();
    enlargeBtn->setFixedSize(50, 50);
    enlargeBtn->setIcon(QIcon("D:/pushButton_picture/enlargeBtn.png"));
    enlargeBtn->setIconSize(QSize(50, 50));

    QPushButton *reduceBtn = new QPushButton();
    reduceBtn->setFixedSize(50, 50);
    reduceBtn->setIcon(QIcon("D:/pushButton_picture/reduceBtn.png"));
    reduceBtn->setIconSize(QSize(50, 50));

    QString bottomBtnStyle = "QPushButton { background-color: #555; color: white; border: none; padding: 5px 15px; }"
                             "QPushButton:hover { background-color: #666; }";

    rotateBtn->setStyleSheet(bottomBtnStyle);
    panBtn->setStyleSheet(bottomBtnStyle);
    enlargeBtn->setStyleSheet(bottomBtnStyle);
    reduceBtn->setStyleSheet(bottomBtnStyle);

    leftBottomLayout->addStretch();
    leftBottomLayout->addWidget(rotateBtn);
    leftBottomLayout->addStretch();
    leftBottomLayout->addWidget(panBtn);
    leftBottomLayout->addStretch();
    leftBottomLayout->addWidget(enlargeBtn);
    leftBottomLayout->addStretch();
    leftBottomLayout->addWidget(reduceBtn);
    leftBottomLayout->addStretch();
    leftBottomLayout->setAlignment(Qt::AlignJustify);  // 均匀分布所有按钮

    // 左侧按钮的Widget
    QWidget *rightBtnWidget = new QWidget();
    QHBoxLayout *rightBottomLayout = new QHBoxLayout(rightBtnWidget);

    // 3D操作按钮
    QPushButton *rotateBtn2 = new QPushButton();
    rotateBtn2->setFixedSize(50, 50);
    rotateBtn2->setIcon(QIcon("D:/pushButton_picture/rotateBtn.png"));  // 设置图标
    rotateBtn2->setIconSize(QSize(50, 50));

    QPushButton *panBtn2 = new QPushButton();
    panBtn2->setFixedSize(50, 50);
    panBtn2->setIcon(QIcon("D:/pushButton_picture/panBtn.png"));
    panBtn2->setIconSize(QSize(50, 50));

    QPushButton *enlargeBtn2 = new QPushButton();
    enlargeBtn2->setFixedSize(50, 50);
    enlargeBtn2->setIcon(QIcon("D:/pushButton_picture/enlargeBtn.png"));
    enlargeBtn2->setIconSize(QSize(50, 50));

    QPushButton *reduceBtn2 = new QPushButton();
    reduceBtn2->setFixedSize(50, 50);
    reduceBtn2->setIcon(QIcon("D:/pushButton_picture/reduceBtn.png"));
    reduceBtn2->setIconSize(QSize(50, 50));

    rotateBtn2->setStyleSheet(bottomBtnStyle);
    panBtn2->setStyleSheet(bottomBtnStyle);
    enlargeBtn2->setStyleSheet(bottomBtnStyle);
    reduceBtn2->setStyleSheet(bottomBtnStyle);

    rightBottomLayout->addStretch();
    rightBottomLayout->addWidget(rotateBtn2);
    rightBottomLayout->addStretch();
    rightBottomLayout->addWidget(panBtn2);
    rightBottomLayout->addStretch();
    rightBottomLayout->addWidget(enlargeBtn2);
    rightBottomLayout->addStretch();
    rightBottomLayout->addWidget(reduceBtn2);
    rightBottomLayout->addStretch();
    rightBottomLayout->setAlignment(Qt::AlignJustify);  // 均匀分布所有按钮

    bottomLayout->addWidget(leftBtnWidget);
    bottomLayout->addWidget(rightBtnWidget);

    // 连接信号槽
    connect(rotateBtn, &QPushButton::clicked, m_heightViewer, &SlagPondViewWidget::resetView);
    connect(enlargeBtn, &QPushButton::clicked, m_heightViewer, &SlagPondViewWidget::enlarge);
    connect(reduceBtn, &QPushButton::clicked, m_heightViewer, &SlagPondViewWidget::reduce);

    connect(rotateBtn2, &QPushButton::clicked, m_distributionViewer, &SlagPondViewWidget::resetView);
    connect(enlargeBtn2, &QPushButton::clicked, m_distributionViewer, &SlagPondViewWidget::enlarge);
    connect(reduceBtn2, &QPushButton::clicked, m_distributionViewer, &SlagPondViewWidget::reduce);
}

bool SlagPondWidget::loadCSV(const QString &filePath, char separator)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        QMessageBox::warning(this, "错误", QString("无法打开文件: %1").arg(filePath));
        return false;
    }

    QElapsedTimer timer1;
    timer1.start();
    qDebug() << "开始加载并绘制点集";

    QElapsedTimer timer2;
    timer2.start();
    QVector<QVector3D> newPoints;
    newPoints.reserve(1000000);

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Encoding::System);

    // 读取表头
    QString header = in.readLine().trimmed();
    qDebug() << "CSV表头:" << header;

    int lineCount = 0;
    int validPointCount = 0;
    float minHeight;
    float maxHeight;

    // 读取数据
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineCount++;

        if (line.isEmpty()) {
            continue;
        }

        QStringList fields = line.split(separator);
        if (fields.size() < 3) {
            qWarning() << "第" << lineCount << "行数据列数不足，跳过";
            continue;
        }

        // 解析坐标
        bool okX, okY, okZ;
        float x = fields[0].toFloat(&okX);
        //float y = fields[1].toFloat(&okY);
        float z = fields[2].toFloat(&okZ);
        float y = (fields[10].toInt(&okY) % 1151) * 100.0f / 1151.0f;

        if (!okX || !okY || !okZ) {
            qWarning() << "第" << lineCount << "行数据格式错误，跳过";
            continue;
        }

        // 添加点

        // 缩放
        float x_show = x / 4;
        float y_show = y / 4;
        float z_show = z / 4;
        newPoints.append(QVector3D(x_show, y_show, z_show));
        validPointCount++;
        // // 测试：构建多个y分量并绘制
        // for(int i = 0; i < 1000; ++i)
        // {
        //     newPoints.append(QVector3D(x_show, 25.0f * (i / 1000), z_show));
        // }

        // 更新高度范围
        if (validPointCount == 1) {
            minHeight = z;
            maxHeight = z;
            m_maxHeight_x = x;
            m_maxHeight_y = y;
        } else {
            minHeight = qMin(minHeight, z);
            maxHeight = qMax(maxHeight, z);

            if(maxHeight != z)
            {
                m_maxHeight_x = x;
                m_maxHeight_y = y;
            }
        }
        m_minHeight = minHeight;
        m_maxHeight = maxHeight;
        // 限制最大点数量
        if (validPointCount >= 1000000) {
            qDebug() << "达到最大点数限制(10000)，停止读取";
            break;
        }
    }

    file.close();
    float msTime = timer2.nsecsElapsed() / 1000000.0f;
    qDebug() << "加载文件用时:" << msTime << "ms";

    if (validPointCount == 0) {
        qWarning() << "文件中没有有效数据";
        QMessageBox::warning(this, "警告", "文件中没有有效数据");
        return false;
    }

    qDebug() << "成功读取" << validPointCount << "个点，总行数:" << lineCount;
    qDebug() << "高度范围: min=" << minHeight << ", max=" << maxHeight;

    m_heightViewer->setPointsData(newPoints, m_minHeight/4, m_maxHeight/4);

    // 帧时间统计
    msTime = timer1.nsecsElapsed() / 1000000.0f;
    qDebug() << "更新3D图像总耗时:" << msTime << "ms";
    return true;
}

void SlagPondWidget::onSocketReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;

        qint64 bytesRead = m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        if (bytesRead == -1) {
            qDebug() << QString("读取数据报失败: %1").arg(m_udpSocket->errorString());
            continue;
        }

        QString message = QString::fromUtf8(datagram);
        qDebug() << QString("来自 %1:%2 -> %3").arg(senderAddress.toString()).arg(senderPort).arg(message);

    }
}

qint64 SlagPondWidget::sendDatagram(const QByteArray &data, const QHostAddress &targetHost, quint16 targetPort)
{
    if (!m_isBound) {
        qDebug() << "警告: 请先绑定本地端口再发送数据";
        return -1;
    }

    qint64 bytesSent = m_udpSocket->writeDatagram(data, targetHost, targetPort);
    if (bytesSent == -1) {
        qDebug() << QString("发送失败: %1").arg(m_udpSocket->errorString());
    }
    return bytesSent;
}
