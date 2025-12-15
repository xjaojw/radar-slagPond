#ifndef SLAGPONDWIDGET_H
#define SLAGPONDWIDGET_H

#include "SlagPondViewWidget.h"

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QUdpSocket>
#include <QHostAddress>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui {
class SlagPondWidget;
}
QT_END_NAMESPACE

class SlagPondWidget : public QWidget
{
    Q_OBJECT

public:
    SlagPondWidget(QWidget *parent = nullptr);
    ~SlagPondWidget();
private slots:
    void selectFile();

private:
    Ui::SlagPondWidget *ui;

    void setupLeftToolbar();
    void setup3DViewers();
    void setupRightPanel();
    void setupBottomControls();

    bool loadCSV(const QString& filePath, char separator = ',');

    void onSocketReadyRead();
    qint64 sendDatagram(const QByteArray &data, const QHostAddress &targetHost, quint16 targetPort);

    // 左侧工具栏
    QListWidget *m_toolList;

    // 3D显示区域
    SlagPondViewWidget *m_heightViewer;    // 料堆实时高度图
    SlagPondViewWidget *m_distributionViewer; // 料堆实时水渣分布图

    // 右侧控制面板
    QWidget *m_rightWidget;
    QGroupBox *m_statusGroup;
    QGroupBox *m_resultGroup;

    // 底部控制按钮
    QWidget *m_bottomControls;

    float m_minHeight = 0;
    float m_maxHeight = 0;
    float m_maxHeight_x = 0;
    float m_maxHeight_y = 0;

    // UDP连接相关成员
    QUdpSocket *m_udpSocket;
    quint16 m_currentPort;
    bool m_isBound;
    QVector<QVector3D> m_coordinateQueue;
};
#endif // SLAGPONDWIDGET_H
