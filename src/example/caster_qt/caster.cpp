#include "caster.h"
#include "display.h"
#include "ui_caster.h"
#include <cast/cast.h>

static Caster* _me;

/// default constructor
/// @param[in] parent the parent object
Caster::Caster(QWidget *parent) : QMainWindow(parent), connected_(false), ui_(new Ui::Caster)
{
    _me = this;
    ui_->setupUi(this);
    setWindowIcon(QIcon(":/res/cast.png"));
    image_ = new UltrasoundImage(this);
    signal_ = new RfSignal(this);
    ui_->image->addWidget(image_);
    ui_->image->addWidget(signal_);
    imageTimer_.setSingleShot(true);
    connect(&imageTimer_, &QTimer::timeout, [this]()
    {
        image_->setNoImage(true);
        ui_->status->showMessage(QStringLiteral("No Image? Check the O/S Firewall Configuration"));
    });
}

/// destructor
Caster::~Caster()
{
    delete ui_;
}

/// called when the window is closing to clean up the clarius library
void Caster::closeEvent(QCloseEvent*)
{
    if (connected_)
        cusCastDisconnect(nullptr);

    cusCastDestroy();
}

/// handles custom events posted by caster api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Caster::event(QEvent *event)
{
    if (event->type() == IMAGE_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newProcessedImage(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->size_);
        if (imageTimer_.isActive())
        {
            imageTimer_.stop();
            image_->setNoImage(false);
        }
        return true;
    }
    else if (event->type() == PRESCAN_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newPrescanImage(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->size_);
        return true;
    }
    else if (event->type() == RF_EVENT)
    {
        auto evt = static_cast<event::RfImage*>(event);
        newRfData(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->lateral_, evt->axial_);
        return true;
    }
    else if (event->type() == SPECTRUM_EVENT)
    {
        auto evt = static_cast<event::Spectrum*>(event);
        if (evt->pw_)
            newPwSpectrum(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->period_, evt->velocityPerSample_);
        else
            newMSpectrum(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->period_, evt->micronsPerSample_);
        return true;
    }
    else if (event->type() == FREEZE_EVENT)
    {
        setFreeze((static_cast<event::Freeze*>(event))->frozen_);
        return true;
    }
    else if (event->type() == BUTTON_EVENT)
    {
        auto evt = static_cast<event::Button*>(event);
        onButton(evt->button_, evt->clicks_);
        return true;
    }
    else if (event->type() == PROGRESS_EVENT)
    {
        setProgress((static_cast<event::Progress*>(event))->progress_);
        return true;
    }
    else if (event->type() == RAWDATA_EVENT)
    {
        rawDataReady((static_cast<event::RawData*>(event))->success_);
        return true;
    }
    else if (event->type() == ERROR_EVENT)
    {
        setError((static_cast<event::Error*>(event))->error_);
        return true;
    }

    return QMainWindow::event(event);
}

/// called when the api returns an error
/// @param[in] err the error message
void Caster::setError(const QString& err)
{
    ui_->status->showMessage(QStringLiteral("Error: %1").arg(err));
}

/// called when the freeze status changes
/// @param[in] en the freeze state
void Caster::setFreeze(bool en)
{
    ui_->status->showMessage(QStringLiteral("Image: %1").arg(en ? QStringLiteral("Frozen") : QStringLiteral("Running")));
    ui_->freeze->setText(en ? QStringLiteral("Run") : QStringLiteral("Stop"));
    ui_->request->setEnabled(en);
    ui_->download->setEnabled(false);
    ui_->shallower->setEnabled(!en);
    ui_->deeper->setEnabled(!en);
    rawData_ = RawDataInfo();

    if (en)
        imageTimer_.start(3000);
    else
        imageTimer_.stop();
}

/// called when there is a button press on the ultrasound
/// @param[in] btn the button pressed
/// @param[in] clicks # of clicks used
void Caster::onButton(int btn, int clicks)
{
    ui_->status->showMessage(QStringLiteral("Button %1 Pressed, %2 Clicks").arg(btn ? QStringLiteral("Down") : QStringLiteral("Up")).arg(clicks));
}

/// called when the download progress changes
/// @param[in] progress the current progress
void Caster::setProgress(int progress)
{
    ui_->progress->setValue(progress);
}

/// called when a new image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel
/// @param[in] sz size of the image in bytes
void Caster::newProcessedImage(const void* img, int w, int h, int bpp, int sz)
{
    image_->loadImage(img, w, h, bpp, sz);
}

/// called when a new pre-scan image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel
/// @param[in] sz size of the image in bytes
void Caster::newPrescanImage(const void* img, int w, int h, int bpp, int sz)
{
    if (sz == (w * h * (bpp / 8)))
        prescan_ = QImage(reinterpret_cast<const uchar*>(img), w, h, QImage::Format_ARGB32);
    else
        prescan_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");
}

/// called when new rf data has been sent
/// @param[in] rfdata the rf data
/// @param[in] l # of lines
/// @param[in] s # of samples
/// @param[in] bps the bits per sample (should always be 16)
/// @param[in] lateral spacing between lines
/// @param[in] axial sample size
void Caster::newRfData(const void* rfdata, int l, int s, int bps, double lateral, double axial)
{
    signal_->loadSignal(rfdata, l, s, bps / 8);
    Q_UNUSED(lateral)
    Q_UNUSED(axial)
}

