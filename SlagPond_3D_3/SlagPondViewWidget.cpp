#include "SlagPondViewWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QMessageBox>
#include <cmath>
#include <algorithm>
//#include <random>

static const float GRID_SIZE = 5.0f;

SlagPondViewWidget::SlagPondViewWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 0.0f))
    , m_shaderProgram(nullptr)
    , m_vaoGrid(nullptr)
    , m_vaoPoints(nullptr)
    , m_perspective(0)  // 初始视角设为0
    , m_minHeight(0.0f)
    , m_maxHeight(8.0f)
    , m_pointsCount(0)
    , m_pointsColor(1.0f, 0.0f, 0.0f, 1.0f)
    , m_pointsSize(2.0f)
    , m_lowColor(0.0f, 0.0f, 1.0f, 1.0f)
    , m_highColor(1.0f, 0.0f, 0.0f, 1.0f)
    , m_surfaceOpacity(1.0f)
    , m_pointsDirty(false)
    , m_geometryValid(false)
    , m_colorTableValid(false)
    , m_transformDirty(true)
    , m_frameCount(0)
    , m_frameTime(0.0f)
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);

    // 初始化VAO指针数组
    for (int i = 0; i < 5; ++i) {
        m_vaoFills[i] = nullptr;
    }

    // 设置默认颜色渐变
    m_colorGradient[0.0f] = QColor(0, 0, 255);      // 蓝色（低）
    m_colorGradient[0.3f] = QColor(0, 255, 255);    // 青色
    m_colorGradient[0.6f] = QColor(0, 255, 0);      // 绿色
    m_colorGradient[0.8f] = QColor(255, 255, 0);    // 黄色
    m_colorGradient[1.0f] = QColor(255, 0, 0);      // 红色（高）

    // 初始化填充面状态
    m_geometries.fillValid.fill(false);

    // 预构建颜色查找表
    buildColorLookupTable();

    // 设置初始视角对应的摄像机位置
    m_xDistance = m_cameraPositions[m_perspective].x();
    m_yDistance = m_cameraPositions[m_perspective].y();
    m_zDistance = m_cameraPositions[m_perspective].z();

    // 初始化视图矩阵
    m_view.setToIdentity();
    m_view.lookAt(QVector3D(m_xDistance, m_yDistance, m_zDistance),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 0, 1));
}

SlagPondViewWidget::~SlagPondViewWidget()
{
    makeCurrent();

    // 清理VAO
    if (m_vaoGrid) {
        m_vaoGrid->destroy();
        delete m_vaoGrid;
    }

    if (m_vaoPoints) {
        m_vaoPoints->destroy();
        delete m_vaoPoints;
    }

    for (auto& vao : m_vaoFills) {
        if (vao) {
            vao->destroy();
            delete vao;
        }
    }

    // 清理几何体缓存
    m_geometries.grid.vertexBuffer.destroy();
    m_geometries.grid.indexBuffer.destroy();

    for (auto& fill : m_geometries.fills) {
        fill.vertexBuffer.destroy();
        fill.indexBuffer.destroy();
    }

    // 清理点集缓冲区
    m_pointsBuffer.destroy();

    delete m_shaderProgram;
    doneCurrent();
}

void SlagPondViewWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // 启用深度测试和混合
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // 初始化点集VAO和缓冲区
    m_vaoPoints = new QOpenGLVertexArrayObject();
    m_vaoPoints->create();
    m_pointsBuffer.create();
    m_pointsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    // 设置着色器
    setupShaderProgram();

    // 创建几何体
    updateAllGeometries();

    // 初始化时使用当前视角（而不是调用resetView()切换视角）
    m_transformDirty = true;
    update();
}

void SlagPondViewWidget::setupShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram(this);

    // 顶点着色器
    const char *vshader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec4 color;\n"
        "uniform mat4 mvp;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "    vColor = color;\n"
        "    gl_Position = mvp * vec4(position, 1.0);\n"
        "}";

    // 片段着色器
    const char *fshader =
        "#version 330 core\n"
        "in vec4 vColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vColor;\n"
        "}";

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vshader)) {
        qDebug() << "顶点着色器编译错误:" << m_shaderProgram->log();
    }

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fshader)) {
        qDebug() << "片段着色器编译错误:" << m_shaderProgram->log();
    }

    if (!m_shaderProgram->link()) {
        qDebug() << "着色器链接错误:" << m_shaderProgram->log();
    }
}

