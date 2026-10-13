#pragma once

/// probe rendering output
class ProbeRender : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT
public:
    explicit ProbeRender(QScreen* sc);
    bool init(const QString& model);
    bool load(const QString& model);
    void reset();
    void update(const QQuaternion& imu);

private:
    Qt3DCore::QEntity* createScene(const QString& model);
    void applyOrientation();

private:
    QQuaternion orientation_;           ///< current orientation (normalized, probe body frame to world frame)
    Qt3DCore::QTransform* transform_;   ///< transform component, owned by the probe entity
    Qt3DCore::QEntity* probeEntity_;    ///< probe model node
    Qt3DRender::QSceneLoader* probe_;   ///< probe model scene
};
