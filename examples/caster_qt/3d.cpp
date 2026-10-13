#include "3d.h"

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DExtras;

#define DEFAULT_VIEW    0.5, 0.5, -0.5, 0.5

namespace
{
    // the imu quaternion rotates the probe body frame into a fixed world frame with z up
    // (gravity aligned) and x toward magnetic north; yaw is arbitrary and slowly drifts when
    // the magnetometer is not collecting. these corrections are fixed rotations, not part of
    // the imu data: axisCorrection maps that world frame onto the Qt3D view frame (y up,
    // z toward the viewer), and modelCorrection accounts for the orientation the probe mesh
    // was authored in.
    const QQuaternion axisCorrection = QQuaternion::fromEulerAngles(0, 180, 90);
    const QQuaternion modelCorrection = QQuaternion::fromEulerAngles(-90, 0, 90);

    /// builds the url for a probe model path, supporting both embedded resources (":/...") and loose files on disk
    /// @param[in] model path to the model
    /// @return the url to load the model from
    QUrl modelUrl(const QString& model)
    {
        return model.startsWith(QLatin1String(":/")) ? QUrl(QStringLiteral("qrc") + model) : QUrl::fromLocalFile(model);
    }
}

/// default constructor
ProbeRender::ProbeRender(QScreen* sc) : Qt3DWindow(sc), orientation_(QQuaternion(DEFAULT_VIEW)), transform_(nullptr)
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
    probe_->setSource(modelUrl(model));
    probeEntity_->addComponent(probe_);
    // the transform component is created once and owned by the entity; only its rotation
    // changes on imu updates
    transform_ = new Qt3DCore::QTransform;
    transform_->setScale3D(QVector3D(100, 100, 100));
    probeEntity_->addComponent(transform_);
    applyOrientation();
    return root;
}

/// loads a different probe model into the existing scene
/// @param[in] model path to the model to load
/// @return success of the call
bool ProbeRender::load(const QString& model)
{
    if (!probe_)
        return false;

    probe_->setSource(modelUrl(model));
    return true;
}

/// applies the current orientation to the probe transform
void ProbeRender::applyOrientation()
{
    if (transform_)
        transform_->setRotation(modelCorrection * (orientation_ * axisCorrection));
}

/// updates the latest orientation
/// @param[in] imu the latest imu data
void ProbeRender::update(const QQuaternion& imu)
{
    // a zero quaternion marks invalid data; streamed values also lose precision in the
    // double to float conversion, so renormalize before rendering to keep the rotation pure
    if (imu.lengthSquared() < 0.5f)
        return;
    orientation_ = imu.normalized();
    applyOrientation();
}

/// resets the view
void ProbeRender::reset()
{
    update(QQuaternion(DEFAULT_VIEW));
}