bool SlagPondViewWidget::loadCSV(const QString& filePath, char separator)
{
    QElapsedTimer timer;
    timer.start();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        QMessageBox::warning(this, "错误", QString("无法打开文件: %1").arg(filePath));
        return false;
    }

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
        } else {
            minHeight = qMin(minHeight, z);
            maxHeight = qMax(maxHeight, z);
        }
        m_minHeight = minHeight / 4;
        m_maxHeight = maxHeight / 4;

        // 限制最大点数量
        if (validPointCount >= 1000000) {
            qDebug() << "达到最大点数限制(10000)，停止读取";
            break;
        }
    }

    file.close();
    float msTime = timer.nsecsElapsed() / 1000000.0f;
    qDebug() << "加载文件用时:" << msTime << "ms";

    if (validPointCount == 0) {
        qWarning() << "文件中没有有效数据";
        QMessageBox::warning(this, "警告", "文件中没有有效数据");
        return false;
    }

    qDebug() << "成功读取" << validPointCount << "个点，总行数:" << lineCount;
    qDebug() << "高度范围: min=" << minHeight << ", max=" << maxHeight;

    timer.start();
    // 更新点集数据
    m_points = newPoints;
    m_pointsDirty = true;

    // 重新构建颜色查找表
    updateColorGradient();

    // 请求重绘
    update();
    msTime = timer.nsecsElapsed() / 1000000.0f;
    qDebug() << "绘制点集用时:" << msTime << "ms";

    return true;
}

void SlagPondViewWidget::setPointsData(const QVector<QVector3D>& points, float minHeight, float maxHeight)
{
    if (points.isEmpty()) {
        return;
    }

    QElapsedTimer timer;
    timer.start();

    m_points = points;


    // 计算高度范围
    m_minHeight = minHeight;
    m_maxHeight = maxHeight;

    m_pointsDirty = true;

    float msTime = timer.nsecsElapsed() / 1000000.0f;
    qDebug() << "复制数据用时:" << msTime << "ms";
    // 重新构建颜色查找表
    // updateColorGradient();

    timer.start();
    update();
    msTime = timer.nsecsElapsed() / 1000000.0f;
    qDebug() << "绘制点集用时:" << msTime << "ms";

}

void SlagPondViewWidget::drawPoints3D(const QVector<QVector3D>& points,
                                      const QVector4D& pointColor,
                                      float pointSize)
{
    m_points = points;
    m_pointsColor = pointColor;
    m_pointsSize = pointSize;

    // 计算高度范围
    if (!points.isEmpty()) {
        m_minHeight = points[0].z();
        m_maxHeight = points[0].z();

        for (const auto& point : points) {
            float z = point.z();
            m_minHeight = qMin(m_minHeight, z);
            m_maxHeight = qMax(m_maxHeight, z);
        }
    }

    m_pointsDirty = true;

    // 重新构建颜色查找表
    updateColorGradient();

    update();
}

void SlagPondViewWidget::updatePointsGeometry()
{
    if (!m_pointsDirty || m_points.isEmpty()) {
        return;
    }

    m_pointVertices.clear();
    m_pointVertices.reserve(m_points.size());

    // 坐标偏移
    const float offsetX = 0.0f;
    const float offsetY = m_width/2;
    const float offsetZ = 0.0f;

    for (const auto& point : m_points) {
        float x = point.x() - offsetX;
        float y = point.y() - offsetY;
        float z = point.z() - offsetZ;

        // 获取颜色
        QColor color = getColorForHeight(point.z());

        m_pointVertices.append(Vertex(x, y, z,
                                      color.redF(), color.greenF(),
                                      color.blueF(), color.alphaF()));
    }

    m_pointsCount = m_pointVertices.size();

    // 更新点集缓冲区
    if (!m_pointsBuffer.isCreated()) {
        m_pointsBuffer.create();
        m_pointsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }

    m_pointsBuffer.bind();
    m_pointsBuffer.allocate(m_pointVertices.constData(), m_pointVertices.size() * sizeof(Vertex));
    m_pointsBuffer.release();

    m_pointsDirty = false;

    qDebug() << "更新点集几何体，点数:" << m_pointsCount;
}

void SlagPondViewWidget::updateColorGradient()
{
    m_colorTableValid = false;
    buildColorLookupTable();
    m_pointsDirty = true;  // 标记点集需要更新
}

// void SlagPondViewWidget::buildColorLookupTable()
// {
//     m_colorLookupTable.clear();
//     m_colorLookupTable.resize(COLOR_TABLE_SIZE);

//     for (int i = 0; i < COLOR_TABLE_SIZE; ++i) {
//         float t = static_cast<float>(i) / (COLOR_TABLE_SIZE - 1);
//         m_colorLookupTable[i] = QVector4D(
//             m_lowColor.x() * (1.0f - t) + m_highColor.x() * t,
//             m_lowColor.y() * (1.0f - t) + m_highColor.y() * t,
//             m_lowColor.z() * (1.0f - t) + m_highColor.z() * t,
//             m_surfaceOpacity
//             );
//     }

