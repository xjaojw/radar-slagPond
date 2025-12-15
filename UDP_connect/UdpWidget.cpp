#include "UdpWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>

UdpWidget::UdpWidget(QWidget *parent)
    : QWidget(parent)
    , m_udpSocket(new QUdpSocket(this))
    , m_currentPort(0)
    , m_isBound(false)
{
    initUI();
    initConnections();    
}

UdpWidget::~UdpWidget()
{
    // 析构函数 - 清理资源
    if (m_udpSocket) {
        m_udpSocket->close();
    }
}

bool UdpWidget::bindToPort(quint16 port)
{
    if (m_udpSocket->state() == QAbstractSocket::BoundState) {
        m_udpSocket->close();
    }

    if (m_udpSocket->bind(QHostAddress::Any, port)) {
        m_currentPort = port;
        m_isBound = true;
        m_bindButton->setText("解绑");
        logMessage("系统", QString("已绑定到本地端口: %1").arg(port));
        emit bindingStatusChanged(true, port);
        return true;
    } else {
        logMessage("错误", QString("绑定端口 %1 失败: %2").arg(port).arg(m_udpSocket->errorString()));
        emit bindingStatusChanged(false, port);
        return false;
    }
}

void UdpWidget::closeSocket()
{
    if (m_udpSocket) {
        m_udpSocket->close();
        m_isBound = false;
        m_bindButton->setText("绑定");
        logMessage("系统", "UDP socket 已关闭");
        emit bindingStatusChanged(false, 0);
    }
}

qint64 UdpWidget::sendDatagram(const QByteArray &data, const QHostAddress &targetHost, quint16 targetPort)
{
    if (!m_isBound) {
        logMessage("警告", "请先绑定本地端口再发送数据");
        return -1;
    }

    qint64 bytesSent = m_udpSocket->writeDatagram(data, targetHost, targetPort);
    if (bytesSent == -1) {
        logMessage("错误", QString("发送失败: %1").arg(m_udpSocket->errorString()));
        emit socketErrorOccurred(m_udpSocket->errorString());
    } else {
        logMessage("发送", QString("%4 到 %1:%2 (长度: %3字节)").arg(targetHost.toString()).arg(targetPort).arg(bytesSent)
                           .arg(data));
    }
    return bytesSent;
}

bool UdpWidget::isBound() const
{
    return m_isBound;
}

quint16 UdpWidget::currentPort() const
{
    return m_currentPort;
}

void UdpWidget::sendMessage()
{
    QString targetHost = m_targetHostEdit->text();
    quint16 targetPort = m_targetPortEdit->text().toUShort();
    QString message = m_sendTextEdit->toPlainText();

    if (message.isEmpty()) {
        logMessage("警告", "发送消息不能为空");
        return;
    }

    QByteArray datagram = message.toUtf8();
    sendDatagram(datagram, QHostAddress(targetHost), targetPort);

    // 清空发送框
    m_sendTextEdit->clear();
}

void UdpWidget::clearLog()
{
    m_logTextEdit->clear();
    logMessage("系统", "日志已清空");
}

void UdpWidget::onSendButtonClicked()
{
    sendMessage();
}

void UdpWidget::onSocketReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;

        qint64 bytesRead = m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        if (bytesRead == -1) {
            logMessage("错误", QString("读取数据报失败: %1").arg(m_udpSocket->errorString()));
            emit socketErrorOccurred(m_udpSocket->errorString());
            continue;
        }

        QString message = QString::fromUtf8(datagram);
        logMessage("接收", QString("来自 %1:%2 -> %3").arg(getIPV4(senderAddress)).arg(senderPort).arg(message));

        m_targetHostEdit->setText(getIPV4(senderAddress));
        m_targetPortEdit->setText(QString::number(senderPort));
        // 发射信号，供其他组件使用
        emit dataReceived(datagram, senderAddress, senderPort);
    }
}

void UdpWidget::onBindButtonClicked()
{
    if (m_isBound) {
        closeSocket();
    } else {
        quint16 port = m_localPortEdit->text().toUShort();
        if (port > 0) {
            bindToPort(port);
        } else {
            logMessage("错误", "无效的端口号");
        }
    }
}

void UdpWidget::onClearLogButtonClicked()
{
    clearLog();
}

