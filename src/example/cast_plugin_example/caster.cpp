#include "caster.h"
#include "ui_caster.h"
#ifdef Clarius_BUILD
    #include <cast_plugin/cast_interface.h>
#else
    #include <cast/cast_interface.h>
#endif

namespace
{
    cus::CastInterface* loadPlugin()
    {
#ifdef CLARIUS_CAST_STATIC
        auto staticPlugins = QPluginLoader::staticInstances();
        for (QObject *plugin : staticPlugins)
        {
            auto cast = qobject_cast<cus::CastInterface*>(plugin);
            if (cast)
            {
                return cast;
            }
        }
#else
        QDir appDir = QCoreApplication::applicationDirPath();
        auto possiblePlugins = appDir.entryList(QDir::Files);
        for (QString fileName : possiblePlugins)
        {
            QPluginLoader loader(appDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (plugin)
            {
                auto cast = qobject_cast<cus::CastInterface*>(plugin);
                if (cast)
                {
                    return cast;
                }
            }
        }
#endif //  CLARIUS_CAST_STATIC
        return nullptr;
    }
}

/// default constructor
/// @param[in] parent the parent object
Caster::Caster(QWidget *parent)
    : QMainWindow(parent)
    , connected_(false)
    , ui_(new Ui::Caster)
    , cast_(loadPlugin())
{
    ui_->setupUi(this);
    ui_->image->addWidget(new UltrasoundImage(cast_, this), 1);
    if (cast_)
    {
        setMessage(QStringLiteral("Plugin loaded successfully"));
        cast_->setDisconnectCallback(this, [](QObject* caster)
        {
            auto thisPtr = static_cast<Caster*>(caster);
            thisPtr->setMessage(QStringLiteral("Connection dropped"));
            thisPtr->setConnected(false);
        });
        cast_->setFreezeCallback(this, [](QObject* caster, bool freeze)
        {
            QString state = freeze ? QStringLiteral("Frozen") : QStringLiteral("Running");
            static_cast<Caster*>(caster)->setMessage(QStringLiteral("Image: ") + state);
        });
        cast_->initialize(QCoreApplication::applicationDirPath(), this, [](QObject* caster, bool success, const QString& err)
        {
            auto thisPtr = static_cast<Caster*>(caster);
            if (success)
            {
                thisPtr->setMessage(QStringLiteral("Initialized plugin"));
            }
            else
            {
                thisPtr->setMessage(QStringLiteral("Failed to initialize plugin: ") + err);
            }
        });
    }
    else
    {
        setMessage(QStringLiteral("Failed to load plugin"));
    }
}

/// destructor
Caster::~Caster()
{
}

/// called when the window is closing to clean up the clarius library
void Caster::closeEvent(QCloseEvent* event)
{
    if (cast_)
    {
        // Don't care about when this finishes (hence no callback)
        cast_->disconnect(nullptr, nullptr);
    }
    QMainWindow::closeEvent(event);
}

/// called to update the status bar message
/// @param[in] message the new status bar message
void Caster::setMessage(const QString& message)
{
    ui_->status->showMessage(message);
}

/// called to update the connected/disconnected state
/// @param[in] connected true if the caster is currently connected
void Caster::setConnected(bool connected)
{
    if (connected_ == connected)
    {
        return;
    }
    connected_ = connected;
    ui_->connect->setText(connected ? QStringLiteral("Disconnect") : QStringLiteral("Connect"));

}

/// called when the connect/disconnect button is clicked
void Caster::onConnect()
{
    if (!cast_)
    {
        return;
    }
    if (!connected_)
    {
        cast_->connect(ui_->ip->text(), ui_->port->text().toInt(), this, [](QObject* caster, bool success, int udpPort, const QString& err)
        {
            auto thisPtr = static_cast<Caster*>(caster);
            if (success)
            {
                thisPtr->setMessage(QStringLiteral("Connection successful, UDP port is %1").arg(udpPort));
                thisPtr->setConnected(true);
            }
            else
            {
                thisPtr->setMessage(QStringLiteral("Connection failed with error ") + err);
            }
        });
    }
    else
    {
        cast_->disconnect(this, [](QObject* caster, bool success, const QString& err)
        {
            auto thisPtr = static_cast<Caster*>(caster);
            thisPtr->setConnected(false);
            if (success)
            {
                thisPtr->setMessage(QStringLiteral("Disconnect successful"));
            }
            else
            {
                thisPtr->setMessage(QStringLiteral("Disconnect failed with error ") + err);
            }
        });
    }
}

/// default constructor
/// @param[in] cast the cast plugin interface, if loaded
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(cus::CastInterface* cast, QWidget* parent)
    : QOpenGLWidget(parent)
    , cast_(cast)
{
    if (cast_)
    {
        cast_->setNewImageCallback(this, [](QObject* image)
        {
            static_cast<UltrasoundImage*>(image)->update();
        });
    }
}

/// function to initialize OpenGL
void UltrasoundImage::initializeGL()
{
    auto glContext = context();
    // Setting the background color to black
    glContext->functions()->glClearColor(0, 0, 0, 1.0f);
    // Casting to the context distroyed signal to allow the plugin clean up GL
    connect(glContext, &QOpenGLContext::aboutToBeDestroyed, this, &UltrasoundImage::cleanupGL);
}

/// function to initialize OpenGL
void UltrasoundImage::cleanupGL()
{
    if (cast_)
    {
        // Have to release all resources with the QOpenGLContext made current
        makeCurrent();
        cast_->cleanupGL();
        doneCurrent();
    }
}

void UltrasoundImage::paintGL()
{
    if (cast_)
    {
        cast_->render(context(), QPoint(0, 0), size(), devicePixelRatioF());
    }
}