//     m_colorTableValid = true;
//     qDebug() << "颜色查找表构建完成，大小:" << COLOR_TABLE_SIZE;
// }

void SlagPondViewWidget::buildColorLookupTable()
{
    m_colorLookupTable.clear();
    m_colorLookupTable.resize(COLOR_TABLE_SIZE);

    if (m_colorGradient.isEmpty()) {
        // 回退到简单渐变
        for (int i = 0; i < COLOR_TABLE_SIZE; ++i) {
            float t = static_cast<float>(i) / (COLOR_TABLE_SIZE - 1);
            m_colorLookupTable[i] = QVector4D(
                m_lowColor.x() * (1.0f - t) + m_highColor.x() * t,
                m_lowColor.y() * (1.0f - t) + m_highColor.y() * t,
                m_lowColor.z() * (1.0f - t) + m_highColor.z() * t,
                m_surfaceOpacity
                );
        }
    } else {
        // 使用多段渐变
        QList<float> keys = m_colorGradient.keys();
        std::sort(keys.begin(), keys.end());
        for (int i = 0; i < COLOR_TABLE_SIZE; ++i) {
            float t = static_cast<float>(i) / (COLOR_TABLE_SIZE - 1);

            // 找到t所在的分段
            auto it = m_colorGradient.upperBound(t);
            if (it == m_colorGradient.begin()) {
                QColor color = m_colorGradient.first();
                m_colorLookupTable[i] = QVector4D(
                    color.redF(), color.greenF(), color.blueF(), m_surfaceOpacity
                    );
            } else if (it == m_colorGradient.end()) {
                QColor color = m_colorGradient.last();
                m_colorLookupTable[i] = QVector4D(
                    color.redF(), color.greenF(), color.blueF(), m_surfaceOpacity
                    );
            } else {
                auto next = it;
                auto prev = it--;

                float t1 = prev.key();
                float t2 = next.key();
                float segmentT = (t - t1) / (t2 - t1);
                segmentT = qBound(0.0f, segmentT, 1.0f);

                QColor c1 = prev.value();
                QColor c2 = next.value();

                float r = c1.redF() * (1.0f - segmentT) + c2.redF() * segmentT;
                float g = c1.greenF() * (1.0f - segmentT) + c2.greenF() * segmentT;
                float b = c1.blueF() * (1.0f - segmentT) + c2.blueF() * segmentT;

                m_colorLookupTable[i] = QVector4D(r, g, b, m_surfaceOpacity);
            }
        }
    }

    m_colorTableValid = true;
    qDebug() << "颜色查找表构建完成，大小:" << COLOR_TABLE_SIZE;
}

QColor SlagPondViewWidget::getColorForHeight(float height) const
{
    if (m_colorGradient.isEmpty()) {
        return QColor(255, 255, 255, static_cast<int>(255 * m_surfaceOpacity));
    }

    float normalizedHeight = 0.0f;
    if (m_maxHeight > m_minHeight) {
        normalizedHeight = (height - m_minHeight) / (m_maxHeight - m_minHeight);
        normalizedHeight = qBound(0.0f, normalizedHeight, 1.0f);
    }

    float prev;
    float next;
    if(normalizedHeight >= 0.0f && normalizedHeight < 0.3f)
    {
        prev = 0.0f;
        next = 0.3f;
    }
    if(normalizedHeight >= 0.3f && normalizedHeight < 0.6f)
    {
        prev = 0.3f;
        next = 0.6f;
    }
    if(normalizedHeight >= 0.6f && normalizedHeight < 0.8f)
    {
        prev = 0.6f;
        next = 0.8f;
    }
    if(normalizedHeight >= 0.8f && normalizedHeight <= 1.0f)
    {
        prev = 0.8f;
        next = 1.0f;
    }
    QColor c1 = m_colorGradient[prev];
    QColor c2 = m_colorGradient[next];
    float t = (normalizedHeight - prev) / (next - prev);
    float r = c1.red() * (1.0f - t) + c2.red() * t;
    float g = c1.green() * (1.0f - t) + c2.green() * t;
    float b = c1.blue() * (1.0f - t) + c2.blue() * t;
    return QColor(r, g, b, static_cast<int>(255 * m_surfaceOpacity));

    // // 使用颜色查找表
    // if (m_colorTableValid && !m_colorLookupTable.isEmpty()) {
    //     int index = static_cast<int>(normalizedHeight * (COLOR_TABLE_SIZE - 1));
    //     index = qBound(0, index, COLOR_TABLE_SIZE - 1);
    //     const auto& color = m_colorLookupTable[index];
    //     return QColor(static_cast<int>(color.x() * 255),
    //                   static_cast<int>(color.y() * 255),
    //                   static_cast<int>(color.z() * 255),
    //                   static_cast<int>(color.w() * 255));
    // }

    // // 回退到渐变插值
    // auto it = m_colorGradient.upperBound(normalizedHeight);
    // if (it == m_colorGradient.begin()) {
    //     QColor color = m_colorGradient.first();
    //     color.setAlphaF(m_surfaceOpacity);
    //     return color;
    // }
    // else if (it == m_colorGradient.end()) {
    //     QColor color = m_colorGradient.last();
    //     color.setAlphaF(m_surfaceOpacity);
    //     return color;
    // }
    // else {
    //     auto prev = it--;
    //     float t = (normalizedHeight - prev.key()) / (it.key() - prev.key());
    //     t = qBound(0.0f, t, 1.0f);

    //     QColor c1 = prev.value();
    //     QColor c2 = it.value();

    //     int r = static_cast<int>(c1.red() * (1.0f - t) + c2.red() * t);
    //     int g = static_cast<int>(c1.green() * (1.0f - t) + c2.green() * t);
    //     int b = static_cast<int>(c1.blue() * (1.0f - t) + c2.blue() * t);

    //     return QColor(r, g, b, static_cast<int>(255 * m_surfaceOpacity));
    // }
}

