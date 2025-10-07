#include "3d.h"

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DExtras;

#define DEFAULT_VIEW    0.5, 0.5, -0.5, 0.5

/// default constructor
ProbeRender::ProbeRender(QScreen* sc) : Qt3DWindow(sc), orientation_(QQuaternion(DEFAULT_VIEW))
{
}

/// initializes the renderer
/// @param[in] model path to the model to load
/// @return success of the call
bool ProbeRender::init(const QString& model)
{
    auto sc = createScene(model);
    auto cam = camera();
    cam->lens()->setPerspectiveProjection(50, 16.0f / 9.0f, 0.1f, 1000);
    cam->setPosition(QVector3D(0, 0, 30));
    cam->setViewCenter(QVector3D(0, 0, 0));
    setRootEntity(sc);
    return true;
}

/// creates the scene to render to
/// @param[in] model path to the model to load
/// @return the root entity
QEntity* ProbeRender::createScene(const QString& model)
{
    QEntity* root = new QEntity;
    probeEntity_ = new QEntity(root);
    probe_ = new QSceneLoader(probeEntity_);
    probe_->setSource(QUrl::fromLocalFile(model));
    probeEntity_->addComponent(probe_);
    addTransform();
    return root;
}

/// adds a new transform based on the latest orientation
void ProbeRender::addTransform()
{
    transform_.setScale3D(QVector3D(100, 100, 100));
    QQuaternion axisCorrection(QQuaternion::fromEulerAngles(0, 180, 90));
    QQuaternion modelCorrection(QQuaternion::fromEulerAngles(-90, 0, 90));
    auto modelRotation = orientation_ * axisCorrection;
    auto correctedOrientation = modelCorrection * modelRotation;
    transform_.setRotation(correctedOrientation);
    probeEntity_->addComponent(&transform_);
}

/// updates the latest orientation
/// @param[in] imu the latest imu data
void ProbeRender::update(const QQuaternion& imu)
{
    orientation_ = imu;
    addTransform();
}

/// resets the view
void ProbeRender::reset()
{
    update(QQuaternion(DEFAULT_VIEW));
}
