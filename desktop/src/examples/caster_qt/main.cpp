#include "caster.h"
#include <memory>
#include <cast/cast.h>
#include <iostream>

static std::unique_ptr<Caster> _caster;
static std::vector<char> _image;
static std::vector<char> _prescanImage;
static std::vector<char> _spectrum;
static std::vector<char> _rfData;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Clarius"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("clarius.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Cast Demo"));

    _caster = std::make_unique<Caster>();
    const int width  = 640; // Width of the rendered image
    const int height = 480; // Height of the rendered image

    if (cusCastInit(argc, argv, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str(),
        // new image callback
        [](const void* img, const CusProcessedImageInfo* nfo, int, const CusPosInfo*)
        {
            int sz = nfo->imageSize;
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            if (_image.size() < static_cast<size_t>(sz))
                _image.resize(sz);
            memcpy(_image.data(), img, sz);

            QApplication::postEvent(_caster.get(), new event::Image(IMAGE_EVENT, _image.data(), nfo->width, nfo->height, nfo->bitsPerPixel, sz));
        },
        // new raw data callback
        [](const void* data, const CusRawImageInfo* nfo, int, const CusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            int sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (nfo->rf)
            {
                if (_rfData.size() < static_cast<size_t>(sz))
                    _rfData.resize(sz);
                memcpy(_rfData.data(), data, sz);
                QApplication::postEvent(_caster.get(), new event::RfImage(_rfData.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, sz, nfo->lateralSize, nfo->axialSize));
            }
            else
            {
                // image may be a jpeg, adjust the size
                if (nfo->jpeg)
                    sz = nfo->jpeg;
                if (_prescanImage.size() < static_cast<size_t>(sz))
                    _prescanImage.resize(sz);
                memcpy(_prescanImage.data(), data, sz);
                QApplication::postEvent(_caster.get(), new event::Image(PRESCAN_EVENT, _prescanImage.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, sz));
            }
        },
        // new spectral image callback
        [](const void* img, const CusSpectralImageInfo* nfo)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            int sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (_spectrum.size() < static_cast<size_t>(sz))
                _spectrum.resize(sz);
            memcpy(_spectrum.data(), img, sz);

            QApplication::postEvent(_caster.get(), new event::Spectrum(_spectrum.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, sz, nfo->period,
                                                                       nfo->micronsPerSample, nfo->velocityPerSample, nfo->pw ? true : false));
        },
        // freeze state change callback
        [](int frozen)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_caster.get(), new event::Freeze(frozen ? true : false));
        },
        // button press callback
        [](CusButton btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_caster.get(), new event::Button(btn, clicks));
        },
        // download progress state change callback
        [](int progress)
        {
            // post event here, as the gui (proress bar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_caster.get(), new event::Progress(progress));
        },
        // error message callback
        [](const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_caster.get(), new event::Error(err));
        },
        width, height) != 0)
    {
        qDebug() << "error initializing listener";
        return -1;
    }

    _caster->show();
    const int result = a.exec();
    cusCastDestroy();
    _caster.reset();
    return result;
}