void SlagPondViewWidget::updateAllGeometries()
{
    // 更新网格几何体
    updateGridGeometry();

    // 更新填充几何体
    for (int i = 0; i < 5; ++i) {
        float transparency = 0.4f;

        switch(m_perspective) {
        case 0: transparency = (i == 1 || i == 2) ? 0.2f : 0.4f; break;
        case 1: transparency = (i == 2 || i == 3) ? 0.2f : 0.4f; break;
        case 2: transparency = (i == 3 || i == 4) ? 0.2f : 0.4f; break;
        case 3: transparency = (i == 4 || i == 1) ? 0.2f : 0.4f; break;
        }

        updateFillGeometry(i, transparency);
    }

    m_geometryValid = true;
}

void SlagPondViewWidget::updateGridGeometry()
{
    // if (m_geometries.grid.vertexBuffer.isCreated() && !m_geometries.grid.needsUpdate) {
    //     return;
    // }

    const int lengthSegments = static_cast<int>(m_length / GRID_SIZE);
    const int widthSegments = static_cast<int>(m_width / GRID_SIZE);
    const int heightSegments = static_cast<int>(m_height / GRID_SIZE);

    QVector<Vertex> vertices;
    vertices.reserve(2 * ((lengthSegments + 1) * 2 + (widthSegments + 1) * 2 + (heightSegments + 1) * 2));

    auto addLine = [&](float x1, float y1, float z1, float x2, float y2, float z2) {
        vertices.append(Vertex(x1 - m_length/2, y1 - m_width/2, z1 - m_height / 2, 0.8f, 0.8f, 0.8f, 1.0f));
        vertices.append(Vertex(x2 - m_length/2, y2 - m_width/2, z2 - m_height / 2, 0.8f, 0.8f, 0.8f, 1.0f));
    };

    // 底部网格
    for (int i = 0; i <= lengthSegments; ++i) {
        float x = i * GRID_SIZE;
        addLine(x, 0.0f, 0.0f, x, m_width, 0.0f);
    }
    for (int i = 0; i <= widthSegments; ++i) {
        float y = i * GRID_SIZE;
        addLine(0.0f, y, 0.0f, m_length, y, 0.0f);
    }

    // 侧面网格
    switch (m_perspective) {
    case 0: // 后面和右面
        for (int i = 0; i <= lengthSegments; ++i) {
            float x = i * GRID_SIZE;
            addLine(x, m_width, 0.0f, x, m_width, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, m_width, z, m_length, m_width, z);
        }
        for (int i = 0; i <= widthSegments; ++i) {
            float y = i * GRID_SIZE;
            addLine(m_length, y, 0.0f, m_length, y, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(m_length, 0.0f, z, m_length, m_width, z);
        }
        break;

    case 1: // 后面和左面
        for (int i = 0; i <= lengthSegments; ++i) {
            float x = i * GRID_SIZE;
            addLine(x, m_width, 0.0f, x, m_width, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, m_width, z, m_length, m_width, z);
        }
        for (int i = 0; i <= widthSegments; ++i) {
            float y = i * GRID_SIZE;
            addLine(0.0f, y, 0.0f, 0.0f, y, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, 0.0f, z, 0.0f, m_width, z);
        }
        break;

    case 2: // 前面和左面
        for (int i = 0; i <= lengthSegments; ++i) {
            float x = i * GRID_SIZE;
            addLine(x, 0.0f, 0.0f, x, 0.0f, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, 0.0f, z, m_length, 0.0f, z);
        }
        for (int i = 0; i <= widthSegments; ++i) {
            float y = i * GRID_SIZE;
            addLine(0.0f, y, 0.0f, 0.0f, y, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, 0.0f, z, 0.0f, m_width, z);
        }
        break;

    case 3: // 前面和右面
        for (int i = 0; i <= lengthSegments; ++i) {
            float x = i * GRID_SIZE;
            addLine(x, 0.0f, 0.0f, x, 0.0f, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(0.0f, 0.0f, z, m_length, 0.0f, z);
        }
        for (int i = 0; i <= widthSegments; ++i) {
            float y = i * GRID_SIZE;
            addLine(m_length, y, 0.0f, m_length, y, m_height);
        }
        for (int i = 0; i <= heightSegments; ++i) {
            float z = i * GRID_SIZE;
            addLine(m_length, 0.0f, z, m_length, m_width, z);
        }
        break;
    }

    // 顺手添加一下坐标轴的数据
    // X轴
    vertices.append(Vertex(-1.0f - m_length/2, -1.0f - m_width/2, -1.0f - m_height / 2, 1.0f, 0.0f, 0.0f, 1.0f));
    vertices.append(Vertex(5.0f - m_length/2, -1.0f - m_width/2, -1.0f - m_height / 2, 1.0f, 0.0f, 0.0f, 1.0f));
    // Y轴
    vertices.append(Vertex(-1.0f - m_length/2, -1.0f - m_width/2, -1.0f - m_height / 2, 0.0f, 1.0f, 0.0f, 1.0f));
    vertices.append(Vertex(-1.0f - m_length/2, 5.0f - m_width/2, -1.0f - m_height / 2, 0.0f, 1.0f, 0.0f, 1.0f));
    // Z轴
    vertices.append(Vertex(-1.0f - m_length/2, -1.0f - m_width/2, -1.0f - m_height / 2, 0.0f, 0.0f, 1.0f, 1.0f));
    vertices.append(Vertex(-1.0f - m_length/2, -1.0f - m_width/2, 5.0f - m_height / 2, 0.0f, 0.0f, 1.0f, 1.0f));

    // 更新缓冲区
    if (!m_geometries.grid.vertexBuffer.isCreated()) {
        m_geometries.grid.vertexBuffer.create();
    }

    m_geometries.grid.vertexBuffer.bind();
    m_geometries.grid.vertexBuffer.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));
    m_geometries.grid.vertexBuffer.release();

    m_geometries.grid.vertexCount = vertices.size();
    m_geometries.grid.needsUpdate = false;
}

