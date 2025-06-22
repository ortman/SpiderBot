#include "sbmviewer.h"
#include <QVBoxLayout>
#include <Qt3DRender/QObjectPicker>
#include "qsbmcameracontroller.h"

Qt3DRender::QGeometryRenderer *createLine(const QVector3D& start, const QVector3D& end) {
    auto *geometry = new Qt3DCore::QGeometry();

    // position vertices (start and end)
    QByteArray bufferBytes;
    bufferBytes.resize(3 * 2 * sizeof(float)); // start.x, start.y, start.end + end.x, end.y, end.z
    float *positions = reinterpret_cast<float*>(bufferBytes.data());
    *positions++ = start.x();
    *positions++ = start.y();
    *positions++ = start.z();
    *positions++ = end.x();
    *positions++ = end.y();
    *positions++ = end.z();

    auto *buf = new Qt3DCore::QBuffer(geometry);
    buf->setData(bufferBytes);

    auto *positionAttribute = new Qt3DCore::QAttribute(geometry);
    positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(buf);
    positionAttribute->setByteStride(3 * sizeof(float));
    positionAttribute->setCount(2);
    geometry->addAttribute(positionAttribute); // We add the vertices in the geometry

    // connectivity between vertices
    QByteArray indexBytes;
    indexBytes.resize(2 * sizeof(unsigned int)); // start to end
    unsigned int *indices = reinterpret_cast<unsigned int*>(indexBytes.data());
    *indices++ = 0;
    *indices++ = 1;

    auto *indexBuffer = new Qt3DCore::QBuffer(geometry);
    indexBuffer->setData(indexBytes);

    auto *indexAttribute = new Qt3DCore::QAttribute(geometry);
    indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(2);
    geometry->addAttribute(indexAttribute); // We add the indices linking the points in the geometry

    // mesh
    auto *line = new Qt3DRender::QGeometryRenderer();
    line->setGeometry(geometry);
    line->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    return line;
}

Qt3DRender::QGeometryRenderer *createGrid(QSize size, int gSize, int startX, int startY) {
    startX = startX % size.width();
    startY = startY % size.height();

    int xLineCount = (size.width() - startX) / gSize + 1;
    int yLineCount = (size.height() - startY) / gSize + 1;

    uint vertexCount = (xLineCount + yLineCount) * 2;

    int wPos = size.width() / 2;
    int wNeg = -wPos;
    int hPos = size.height() / 2;
    int hNeg = -hPos;

    Qt3DCore::QGeometry *geometry = new Qt3DCore::QGeometry();
    // position vertices (start and end)
    QByteArray bufferBytes;
    bufferBytes.resize(3 * vertexCount * sizeof(float)); // (x + y + z) * pointCount
    float *positions = reinterpret_cast<float*>(bufferBytes.data());
    int coords = wNeg + startX;
    for (int i=0; i<xLineCount; ++i) {
        *positions++ = wNeg;
        *positions++ = coords;
        *positions++ = 0;

        *positions++ = wPos;
        *positions++ = coords;
        *positions++ = 0;

        coords += gSize;
    }
    coords = hNeg + startY;
    for (int i=0; i<yLineCount; ++i) {
        *positions++ = coords;
        *positions++ = hNeg;
        *positions++ = 0;

        *positions++ = coords;
        *positions++ = hPos;
        *positions++ = 0;

        coords += gSize;
    }

    Qt3DCore::QBuffer *buf = new Qt3DCore::QBuffer(geometry);
    buf->setData(bufferBytes);

    auto *positionAttribute = new Qt3DCore::QAttribute(geometry);
    positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(buf);
    positionAttribute->setByteStride(3 * sizeof(float));
    positionAttribute->setCount(vertexCount);
    geometry->addAttribute(positionAttribute); // We add the vertices in the geometry

    // connectivity between vertices
    QByteArray indexBytes;
    indexBytes.resize(vertexCount * sizeof(unsigned int)); // start to end
    unsigned int *indices = reinterpret_cast<unsigned int*>(indexBytes.data());
    for (uint i =0; i<vertexCount; ++i) {
        *indices++ = i;
    }

    auto *indexBuffer = new Qt3DCore::QBuffer(geometry);
    indexBuffer->setData(indexBytes);

    auto *indexAttribute = new Qt3DCore::QAttribute(geometry);
    indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(vertexCount);
    geometry->addAttribute(indexAttribute); // We add the indices linking the points in the geometry

    // mesh
    Qt3DRender::QGeometryRenderer *grid = new Qt3DRender::QGeometryRenderer();
    grid->setGeometry(geometry);
    grid->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    return grid;
}

