#include "listener.h"
#include "ui_listener.h"
#ifdef Clarius_BUILD
    #include <listen_plugin/listen_interface.h>
#else
    #include <listen/listen_interface.h>
#endif

namespace
{
    cus::ListenInterface* loadPlugin()
    {
#ifdef CLARIUS_LISTEN_STATIC
        auto staticPlugins = QPluginLoader::staticInstances();
        for (QObject *plugin : staticPlugins)
        {
            auto listen = qobject_cast<cus::ListenInterface*>(plugin);
            if (listen)
            {
                return listen;
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
                auto listen = qobject_cast<cus::ListenInterface*>(plugin);
                if (listen)
                {
                    return listen;
                }
            }
        }
#endif //  CLARIUS_LISTEN_STATIC
        return nullptr;
    }
}

/// default constructor
/// @param[in] parent the parent object
Listener::Listener(QWidget *parent)
    : QMainWindow(parent)
    , connected_(false)
    , ui_(new Ui::Listener)
    , listen_(loadPlugin())
{
    ui_->setupUi(this);
    ui_->image->addWidget(new UltrasoundImage(listen_, this), 1);
    if (listen_)
    {
        setMessage(QStringLiteral("Plugin loaded successfully"));
        listen_->setDisconnectCallback(this, [](QObject* listener)
        {
            auto thisPtr = static_cast<Listener*>(listener);
            thisPtr->setMessage(QStringLiteral("Connection dropped"));
            thisPtr->setConnected(false);
        });
        listen_->setFreezeCallback(this, [](QObject* listener, bool freeze)
        {
            QString state = freeze ? QStringLiteral("Frozen") : QStringLiteral("Running");
            static_cast<Listener*>(listener)->setMessage(QStringLiteral("Image: ") + state);
        });
        listen_->initialize(QCoreApplication::applicationDirPath(), this, [](QObject* listener, bool success, const QString& err)
        {
            auto thisPtr = static_cast<Listener*>(listener);
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
Listener::~Listener()
{
}

/// called when the window is closing to clean up the clarius library
void Listener::closeEvent(QCloseEvent* event)
{
    if (listen_)
    {
        // Don't care about when this finishes (hence no callback)
        listen_->disconnect(nullptr, nullptr);
    }
    QMainWindow::closeEvent(event);
}

/// called to update the status bar message
/// @param[in] message the new status bar message
void Listener::setMessage(const QString& message)
{
    ui_->status->showMessage(message);
}

/// called to update the connected/disconnected state
/// @param[in] connected true if the listener is currently connected
void Listener::setConnected(bool connected)
{
    if (connected_ == connected)
    {
        return;
    }
    connected_ = connected;
    ui_->connect->setText(connected ? QStringLiteral("Disconnect") : QStringLiteral("Connect"));

}

/// called when the connect/disconnect button is clicked
void Listener::onConnect()
{
    if (!listen_)
    {
        return;
    }
    if (!connected_)
    {
        listen_->connect(ui_->ip->text(), ui_->port->text().toInt(), this, [](QObject* listener, bool success, int udpPort, const QString& err)
        {
            auto thisPtr = static_cast<Listener*>(listener);
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
        listen_->disconnect(this, [](QObject* listener, bool success, const QString& err)
        {
            auto thisPtr = static_cast<Listener*>(listener);
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
/// @param[in] listen the listen plugin interface, if loaded
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(cus::ListenInterface* listen, QWidget* parent)
    : QOpenGLWidget(parent)
    , listen_(listen)
{
    if (listen_)
    {
        listen_->setNewImageCallback(this, [](QObject* image)
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
    // Listening to the context distroyed signal to allow the plugin clean up GL
    connect(glContext, &QOpenGLContext::aboutToBeDestroyed, this, &UltrasoundImage::cleanupGL);
}

/// function to initialize OpenGL
void UltrasoundImage::cleanupGL()
{
    if (listen_)
    {
        // Have to release all resources with the QOpenGLContext made current
        makeCurrent();
        listen_->cleanupGL();
        doneCurrent();
    }
}

void UltrasoundImage::paintGL()
{
    if (listen_)
    {
        listen_->render(context(), QPoint(0, 0), size(), devicePixelRatioF());
    }
}