void SlagPondViewWidget::updateFillGeometry(int perspective, float transparency)
{
    if (perspective < 0 || perspective > 4) return;

    // if (m_geometries.fillValid[perspective] && m_geometries.fills[perspective].vertexBuffer.isCreated()) {
    //     return;
    // }

    float depth = 0.5f;
    QVector<Vertex> vertices;
    QVector<unsigned int> indices;

    vertices.reserve(8);
    indices.reserve(12);

    switch (perspective) {
    case 0: { // 底面
        float r = depth, g = depth, b = depth, a = transparency;
        vertices << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a);
        indices << 0 << 1 << 2 << 2 << 3 << 0 << 4 << 5 << 6 << 6 << 7 << 4;
        break;
    }
    case 1: { // 左面
        float r = depth, g = depth, b = depth, a = transparency;
        vertices << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a);
        indices << 0 << 1 << 2 << 2 << 3 << 0 << 4 << 5 << 6 << 6 << 7 << 4;
        break;
    }
    case 2: { // 前面
        float r = depth, g = depth, b = depth, a = transparency;
        vertices << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a);
        indices << 0 << 1 << 2 << 2 << 3 << 0 << 4 << 5 << 6 << 6 << 7 << 4;
        break;
    }
    case 3: { // 右面
        float r = depth, g = depth, b = depth, a = transparency;
        vertices << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, -m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a);
        indices << 0 << 1 << 2 << 2 << 3 << 0 << 4 << 5 << 6 << 6 << 7 << 4;
        break;
    }
    case 4: { // 后面
        float r = depth, g = depth, b = depth, a = transparency;
        vertices << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a)
                 << Vertex(-m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, m_height - m_height / 2, r, g, b, a)
                 << Vertex(m_length / 2, m_width / 2, 0.0f - m_height / 2, r, g, b, a);
        indices << 0 << 1 << 2 << 2 << 3 << 0 << 4 << 5 << 6 << 6 << 7 << 4;
        break;
    }
    }

    auto& fill = m_geometries.fills[perspective];

    if (!fill.vertexBuffer.isCreated()) {
        fill.vertexBuffer.create();
    }
    if (!fill.indexBuffer.isCreated()) {
        fill.indexBuffer.create();
        fill.indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    }

    fill.vertexBuffer.bind();
    fill.vertexBuffer.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));
    fill.vertexBuffer.release();

    fill.indexBuffer.bind();
    fill.indexBuffer.allocate(indices.constData(), indices.size() * sizeof(unsigned int));
    fill.indexBuffer.release();

    fill.vertexCount = vertices.size();
    fill.indexCount = indices.size();
    m_geometries.fillValid[perspective] = true;
}

