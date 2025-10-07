#include "caster.h"
#include "display.h"
#include "3d.h"
#include "ui_caster.h"
#include <cast/cast.h>

static Caster* _me;

/// default constructor
/// @param[in] parent the parent object
Caster::Caster(QWidget *parent) : QMainWindow(parent), connected_(false), frozen_(false), lasttime_(0), imuSamples_(0), ui_(new Ui::Caster)
{
    _me = this;
    ui_->setupUi(this);
    setWindowIcon(QIcon(":/res/cast.png"));
    image_ = new UltrasoundImage(this);
    signal_ = new RfSignal(this);
    ui_->image->addWidget(image_);
    ui_->image->addWidget(signal_);
    imageTimer_.setSingleShot(true);

    render_ = new ProbeRender(QGuiApplication::primaryScreen());
    ui_->render->addWidget(QWidget::createWindowContainer(render_));
    auto reset = new QPushButton(QStringLiteral("Reset"), this);
    ui_->render->addWidget(reset);
    render_->init(QStringLiteral(":/res/l15.obj"));
    render_->show();

    settings_ = std::make_unique<QSettings>(QStringLiteral("settings.ini"), QSettings::IniFormat);
    auto ip = settings_->value("ip").toString();
    if (!ip.isEmpty())
        ui_->ip->setText(ip);
    auto port = settings_->value("port").toString();
    if (!port.isEmpty())
        ui_->port->setText(port);

    connect(&imageTimer_, &QTimer::timeout, [this]()
    {
        image_->setNoImage(true);
        lasttime_ = 0;
        updateCaptureButtons();
        ui_->status->showMessage(NO_IMAGE_STATEMENT);
    });

    QObject::connect(reset, &QPushButton::clicked, [this]()
    {
        render_->reset();
        imuSamples_ = 0;
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
        castDisconnect(nullptr);

    castDestroy();
}

/// handles custom events posted by caster api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Caster::event(QEvent *event)
{
    if (event->type() == IMAGE_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newProcessedImage(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->size_, evt->imu_);
        image_->setNoImage(false);
        lasttime_ = evt->tm_;
        updateCaptureButtons();
        if (imageTimer_.isActive())
            imageTimer_.stop();

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
    else if (event->type() == IMU_EVENT)
    {
        auto evt = static_cast<event::Imu*>(event);
        newImuData(evt->imu_);
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
    frozen_ = en;
    if (!frozen_)
        lasttime_ = 0;
    ui_->status->showMessage(QStringLiteral("Image: %1").arg(en ? QStringLiteral("Frozen") : QStringLiteral("Running")));
    ui_->freeze->setText(en ? QStringLiteral("Run") : QStringLiteral("Stop"));
    ui_->request->setEnabled(en);
    ui_->download->setEnabled(false);
    ui_->shallower->setEnabled(!en);
    ui_->deeper->setEnabled(!en);
    updateCaptureButtons();
    rawData_ = RawDataInfo();

    if (!en)
        imageTimer_.start(3000);
    else
        imageTimer_.stop();
}

void Caster::updateCaptureButtons()
{
    ui_->addLabel->setEnabled(lasttime_ != 0);
    ui_->captureImage->setEnabled(lasttime_ != 0);
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
void Caster::newProcessedImage(const void* img, int w, int h, int bpp, int sz, const QQuaternion& imu)
{
    image_->loadImage(img, w, h, bpp, sz);
    if (!imu.isNull())
        render_->update(imu);
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
/// @param[in] imagePort the image udp streaming port, 0 if failure
/// @param[in] imuPort the imu udp streaming port, 0 if failure
void Caster::connected(int imagePort, int imuPort)
{
    if (imagePort > 0)
    {
        ui_->status->showMessage(QString("Connection successful, streaming port: %1, imu port: %2").arg(imagePort).arg(imuPort));
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
        if (castConnect(ui_->ip->text().toStdString().c_str(), ui_->port->text().toInt(), "research",
            [](int imagePort, int imuPort, int swRevMatch)
        {
            _me->ui_->swRevMatch->setText((swRevMatch == CUS_SUCCESS) ? "Matches" : "Mismatch");
            _me->connected(imagePort, imuPort);
        }) < 0)
        {
            ui_->status->showMessage("Connection attempt failed");
        }

        settings_->setValue("ip", ui_->ip->text());
        settings_->setValue("port", ui_->port->text());
    }
    else
    {
        if (castDisconnect([](int ret)
        {
            _me->disconnected(ret == CUS_SUCCESS);
        }) < 0)
            ui_->status->showMessage("Disconnect attempt failed");
    }
}

/// called when the freeze button is clicked
void Caster::onFreeze()
{
    if (!connected_)
        return;

    if (castUserFunction(Freeze, 0, nullptr) < 0)
        ui_->status->showMessage("Toggle freeze failed");
}

/// called when the shallower button is clicked
void Caster::onShallower()
{
    if (!connected_)
        return;

    if (castUserFunction(DepthDec, 0, nullptr) < 0)
        ui_->status->showMessage("Could not image shallower");
}

/// called when the deeper button is clicked
void Caster::onDeeper()
{
    if (!connected_)
        return;

    if (castUserFunction(DepthInc, 0, nullptr) < 0)
        ui_->status->showMessage("Could not image deeper");
}

/// called when the addLabel button is clicked
void Caster::onAddLabel()
{
    QString label = ui_->labelText->text();
    if (label.isEmpty())
    {
        ui_->status->showMessage("No label text provided");
        return;
    }
    image_->addLabel(label);
}

/// called when the addTrace button is clicked
void Caster::onAddTrace()
{
    QString label = ui_->labelText->text();
    if (label.isEmpty())
    {
        ui_->status->showMessage("No label text provided");
        return;
    }
    image_->addTrace(label);
}

/// called when the captureImage button is clicked
void Caster::onCaptureImage()
{
    if (lasttime_ == 0)
    {
        ui_->status->showMessage("No image to capture");
        return;
    }
    const int captureID = castStartCapture(lasttime_);
    if (captureID < 0)
    {
        ui_->status->showMessage("Failed to initialize capture");
        return;
    }
    const std::vector<LabelInfo> labels = image_->getLabels();
    for (const LabelInfo& label : labels)
    {
        const std::string text = label.text_.toStdString();
        const QPointF center = label.rect_.center();
        if (castAddLabelOverlay(captureID, text.c_str(), center.x(), center.y(), label.rect_.width(), label.rect_.height()) < 0)
            ui_->status->showMessage("Failed to add label " + label.text_ + " to capture");
    }
    const std::vector<TraceInfo> traces = image_->getTraces();
    for (const TraceInfo& trace : traces)
    {
        const std::string text = trace.text_.toStdString();
        std::vector<double> points;
        for (const QPointF& pt : trace.points_)
        {
            points.push_back(pt.x());
            points.push_back(pt.y());
        }
        if (castAddMeasurement(captureID, CusMeasurementTypeTraceDistance, text.c_str(), points.data(), static_cast<int>(points.size())) < 0)
            ui_->status->showMessage("Failed to add trace measurement " + trace.text_ + " to capture");
    }
    const QImage overlayImage = image_->overlayImage();
    if (!overlayImage.isNull())
    {
        const QColor overlayColor = image_->overlayColor();
        const int height = overlayImage.height();
        const int width = overlayImage.width();
        std::vector<unsigned char> bytes(width * height);
        for (int i = 0; i < height; ++i)
        {
            std::memcpy(bytes.data() + i * width, overlayImage.scanLine(i), width);
        }
        castAddImageOverlay(
            captureID,
            bytes.data(),
            width,
            height,
            static_cast<float>(overlayColor.redF()),
            static_cast<float>(overlayColor.greenF()),
            static_cast<float>(overlayColor.blueF()),
            static_cast<float>(overlayColor.alphaF())
        );
    }
    if (castFinishCapture(captureID, [](int ret)
    {
        if (ret < 0)
            _me->ui_->status->showMessage("Failed to send capture");
        else
            _me->ui_->status->showMessage("Sent capture");
    }) < 0)
    {
        ui_->status->showMessage("Failed to finish capture");
    }
}

/// called when the clearScreen button is clicked
void Caster::onClearScreen()
{
    image_->clearOverlays();
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

    if (castRequestRawData(0, 0, ui_->lzo->isChecked() ? 1 : 0, [](int sz, const char*)
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

        if (castReadRawData((void**)(&rawData_.ptr_), [](int ret)
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

/// called when a new imu data been sent
/// @param[in] imu the imu data if valid
void Caster::newImuData(const QQuaternion& imu)
{
    if (!imu.isNull())
    {
        render_->update(imu);
        ui_->imuData->setText(QStringLiteral("Collected %1 IMU Samples").arg(++imuSamples_));
    }
}