SbmViewer::SbmViewer(QWidget *parent) : QWidget(parent) {
    node3DBody = nullptr;
    node3DFoots = nullptr;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    container = createWindowContainer(view, this);
    container->setMinimumSize(100, 100);

    Qt3DRender::QPickingSettings *pSettings = view->renderSettings()->pickingSettings();
    pSettings->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontFace);
    pSettings->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
    pSettings->setPickResultMode(Qt3DRender::QPickingSettings::NearestPick);

    container->setAttribute(Qt::WA_PaintOnScreen);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = new QVBoxLayout();
    //l->setMargin(0);
    l->addWidget(container);
    setLayout(l);

    rootEntity = new Qt3DCore::QEntity;
    spiderBotEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DCore::QTransform *spiderBot = new Qt3DCore::QTransform();
    spiderBot->setTranslation(QVector3D(0,0,55));
    spiderBotEntity->addComponent(spiderBot);

    Qt3DCore::QEntity *groundEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QGeometryRenderer *grid = createGrid({600, 600}, 20, 0, 0);
    groundEntity->addComponent(grid);
    Qt3DExtras::QPhongMaterial *groundMaterial = new Qt3DExtras::QPhongMaterial();
    //groundMaterial->setDiffuse(Qt::gray);
    groundMaterial->setAmbient(Qt::gray);
    groundEntity->addComponent(groundMaterial);

    camera = view->camera();
    camera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    camera->setUpVector(QVector3D(.0f, .0f, 1.0f));
    camera->setPosition(QVector3D(100, 600,  100.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setFarPlane(2000.f);
    //camera->rollAboutViewCenter(135);

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(camera);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1.f);
    lightEntity->addComponent(light);

    Qt3DExtras::QSbmCameraController *camController = new Qt3DExtras::QSbmCameraController(rootEntity);
    camController->setCamera(camera);
    camController->setLinearSpeed(500.0f);
    camController->setLookSpeed(100.0f);

    view->setRootEntity(rootEntity);
    view->show();
}

void SbmViewer::setSbmSettings(sbmSpiderBotSettings_t *settings) {
    //create body
    if (node3DBody != nullptr) {
        node3DBody->removeComponent(bodyMesh);
        node3DBody->removeComponent(bodyMaterial);
        delete node3DBody;
    }
    node3DBody = new Qt3DCore::QEntity(spiderBotEntity);
    bodyMesh = new Qt3DRender::QMesh();
    bodyMaterial = new Qt3DExtras::QPhongMaterial();

    node3DBody->addComponent(bodyMesh);
    node3DBody->addComponent(bodyMaterial);
    if (settings->body->stlUrl->isValid()) {
        bodyMaterial->setDiffuse(settings->body->color);
        //bodyMesh->setMeshName("Body");
        bodyMesh->setSource(*settings->body->stlUrl);
    }

    // create foots
    footCount =  settings->footCount;
    if (node3DFoots != nullptr) {
        free(node3DFoots);
    }

    Qt3DRender::QObjectPicker  *picker;
    node3DFoots = static_cast<Node3DFoot**>(malloc(sizeof (Node3DFoot**) * footCount));
    for (uint32_t i=0; i<footCount; ++i) {
        node3DFoots[i] = new Node3DFoot(&settings->foots[i]);
        node3DFoots[i]->setVectorEnabled(false);
        node3DFoots[i]->setParent(spiderBotEntity);
        picker = new Qt3DRender::QObjectPicker(node3DFoots[i]);
        picker->setHoverEnabled(false);
        picker->setEnabled(true);
        connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &SbmViewer::onClickNode3DFoot);
        node3DFoots[i]->addComponent(picker);
    }
}

void SbmViewer::setActiveFoot(int32_t footIndex) {
    if (footIndex < static_cast<int32_t>(footCount)) {
        for (uint32_t i=0; i < footCount; ++i) {
            node3DFoots[i]->setVectorEnabled(false);
        }
        if (footIndex >= 0) {
            node3DFoots[footIndex]->setVectorEnabled(true);
        }
    }
}

void SbmViewer::setFootAngles(uint32_t footIndex, float *angles) {
    if (footIndex < footCount) {
        node3DFoots[footIndex]->setAngles(angles);
    }
}

void SbmViewer::onClickNode3DFoot(Qt3DRender::QPickEvent *event) {
    if (event->button() == Qt3DRender::QPickEvent::LeftButton) {
        int32_t currentIndex = -1;
        Node3DFoot *nodeFoot3D = dynamic_cast<Node3DFoot*>(sender()->parent());
        if (nodeFoot3D != nullptr) {
            for (uint32_t i=0; i < footCount; ++i) {
                if (node3DFoots[i] == nodeFoot3D) {
                    currentIndex = static_cast<int32_t>(i);
                }
                node3DFoots[i]->setVectorEnabled(false);
            }
            nodeFoot3D->setVectorEnabled(true);
        }
        emit onSelectFoot(currentIndex);
    }
}