void SlagPondViewWidget::resizeGL(int w, int h)
{
    float aspect = static_cast<float>(w) / static_cast<float>(h ? h : 1);
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, aspect, 0.1f, 100.0f);
    m_transformDirty = true;
}

void SlagPondViewWidget::paintGL()
{
    QElapsedTimer timer;
    timer.start();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shaderProgram->bind()) {
        qDebug() << "无法绑定着色器程序";
        return;
    }

    // 更新变换矩阵
    if (m_transformDirty) {
        QVector3D rotationCenter(0, 0, 0);
        m_model.setToIdentity();
        m_model.translate(rotationCenter);
        m_model.rotate(m_rotation);
        m_model.translate(-rotationCenter);

        m_mvpMatrix = m_projection * m_view * m_model;
        m_transformDirty = false;
    }

    m_shaderProgram->setUniformValue("mvp", m_mvpMatrix);

    // 更新点集几何体
    updatePointsGeometry();

    // 批量绘制
    drawAll();

    m_shaderProgram->release();

    // 帧时间统计
    m_frameTime = timer.nsecsElapsed() / 1000000.0f;
    m_frameCount++;

    if (m_frameCount % 60 == 0) {
        qDebug() << "Frame time:" << m_frameTime << "ms";
    }
}

void SlagPondViewWidget::drawAll()
{
    // 绘制半透明填充
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    static const std::array<std::array<int, 5>, 4> fillingOrders = {{
        {1, 2, 0, 3, 4},
        {2, 3, 0, 1, 4},
        {3, 4, 0, 1, 2},
        {4, 1, 0, 2, 3}
    }};

    const auto& order = fillingOrders[m_perspective];
    for (int i = 0; i < 5; ++i) {
        if (m_geometries.fillValid[order[i]]) {
            drawFillGeometry(order[i]);
        }
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // 绘制网格
    drawGrid();

    // 绘制刻度
    drawTickMarks();

    // 绘制点集
    drawPoints();
}

void SlagPondViewWidget::drawGrid()
{
    if (!m_geometryValid || m_geometries.grid.vertexCount <= 0 || !m_geometries.grid.vertexBuffer.isCreated()) {
        return;
    }

    if (!m_vaoGrid) {
        m_vaoGrid = new QOpenGLVertexArrayObject();
        m_vaoGrid->create();
    }

    m_vaoGrid->bind();
    m_geometries.grid.vertexBuffer.bind();

    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(float));
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 4, 7 * sizeof(float));

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, m_geometries.grid.vertexCount);

    m_shaderProgram->disableAttributeArray(0);
    m_shaderProgram->disableAttributeArray(1);
    m_geometries.grid.vertexBuffer.release();
    m_vaoGrid->release();
}

void SlagPondViewWidget::drawFillGeometry(int perspective)
{
    if (perspective < 0 || perspective >= 5 || !m_geometries.fillValid[perspective]) {
        return;
    }

    auto& fill = m_geometries.fills[perspective];
    if (fill.vertexCount <= 0 || !fill.vertexBuffer.isCreated() || !fill.indexBuffer.isCreated()) {
        return;
    }

    if (!m_vaoFills[perspective]) {
        m_vaoFills[perspective] = new QOpenGLVertexArrayObject();
        m_vaoFills[perspective]->create();
    }

    m_vaoFills[perspective]->bind();
    fill.vertexBuffer.bind();
    fill.indexBuffer.bind();

    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(float));
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 4, 7 * sizeof(float));

    glDrawElements(GL_TRIANGLES, fill.indexCount, GL_UNSIGNED_INT, nullptr);

    m_shaderProgram->disableAttributeArray(0);
    m_shaderProgram->disableAttributeArray(1);
    fill.indexBuffer.release();
    fill.vertexBuffer.release();
    m_vaoFills[perspective]->release();
}

