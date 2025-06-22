#include "sbmfootviewer.h"
#include "node3dfootelement.h"
#include "qsbmcameracontroller.h"

#include <QVBoxLayout>

SbmFootViewer::SbmFootViewer(QWidget *parent) : QWidget(parent) {
    node3DFoot = nullptr;
    node3DBody = nullptr;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    container = createWindowContainer(view, this);


    container->setAttribute(Qt::WA_PaintOnScreen);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = new QVBoxLayout();
    //l->setMargin(0);
    l->addWidget(container);
    setLayout(l);

    rootEntity = new Qt3DCore::QEntity;

    camera = view->camera();
    camera->lens()->setPerspectiveProjection(60.0f, 16.0f/9.0f, 2.f, -1.0f);
    camera->setPosition(QVector3D(50, 200,  50.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->rollAboutViewCenter(135);
    camera->setFarPlane(2000.f);

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(camera);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1.f);
    lightEntity->addComponent(light);

    Qt3DExtras::QSbmCameraController *camController = new Qt3DExtras::QSbmCameraController(rootEntity);
    camController->setCamera(camera);
    camController->setLinearSpeed( 500.0f );
    camController->setLookSpeed( 100.0f );
    view->setRootEntity(rootEntity);
    view->show();
}

void SbmFootViewer::setFoot(sbmFoot_t *foot) {
    if (node3DFoot != nullptr) {
        delete  node3DFoot;
    }
    node3DFoot = new Node3DFoot(foot);
    node3DFoot->setParent(rootEntity);
}

void SbmFootViewer::setBody(sbmBody_t *body) {
    if (node3DBody != nullptr) {
        node3DBody->removeComponent(bodyMesh);
        node3DBody->removeComponent(bodyMaterial);
        delete node3DBody;
    }
    node3DBody = new Qt3DCore::QEntity(rootEntity);
    bodyMesh = new Qt3DRender::QMesh();
    bodyMaterial = new Qt3DExtras::QPhongMaterial();

    node3DBody->addComponent(bodyMesh);
    node3DBody->addComponent(bodyMaterial);
    if (body->stlUrl->isValid()) {
        bodyMaterial->setDiffuse(body->color);
        //bodyMesh->setMeshName("Body");
        bodyMesh->setSource(*body->stlUrl);
    }
}

void SbmFootViewer::translateFoot(QVector3D vector) {
    if (node3DFoot != nullptr) {
        node3DFoot->translate(vector);
    }
}

void SbmFootViewer::segmentRotation(uint32_t segment, QVector3D vector) {
    if (node3DFoot != nullptr) {
        node3DFoot->segmentRotate(segment, vector);
    }
}

void SbmFootViewer::segmentLength(uint32_t segment, float len) {
    if (node3DFoot != nullptr) {
        node3DFoot->segmentLength(segment, len);
    }
}

void SbmFootViewer::segmentModelTranslate(uint32_t segment, QVector3D vector) {
    if (node3DFoot != nullptr) {
        node3DFoot->segmentModelTranslate(segment, vector);
    }
}

void SbmFootViewer::segmentModelRotation(uint32_t segment, QVector3D vector) {
    if (node3DFoot != nullptr) {
        node3DFoot->segmentModelRotate(segment, vector);
    }
}

void SbmFootViewer::segmentModelColor(uint32_t segment, QColor color) {
    if (node3DFoot != nullptr) {
        node3DFoot->segmentModelColor(segment, color);
    }
}

//void SbmFootViewer::paintEvent(QPaintEvent*) {
//    QPainter p(this);
//    QString text = "Some foo goes here";
//    QFontMetrics metrics(p.font());
//    //resize(metrics.size(0, text));
//    p.drawText(rect(), Qt::AlignCenter, text);
//}
