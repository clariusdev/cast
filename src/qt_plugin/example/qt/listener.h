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
    class Listener;
}
namespace cus
{
    class ListenInterface;
}

/// ultrasound image display
class UltrasoundImage : public QOpenGLWidget
{
    Q_OBJECT
public:
    UltrasoundImage(cus::ListenInterface* listen, QWidget* parent);

public slots:
    void cleanupGL();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;

private:
    cus::ListenInterface* listen_;     ///< listen interface
};

/// listener gui application
class Listener : public QMainWindow
{
    Q_OBJECT

public:
    explicit Listener(QWidget *parent = nullptr);
    ~Listener();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void newImage();
    void setMessage(const QString& message);
    void setConnected(bool connected);

public slots:
    void onConnect();

private:
    bool connected_;                   ///< connection state
    std::unique_ptr<Ui::Listener> ui_; ///< ui controls, etc.
    cus::ListenInterface* listen_;     ///< listen interface
};