void SlagPondViewWidget::drawPoints()
{
    if (m_pointsCount <= 0 || !m_pointsBuffer.isCreated()) {
        return;
    }

    if (!m_vaoPoints) {
        m_vaoPoints = new QOpenGLVertexArrayObject();
        m_vaoPoints->create();
    }

    m_vaoPoints->bind();
    m_pointsBuffer.bind();

    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(float));
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 4, 7 * sizeof(float));

    glPointSize(m_pointsSize);
    glDrawArrays(GL_POINTS, 0, m_pointsCount);

    m_shaderProgram->disableAttributeArray(0);
    m_shaderProgram->disableAttributeArray(1);
    m_pointsBuffer.release();
    m_vaoPoints->release();
}

void SlagPondViewWidget::drawTickMarks()
{
    QVector<Vertex> tickVertices;
    tickVertices.reserve(100);

    auto addTick = [&](float x1, float y1, float z1, float x2, float y2, float z2) {
        tickVertices.append(Vertex(x1 - m_length/2, y1 - m_width/2, z1 - m_height / 2, 0.8f, 0.8f, 0.8f, 1.0f));
        tickVertices.append(Vertex(x2 - m_length/2, y2 - m_width/2, z1 - m_height / 2, 0.8f, 0.8f, 0.8f, 1.0f));
    };

    int tickMarkSize = GRID_SIZE / 5;
    float tickMarkLength = 0.5f;
    // // x轴刻度
    // for (int i = tickMarkSize; i <= m_length; i += tickMarkSize) {
    //     addTick(i, 0.0f, 0.0f, i, -0.3f, 0.0f);
    // }

    // // y轴刻度
    // for (int i = 0; i <= m_width; i += tickMarkSize) {
    //     addTick(0.0f, i, 0.0f, -0.3f, i, 0.0f);
    // }

    // // z轴刻度
    // for (int i = 0; i <= m_height; i += tickMarkSize) {
    //     addTick(0.0f, m_width, i, -0.3f, m_width, i);
    // }
    switch (m_perspective) {
    case 0:
        for (int i = tickMarkSize; i <= m_length; i += tickMarkSize) {
            addTick(i, 0.0f, 0.0f, i, -tickMarkLength, 0.0f);
        }

        // y轴刻度
        for (int i = 0; i <= m_width; i += tickMarkSize) {
            addTick(0.0f, i, 0.0f, -tickMarkLength, i, 0.0f);
        }

        // z轴刻度
        for (int i = 0; i <= m_height; i += tickMarkSize) {
            addTick(0.0f, m_width, i, -tickMarkLength, m_width, i);
        }
        break;
    case 1:
        for (int i = tickMarkSize; i <= m_width; i += tickMarkSize) {
            addTick(m_length, i, 0.0f, m_length + tickMarkLength, i, 0.0f);
        }

        for (int i = 0; i <= m_length; i += tickMarkSize) {
            addTick(i, 0.0f, 0.0f, i, -tickMarkLength, 0.0f);
        }

        // z轴刻度
        for (int i = 0; i <= m_height; i += tickMarkSize) {
            addTick(0.0f, 0.0f, i, 0.0f, -tickMarkLength, i);
        }
        break;
    case 2:
        for (int i = tickMarkSize; i <= m_length; i += tickMarkSize) {
            addTick(m_length - i, m_width, 0.0f, m_length - i, m_width + tickMarkLength, 0.0f);
        }

        // y轴刻度
        for (int i = 0; i <= m_width; i += tickMarkSize) {
            addTick(m_length, m_width - i, 0.0f, m_length + tickMarkLength, m_width - i, 0.0f);
        }

        // z轴刻度
        for (int i = 0; i <= m_height; i += tickMarkSize) {
            addTick(m_length, 0, i, m_length + tickMarkLength, 0, i);
        }
        break;
    case 3:
        for (int i = tickMarkSize; i <= m_width; i += tickMarkSize) {
            addTick(0.0f, m_width - i, 0.0f, -tickMarkLength, m_width - i, 0.0f);
        }

        // y轴刻度
        for (int i = 0; i <= m_length; i += tickMarkSize) {
            addTick(i, m_width, 0.0f, i, m_width + tickMarkLength, 0.0f);
        }

        // z轴刻度
        for (int i = 0; i <= m_height; i += tickMarkSize) {
            addTick(m_length, m_width, i, m_length, m_width + tickMarkLength, i);
        }
        break;
    }

    // 使用临时缓冲区绘制
    static QOpenGLBuffer tickBuffer;
    static bool tickBufferInitialized = false;

    if (!tickBufferInitialized) {
        tickBuffer.create();
        tickBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        tickBufferInitialized = true;
    }

    tickBuffer.bind();
    tickBuffer.allocate(tickVertices.constData(), tickVertices.size() * sizeof(Vertex));

    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(float));
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 4, 7 * sizeof(float));

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, tickVertices.size());

    m_shaderProgram->disableAttributeArray(0);
    m_shaderProgram->disableAttributeArray(1);
    tickBuffer.release();
}

