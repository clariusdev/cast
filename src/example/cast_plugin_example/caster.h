#pragma once

#ifdef __clang__
    #pragma clang diagnostic push
#elif _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4512)
    #pragma warning(disable: 4510)
    #pragma warning(disable: 4610)
    #pragma warning(disable: 4458)
    #pragma warning(disable: 4800)
#endif

#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <memory>

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif

namespace Ui
{
    class Caster;
}
namespace cus
{
    class CastInterface;
}

/// ultrasound image display
class UltrasoundImage : public QOpenGLWidget
{
    Q_OBJECT
public:
    UltrasoundImage(cus::CastInterface* cast, QWidget* parent);

public slots:
    void cleanupGL();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;

private:
    cus::CastInterface* cast_;  ///< cast interface
};

/// caster gui application
class Caster : public QMainWindow
{
    Q_OBJECT

public:
    explicit Caster(QWidget *parent = nullptr);
    ~Caster() override;

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void newImage();
    void setMessage(const QString& message);
    void setConnected(bool connected);

public slots:
    void onConnect();

private:
    bool connected_;                    ///< connection state
    std::unique_ptr<Ui::Caster> ui_;    ///< ui controls, etc.
    cus::CastInterface* cast_;          ///< cast interface
};
