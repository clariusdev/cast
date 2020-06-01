#include "listener.h"
#include <memory>
#include <listen/listen.h>
#include <iostream>

#ifdef Clarius_BUILD
#define Clarius_IMPORT_QT_WIDGETS_LIB
#define Clarius_IMPORT_QT_NETWORK_LIB
#include <cus/qtplugins.h>
#endif

static std::unique_ptr<Listener> _listener;
static std::vector<char> _image;
static std::vector<char> _rawImage;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    _listener = std::make_unique<Listener>();
    const int width  = 640; // Width of the rendered image
    const int height = 480; // Height of the rendered image

    if (clariusInitListener(argc, argv, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str(),
        // new image callback
        [](const void* img, const ClariusProcessedImageInfo* nfo, int, const ClariusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            size_t sz = nfo->width * nfo->height * (nfo->bitsPerPixel / 8);
            if (_image.size() <  sz)
                _image.resize(sz);
            memcpy(_image.data(), img, sz);

            QApplication::postEvent(_listener.get(), new event::Image(_image.data(), nfo->width, nfo->height, nfo->bitsPerPixel));
        },
        // new raw image callback
        [](const void* img, const ClariusRawImageInfo* nfo, int, const ClariusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            size_t sz = nfo->lines * nfo->samples* (nfo->bitsPerSample / 8);
            if (_rawImage.size() <  sz)
                _rawImage.resize(sz);
            memcpy(_rawImage.data(), img, sz);

            QApplication::postEvent(_listener.get(), new event::PreScanImage(_rawImage.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, nfo->jpeg));
        },
        // freeze state change callback
        [](int frozen)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_listener.get(), new event::Freeze(frozen ? true : false));
        },
        // button press callback
        [](int btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_listener.get(), new event::Button(btn, clicks));
        },
        // download progress state change callback
        [](int progress)
        {
            // post event here, as the gui (proress bar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_listener.get(), new event::Progress(progress));
        },
        // error message callback
        [](const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_listener.get(), new event::Error(err));
        },
        nullptr, width, height) != 0)
    {
        qDebug() << "error initializing listner";
        return -1;
    }

    _listener->show();
    return a.exec();
}
