#ifndef SLAGPONDVIEWWIDGET_H
#define SLAGPONDVIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include <QVector>
#include <QMap>
#include <array>

class SlagPondViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SlagPondViewWidget(QWidget *parent = nullptr);
    ~SlagPondViewWidget();

    void resetView();
    void enlarge();
    void reduce();

    // 绘制三维点集函数
    void drawPoints3D(const QVector<QVector3D>& points,
                      const QVector4D& pointColor = QVector4D(1.0f, 0.0f, 0.0f, 1.0f),
                      float pointSize = 2.0f);

    // 批量更新点集数据
    void setPointsData(const QVector<QVector3D>& points, float minHeight, float maxHeight);

    // 加载CSV数据
    bool loadCSV(const QString& filePath, char separator = ',');

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // 顶点结构体
    struct Vertex {
        float x, y, z;
        float r, g, b, a;

        Vertex(float px = 0, float py = 0, float pz = 0,
               float pr = 0, float pg = 0, float pb = 0, float pa = 0)
            : x(px), y(py), z(pz), r(pr), g(pg), b(pb), a(pa) {}
    };

    // 缓存几何体数据
    struct GeometryCache {
        QVector<Vertex> vertices;
        QVector<unsigned int> indices;
        QOpenGLBuffer vertexBuffer;
        QOpenGLBuffer indexBuffer;
        int vertexCount = 0;
        int indexCount = 0;
        bool needsUpdate = true;

        GeometryCache()
            : vertexBuffer(QOpenGLBuffer::VertexBuffer)
            , indexBuffer(QOpenGLBuffer::IndexBuffer) {}
    };

    // 网格和填充面的缓存
    struct {
        GeometryCache grid;
        std::array<GeometryCache, 5> fills;
        std::array<bool, 5> fillValid;
    } m_geometries;
    QVector<int> fillingOrder;

    void setupShaderProgram();
    void updateGridGeometry();
    void updateFillGeometry(int perspective, float transparency);
    void updateAllGeometries();
    void updatePointsGeometry();
    void drawGrid();
    void drawFillGeometry(int perspective);
    void drawTickMarks();
    void drawPoints();
    void drawAll();

    QColor getColorForHeight(float height) const;
    void updateColorGradient();
    void buildColorLookupTable();
    float interpolateColor(float value, float v1, float v2, float c1, float c2) const;

    // 颜色查找表
    QVector<QVector4D> m_colorLookupTable;
    static const int COLOR_TABLE_SIZE = 256;
    bool m_colorTableValid = false;

    QOpenGLShaderProgram *m_shaderProgram;

    // VAOs
    QOpenGLVertexArrayObject *m_vaoGrid;
    QOpenGLVertexArrayObject *m_vaoPoints;
    std::array<QOpenGLVertexArrayObject*, 5> m_vaoFills;

    // 点集数据
    QOpenGLBuffer m_pointsBuffer;
    QVector<QVector3D> m_points;
    QVector<Vertex> m_pointVertices;
    int m_pointsCount;
    QVector4D m_pointsColor;
    float m_pointsSize;
    bool m_pointsDirty;

    // 颜色渐变相关
    QVector4D m_lowColor;
    QVector4D m_highColor;
    QMap<float, QColor> m_colorGradient;
    float m_surfaceOpacity;
    float m_minHeight;
    float m_maxHeight;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    QMatrix4x4 m_mvpMatrix;
    bool m_transformDirty;

    QQuaternion m_rotation;
    QPoint m_lastMousePos;

    // 摄像机距离
    float m_xDistance;
    float m_yDistance;
    float m_zDistance;

    // 地形尺寸
    const float m_length = 25.0f;
    const float m_width = 25.0f;
    const float m_height = 25.0f;

    // 不同视角的摄像机位置
    const std::array<QVector3D, 4> m_cameraPositions = {
        QVector3D(-30.0f, -60.0f, 25.0f),   // 视角0
        QVector3D(60.0f, -30.0f, 25.0f),     // 视角1
        QVector3D(30.0f, 60.0f, 25.0f),      // 视角2
        QVector3D(-60.0f, 30.0f, 25.0f)      // 视角3
    };

    // 当前视角
    int m_perspective;

    // 几何体状态
    bool m_geometryValid;

    // 帧时间统计
    qint64 m_lastFrameTime = 0;
    float m_frameTime = 0.0f;
    int m_frameCount = 0;
};

#endif // SLAGPONDVIEWWIDGET_H
