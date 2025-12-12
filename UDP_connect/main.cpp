#include "UdpWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建UDP widget
    UdpWidget udpWidget;

    // 设置窗口标题
    udpWidget.setWindowTitle("UDP通信工具");

    // 设置初始大小
    udpWidget.resize(700, 600);

    // 连接信号（示例）
    QObject::connect(&udpWidget, &UdpWidget::dataReceived,
                     [](const QByteArray &data, const QHostAddress &sender, quint16 senderPort) {
        qDebug() << "收到数据来自" << UdpWidget::getIPV4(sender) << ":" << senderPort
                                  << "内容:" << QString::fromUtf8(data);
                     });

    QObject::connect(&udpWidget, &UdpWidget::socketErrorOccurred,
                     [](const QString &error) {
                         qWarning() << "Socket错误:" << error;
                     });

    QObject::connect(&udpWidget, &UdpWidget::bindingStatusChanged,
                     [](bool isBound, quint16 port) {
                         qDebug() << (isBound ? "已绑定到端口" : "已解绑端口") << port;
                     });

    // 显示窗口
    udpWidget.show();

    return app.exec();
}