void SlagPondViewWidget::resetView()
{
    m_perspective = (m_perspective + 1) % 4;

    // 重置摄像机位置到当前视角对应的位置
    m_xDistance = m_cameraPositions[m_perspective].x();
    m_yDistance = m_cameraPositions[m_perspective].y();
    m_zDistance = m_cameraPositions[m_perspective].z();

    m_view.setToIdentity();
    m_view.lookAt(QVector3D(m_xDistance, m_yDistance, m_zDistance),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 0, 1));

    // 重置旋转
    m_rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 0.0f);

    // 更新几何体以适应新的视角
    updateAllGeometries();
    m_transformDirty = true;
    update();
}

void SlagPondViewWidget::enlarge()
{
    const float enlargeMultiple = 1.1f;
    m_xDistance /= enlargeMultiple;
    m_yDistance /= enlargeMultiple;
    m_zDistance /= enlargeMultiple;

    m_xDistance = qBound(-200.0f, m_xDistance, 200.0f);
    m_yDistance = qBound(-200.0f, m_yDistance, 200.0f);
    m_zDistance = qBound(5.0f, m_zDistance, 200.0f);

    m_view.setToIdentity();
    m_view.lookAt(QVector3D(m_xDistance, m_yDistance, m_zDistance),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 0, 1));
    m_transformDirty = true;
    update();
}

void SlagPondViewWidget::reduce()
{
    const float reduceMultiple = 1.1f;
    m_xDistance *= reduceMultiple;
    m_yDistance *= reduceMultiple;
    m_zDistance *= reduceMultiple;

    m_xDistance = qBound(-200.0f, m_xDistance, 200.0f);
    m_yDistance = qBound(-200.0f, m_yDistance, 200.0f);
    m_zDistance = qBound(5.0f, m_zDistance, 200.0f);

    m_view.setToIdentity();
    m_view.lookAt(QVector3D(m_xDistance, m_yDistance, m_zDistance),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 0, 1));
    m_transformDirty = true;
    update();
}

void SlagPondViewWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
}

// void SlagPondViewWidget::mouseMoveEvent(QMouseEvent *event)
// {
//     if (event->buttons() & Qt::LeftButton) {
//         QVector2D diff = QVector2D(event->pos() - m_lastMousePos);
//         QVector3D axis = QVector3D(diff.y(), diff.x(), 0.0f).normalized();
//         float angle = diff.length() * 0.5f;

//         QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, angle);
//         m_rotation = rotation * m_rotation;
//         m_rotation.normalize();

//         m_lastMousePos = event->pos();
//         m_transformDirty = true;
//         update();
//     }
// }
void SlagPondViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - m_lastMousePos;

        // 水平移动控制绕Z轴旋转
        float zAngle = diff.x() * 0.5f;  // 水平旋转角度
        QQuaternion zRotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), zAngle);

        // 垂直移动控制绕X轴旋转
        float xAngle = diff.y() * 0.5f;  // 垂直旋转角度
        QQuaternion xRotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), xAngle);

        // 组合旋转：先X轴旋转，再Z轴旋转
        m_rotation = zRotation * xRotation * m_rotation;
        m_rotation.normalize();

        m_lastMousePos = event->pos();
        m_transformDirty = true;
        update();
    }
}

void SlagPondViewWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() * 0.001f;
    m_xDistance *= (1 - delta);
    m_yDistance *= (1 - delta);
    m_zDistance *= (1 - delta);

    m_xDistance = qBound(-200.0f, m_xDistance, 200.0f);
    m_yDistance = qBound(-200.0f, m_yDistance, 200.0f);
    m_zDistance = qBound(5.0f, m_zDistance, 200.0f);

    m_view.setToIdentity();
    m_view.lookAt(QVector3D(m_xDistance, m_yDistance, m_zDistance),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 0, 1));
    m_transformDirty = true;
    update();
}

float SlagPondViewWidget::interpolateColor(float value, float v1, float v2, float c1, float c2) const
{
    if (qFuzzyCompare(v1, v2)) {
        return c1;
    }
    float t = (value - v1) / (v2 - v1);
    t = qBound(0.0f, t, 1.0f);
    return c1 * (1.0f - t) + c2 * t;
}
