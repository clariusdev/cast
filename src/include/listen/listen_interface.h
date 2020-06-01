#ifndef LISTEN_INTERFACE_H
#define LISTEN_INTERFACE_H

#include <QtPlugin>

class QObject;
class QString;
class QOpenGLContext;
class QPoint;
class QSize;

namespace cus
{
    //! General callback with the context that was passed in
    typedef void(*CallbackFn)(QObject* context);
    //! Freeze/unfreeze callback with the context that was passed in and the current freeze state
    typedef void(*FreezeFn)(QObject* context, bool freeze);
    //! General callback with the context that was passed in, whether the call succeeded or not, and a message if the call failed
    typedef void(*ReturnFn)(QObject* context, bool success, const QString& errMessage);
    //! General callback with the context that was passed in, whether the call succeeded or not, the UDP port, and a message if the call failed
    typedef void(*ConnectFn)(QObject* context, bool success, int udpPort, const QString& errMessage);

    class ListenInterface
    {
    public:
        //! @brief Destructor
        virtual ~ListenInterface() {}
        //! @brief Set the function that will be called when the connection is dropped
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The function will no longer be called when the context object is destroyed.
        virtual void setDisconnectCallback(
            QObject* context, //!< Valid QObject instance
            cus::CallbackFn disconnected //!< Function to call; passes back the context pointer given
        ) = 0;
        //! @brief Set the function that will be called when the image needs to be redrawn
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The function will no longer be called when the context object is destroyed.
        virtual void setNewImageCallback(
            QObject* context, //!< Valid QObject instance
            cus::CallbackFn newImage //!< Function to call; passes back the context pointer given
        ) = 0;
        //! @brief Set the function that will be called when the freeze state changed
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The function will no longer be called when the context object is destroyed.
        virtual void setFreezeCallback(
            QObject* context, //!< Valid QObject instance
            cus::FreezeFn freeze //!< Function to call; passes back the context pointer given
        ) = 0;
        //! @brief Initialize the library asynchronously (creates SSL temporary files)
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The callback function will not be called if the context object is destroyed.
        virtual void initialize(
            const QString& tempDir, //!< Directory in which to create SSL temporary files
            QObject* context, //!< Valid QObject instance
            cus::ReturnFn fn //!< Function to call when finished; passes back the context pointer given
        ) = 0;
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The callback function will not be called if the context object is destroyed.
        virtual void connect(
            const QString& ipAddress, //!< IP address to connect to (IP v4 or v6)
            int port, //!< TCP port to connect to (listen port)
            QObject* context, //!< Valid QObject instance
            cus::ConnectFn fn //!< Function to call when finished; passes back the context pointer given
        ) = 0;
        //! @brief Disconnect asynchronously
        //! @details The callback will be called from the main Qt thread (by posting an event to the main message loop).
        //!          The callback function will not be called if the context object is destroyed.
        virtual void disconnect(
            QObject* context, //!< Valid QObject instance
            cus::ReturnFn fn //!< Function to call when finished; passes back the context pointer given
        ) = 0;
        //! @brief Render the ultrasound image to the given context
        //! @details This function should be called with the QOpenGLContext made current.
        virtual void render(
            QOpenGLContext* context, //!< Open GL context to render into (made current already)
            const QPoint& point, //!< Top left of rendering (usually 0,0)
            const QSize& size, //!< Size of rendering (usually widget size)
            double devicePixelRatio //! Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays
        ) = 0;
        //! @brief Get the B prescan data for the center frame
        //! @return An image for the prescan center B frame, or null if not received
        virtual QImage bPrescan() = 0;
        //! @brief Clean up the OpenGL textures, etc. that were created prior to rendering
        //! @details This function should be called with the QOpenGLContext used by 'render' made current.
        virtual void cleanupGL() = 0;
    };
}

#define CUS_LISTENINTERFACE_IID "me.clarius.ListenInterface"
Q_DECLARE_INTERFACE(cus::ListenInterface, CUS_LISTENINTERFACE_IID)

#endif // !LISTEN_INTERFACE_H
