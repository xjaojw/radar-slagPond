#ifndef UDPWIDGET_H
#define UDPWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDateTime>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

class UdpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UdpWidget(QWidget *parent = nullptr);
    ~UdpWidget();
    static QString getIPV4(const QHostAddress &address)
    {
        QString ipv4String;
        quint32 ip_ipv4 = address.toIPv4Address();
        if (ip_ipv4 != 0) {
            ipv4String = QString("%1.%2.%3.%4")
            .arg((ip_ipv4 >> 24) & 0xFF)    // 第一个字节
                .arg((ip_ipv4 >> 16) & 0xFF)    // 第二个字节
                .arg((ip_ipv4 >> 8) & 0xFF)     // 第三个字节
                .arg(ip_ipv4 & 0xFF);           // 第四个字节
        }
        return ipv4String;
    }


    // 公共接口方法
    bool bindToPort(quint16 port);
    void closeSocket();
    qint64 sendDatagram(const QByteArray &data, const QHostAddress &targetHost, quint16 targetPort);
    bool isBound() const;
    quint16 currentPort() const;

public slots:
    void sendMessage();
    void clearLog();

signals:
    void dataReceived(const QByteArray &data, const QHostAddress &sender, quint16 senderPort);
    void socketErrorOccurred(const QString &errorString);
    void bindingStatusChanged(bool isBound, quint16 port);

private slots:
    void onSendButtonClicked();
    void onSocketReadyRead();
    void onBindButtonClicked();
    void onClearLogButtonClicked();

private:
    void initUI();
    void initConnections();
    void logMessage(const QString &type, const QString &msg);

    // UI组件
    QLineEdit *m_targetHostEdit;
    QLineEdit *m_targetPortEdit;
    QLineEdit *m_localPortEdit;
    QPushButton *m_bindButton;
    QPushButton *m_sendButton;
    QPushButton *m_clearLogButton;
    QTextEdit *m_sendTextEdit;
    QTextEdit *m_logTextEdit;

    // 网络组件
    QUdpSocket *m_udpSocket;
    quint16 m_currentPort;
    bool m_isBound;
};

#endif // UDPWIDGET_H
