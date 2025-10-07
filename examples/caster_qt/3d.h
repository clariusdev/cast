#pragma once

/// probe rendering output
class ProbeRender : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT
public:
    explicit ProbeRender(QScreen* sc);
    bool init(const QString& model);
    void reset();
    void update(const QQuaternion& imu);

private:
    Qt3DCore::QEntity* createScene(const QString& model);
    void addTransform();

private:
    QQuaternion orientation_;           ///< current orientation
    Qt3DCore::QTransform transform_;    ///< transform matrix
    Qt3DCore::QEntity* probeEntity_;    ///< probe model node
    Qt3DRender::QSceneLoader* probe_;   ///< probe model scene
};