void UdpWidget::initUI()
{
    m_timer = new QTimer(this);
    // 创建组件
    QLabel *targetLabel = new QLabel("目标主机:");
    m_targetHostEdit = new QLineEdit("123456");
    m_targetHostEdit->setToolTip("目标主机的IP地址或主机名");

    QLabel *targetPortLabel = new QLabel("目标端口:");
    m_targetPortEdit = new QLineEdit("0000");
    m_targetPortEdit->setToolTip("目标主机的UDP端口");
    m_targetPortEdit->setMaximumWidth(100);

    QLabel *localPortLabel = new QLabel("本地端口:");
    m_localPortEdit = new QLineEdit("7998");
    m_localPortEdit->setToolTip("本地监听的UDP端口");
    m_localPortEdit->setMaximumWidth(100);

    m_bindButton = new QPushButton("绑定");
    m_bindButton->setToolTip("绑定到指定的本地端口");

    m_sendButton = new QPushButton("发送");
    m_sendButton->setToolTip("发送消息到目标主机");

    m_clearLogButton = new QPushButton("清空日志");
    m_clearLogButton->setToolTip("清空日志窗口");

    m_startScan = new QPushButton("开始扫描");

    m_sendTextEdit = new QTextEdit;
    m_sendTextEdit->setPlaceholderText("在此输入要发送的消息...");
    m_sendTextEdit->setMaximumHeight(100);

    m_logTextEdit = new QTextEdit;
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setPlaceholderText("通信日志将显示在这里...");

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 目标设置行
    QHBoxLayout *targetLayout = new QHBoxLayout;
    targetLayout->addWidget(targetLabel);
    targetLayout->addWidget(m_targetHostEdit);
    targetLayout->addWidget(targetPortLabel);
    targetLayout->addWidget(m_targetPortEdit);
    targetLayout->addStretch();
    mainLayout->addLayout(targetLayout);

    // 本地设置行
    QHBoxLayout *localLayout = new QHBoxLayout;
    localLayout->addWidget(localPortLabel);
    localLayout->addWidget(m_localPortEdit);
    localLayout->addWidget(m_bindButton);
    localLayout->addStretch();
    mainLayout->addLayout(localLayout);

    mainLayout->addWidget(new QLabel("发送消息:"));
    mainLayout->addWidget(m_sendTextEdit);

    // 发送按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_sendButton);
    buttonLayout->addWidget(m_clearLogButton);
    buttonLayout->addWidget(m_startScan);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    mainLayout->addWidget(new QLabel("通信日志:"));
    mainLayout->addWidget(m_logTextEdit);

    // 设置窗口属性
    this->setWindowTitle("UDP通信组件 (Qt 6.6.3)");
    this->resize(600, 500);
}

void UdpWidget::initConnections()
{
    // 连接按钮信号
    connect(m_sendButton, &QPushButton::clicked, this, &UdpWidget::onSendButtonClicked);
    connect(m_bindButton, &QPushButton::clicked, this, &UdpWidget::onBindButtonClicked);
    connect(m_clearLogButton, &QPushButton::clicked, this, &UdpWidget::onClearLogButtonClicked);
    connect(m_startScan, &QPushButton::clicked, [this]{
        m_timer->start(20);
    });

    // 连接socket信号
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpWidget::onSocketReadyRead);
    connect(m_udpSocket, &QUdpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        logMessage("错误", QString("Socket错误: %1").arg(m_udpSocket->errorString()));
        emit socketErrorOccurred(m_udpSocket->errorString());
    });
    connect(m_timer, &QTimer::timeout, this, &UdpWidget::startSendCoordinate);
}

void UdpWidget::logMessage(const QString &type, const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString logEntry = QString("[%1] %2: %3").arg(timestamp, type, msg);
    m_logTextEdit->append(logEntry);
}

void UdpWidget::startSendCoordinate()
{
    int arrayLength = 360;
    float coordinate[arrayLength];
    static float distance = 0.0f;
    static float distanceStep = 50.0f / 120.0f;
    static int index = 0;
    float angle = 0.0f;
    for(int i = 0; i < 360; i+=3)
    {
        coordinate[i] = distance * cos(angle) + 50.0f;
        coordinate[i + 1] = (distanceStep > 0) ? distance : 100.0f - distance;
        coordinate[i + 2] = distance * sin(angle) + 50.0f;
        angle += 360.0f / 120.0f;
    }
    distance += distanceStep;
    if(distance >= 50.0f)
    {
        distanceStep = -(50.0f / 120.0f);
    }
    if(distance <= 0.0f)
    {
        distanceStep = 50.0f / 120.0f;
    }
    QString targetHost = m_targetHostEdit->text();
    quint16 targetPort = m_targetPortEdit->text().toUShort();
    QByteArray byteArray;
    int totalBytes  = arrayLength * sizeof(float);
    byteArray.resize(totalBytes + sizeof(int));
    memcpy(byteArray.data(), &totalBytes, sizeof(int));
    memcpy(byteArray.data() + 4, coordinate, totalBytes);

    qint64 bytesSent = m_udpSocket->writeDatagram(byteArray, QHostAddress(targetHost), targetPort);
    if (bytesSent == -1) {
        logMessage("错误", QString("发送失败: %1").arg(m_udpSocket->errorString()));
        emit socketErrorOccurred(m_udpSocket->errorString());
    } else {
        QStringList list;
        for (int i = 0; i < arrayLength; ++i) {
            list.append(QString::number(coordinate[i], 'f', 4));
        }
        QString coorStr = "[" + list.join(", ") + "]";
        logMessage("发送", QString("%4 到 %1:%2 (长度: %3字节)").arg(targetHost).arg(targetPort).arg(bytesSent).arg(coorStr));
        qDebug() << "发送：" <<  QString("%4 到 %1:%2 (长度: %3字节)").arg(targetHost).arg(targetPort).arg(bytesSent).arg(coorStr);
    }
}