/// called when new m spectral data has been sent
/// @param[in] rfdata the rf data
/// @param[in] l # of lines
/// @param[in] s # of samples
/// @param[in] bps the bits per sample
/// @param[in] period seconds per line
/// @param[in] micronsPerSample # of microns per sample
void Caster::newMSpectrum(const void* rfdata, int l, int s, int bps, double period, double micronsPerSample)
{
    Q_UNUSED(rfdata)
    Q_UNUSED(l)
    Q_UNUSED(s)
    Q_UNUSED(bps)
    Q_UNUSED(period)
    Q_UNUSED(micronsPerSample)
}

/// called when new pw spectral data has been sent
/// @param[in] rfdata the rf data
/// @param[in] l # of lines
/// @param[in] s # of samples
/// @param[in] bps the bits per sample
/// @param[in] period seconds per line
/// @param[in] velocityPerSample speed per sample in m/s
void Caster::newPwSpectrum(const void* rfdata, int l, int s, int bps, double period, double velocityPerSample)
{
    Q_UNUSED(rfdata)
    Q_UNUSED(l)
    Q_UNUSED(s)
    Q_UNUSED(bps)
    Q_UNUSED(period)
    Q_UNUSED(velocityPerSample)
}

/// handles the connection result
/// @param[in] res the connection result
void Caster::connected(bool res)
{
    if (res)
    {
        ui_->status->showMessage("Connection successful");
        connected_ = true;
        ui_->connect->setText("Disconnect");
        ui_->freeze->setEnabled(true);
        ui_->shallower->setEnabled(true);
        ui_->deeper->setEnabled(true);
    }
    else
        ui_->status->showMessage("Could not connect to the specified device");
}

/// handles the disconnection result
/// @param[in] res the disconnection result
void Caster::disconnected(bool res)
{
    if (res)
    {
        ui_->status->showMessage("Disconnect successful");
        connected_ = false;
        ui_->connect->setText("Connect");
        ui_->freeze->setEnabled(false);
        ui_->shallower->setEnabled(false);
        ui_->deeper->setEnabled(false);
    }
    else
        ui_->status->showMessage("Could not disconnect");
}

/// called when the connect/disconnect button is clicked
void Caster::onConnect()
{
    if (!connected_)
    {
        if (cusCastConnect(ui_->ip->text().toStdString().c_str(), ui_->port->text().toInt(), [](int ret)
        {
            _me->connected(ret > 0);
        }) < 0)
            ui_->status->showMessage("Connection attempt failed");
    }
    else
    {
        if (cusCastDisconnect([](int ret)
        {
            _me->disconnected(ret == 0);
        }) < 0)
            ui_->status->showMessage("Disconnect attempt failed");
    }
}

/// called when the freeze button is clicked
void Caster::onFreeze()
{
    if (!connected_)
        return;

    if (cusCastUserFunction(Freeze, 0, nullptr) < 0)
        ui_->status->showMessage("Toggle freeze failed");
}

/// called when the shallower button is clicked
void Caster::onShallower()
{
    if (!connected_)
        return;

    if (cusCastUserFunction(DepthDec, 0, nullptr) < 0)
        ui_->status->showMessage("Could not image shallower");
}

/// called when the deeper button is clicked
void Caster::onDeeper()
{
    if (!connected_)
        return;

    if (cusCastUserFunction(DepthInc, 0, nullptr) < 0)
        ui_->status->showMessage("Could not image deeper");
}

/// handles the result of a raw data request
/// @param[in] sz size of the raw data available, or status if not available
void Caster::rawData(int sz)
{
    if (sz < 0)
        ui_->status->showMessage("Error requesting raw data");
    else if (sz == 0)
        ui_->status->showMessage("No raw data currently buffered");
    else
    {
        rawData_.size_ = sz;
        ui_->download->setEnabled(true);
        ui_->status->showMessage(QStringLiteral("Raw data available: %1B").arg(rawData_.size_));
    }
}

/// called when the request raw data button is clicked
/// @note this can only be used while imaging is frozen
void Caster::onRequest()
{
    if (!connected_)
        return;

    if (cusCastRequestRawData(0, 0, [](int sz)
    {
        _me->rawData(sz);
    }) < 0)
        ui_->status->showMessage("Raw data request failed");
}

/// called when the request download button is clicked
/// @note this can only be used once a raw data request has been made
void Caster::onDownload()
{
    if (!connected_)
        return;

    if (rawData_.size_)
    {
        rawData_.file_ = QFileDialog::getSaveFileName(this, QStringLiteral("Save Raw Data"), QDir::homePath() + QLatin1Char('/') + "raw_data.tar", QStringLiteral("(*.tar)"));
        if (rawData_.file_.isEmpty())
            return;

        setProgress(0);

        rawData_.data_.resize(rawData_.size_);
        rawData_.ptr_ = rawData_.data_.data();

        if (cusCastReadRawData((void**)(&rawData_.ptr_), [](int ret)
        {
            // call is complete, post event to manage actual storage
            QApplication::postEvent(_me, new event::RawData(ret < 0 ? false : true));
        }) < 0)
            ui_->status->showMessage("Raw data download failed");
    }
}

/// called when the raw data download is ready
/// @param[in] success the success of downloading the data
/// @return success of the call
bool Caster::rawDataReady(bool success)
{
    if (!success)
    {
        ui_->status->showMessage("Error downloading raw data");
        return false;
    }
    else
    {
        QFile f(rawData_.file_);
        if (!f.open(QIODevice::WriteOnly))
        {
            ui_->status->showMessage("Error opening requested file");
            return false;
        }
        f.write(rawData_.data_);
        f.close();
        ui_->status->showMessage("Successfully downloaded data");
    }

    return true;
}
