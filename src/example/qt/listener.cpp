#include "listener.h"
#include "ui_listener.h"
#include <listen/listen.h>

static Listener* _me;

/// default constructor
/// @param[in] parent the parent object
Listener::Listener(QWidget *parent) : QMainWindow(parent), connected_(false), ui_(new Ui::Listener)
{
    _me = this;
    ui_->setupUi(this);
    image_ = new UltrasoundImage(this);
    ui_->image->addWidget(image_);
}

/// destructor
Listener::~Listener()
{
    delete ui_;
}

/// called when the window is closing to clean up the clarius library
void Listener::closeEvent(QCloseEvent*)
{
    if (connected_)
        clariusDisconnect(nullptr);

    clariusDestroyListener();
}

/// handles custom events posted by listener api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Listener::event(QEvent *event)
{
    if (event->type() == IMAGE_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newProcessedImage(evt->data(), evt->width(), evt->height(), evt->bpp());
        return true;
    }
    if (event->type() == PRESCAN_EVENT)
    {
        auto evt = static_cast<event::PreScanImage*>(event);
        newRawImage(evt->data(), evt->width(), evt->height(), evt->bpp(), evt->jpeg());
        return true;
    }
    else if (event->type() == FREEZE_EVENT)
    {
        setFreeze((static_cast<event::Freeze*>(event))->frozen());
        return true;
    }
    else if (event->type() == BUTTON_EVENT)
    {
        auto evt = static_cast<event::Button*>(event);
        onButton(evt->button(), evt->clicks());
        return true;
    }
    else if (event->type() == PROGRESS_EVENT)
    {
        setProgress((static_cast<event::Progress*>(event))->progress());
        return true;
    }
    else if (event->type() == RAWDATA_EVENT)
    {
        rawDataReady((static_cast<event::RawData*>(event))->success());
        return true;
    }
    else if (event->type() == ERROR_EVENT)
    {
        setError((static_cast<event::Error*>(event))->error());
        return true;
    }

    return QMainWindow::event(event);
}

/// called when the api returns an error
/// @param[in] err the error message
void Listener::setError(const QString& err)
{
    ui_->status->showMessage(QStringLiteral("Error: %1").arg(err));
}

/// called when the freeze status changes
/// @param[in] en the freeze state
void Listener::setFreeze(bool en)
{
    ui_->status->showMessage(QStringLiteral("Image: %1").arg(en ? QStringLiteral("Frozen") : QStringLiteral("Running")));
    ui_->freeze->setText(en ? QStringLiteral("Run") : QStringLiteral("Stop"));
    ui_->request->setEnabled(en);
    ui_->download->setEnabled(false);
    ui_->shallower->setEnabled(!en);
    ui_->deeper->setEnabled(!en);
    rawData_ = RawDataInfo();
}

/// called when there is a button press on the ultrasound
/// @param[in] btn the button pressed
/// @param[in] clicks # of clicks used
void Listener::onButton(int btn, int clicks)
{
    ui_->status->showMessage(QStringLiteral("Button %1 Pressed, %2 Clicks").arg(btn ? QStringLiteral("Down") : QStringLiteral("Up")).arg(clicks));
}

/// called when the download progress changes
/// @param[in] progress the current progress
void Listener::setProgress(int progress)
{
    ui_->progress->setValue(progress);
}

/// called when a new image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel (should always be 8)
void Listener::newProcessedImage(const void* img, int w, int h, int bpp)
{
    image_->loadImage(img, w, h, bpp);
}

/// called when a new pre-scan image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel (should always be 8)
/// @param[in] jpg flag if the data is jpeg compressed
void Listener::newRawImage(const void* img, int w, int h, int bpp, bool jpg)
{
    Q_UNUSED(bpp)
    Q_UNUSED(jpg)
    prescan_ = QImage(reinterpret_cast<const uchar*>(img), w, h, QImage::Format_ARGB32);
}

/// called when the connect/disconnect button is clicked
void Listener::onConnect()
{
    if (!connected_)
    {
        if (clariusConnect(ui_->ip->text().toStdString().c_str(), ui_->port->text().toInt(), nullptr) < 0)
            ui_->status->showMessage("Could not connect to the specified device");
        else
        {
            ui_->status->showMessage("Connection successful");
            connected_ = true;
            ui_->connect->setText("Disconnect");
            ui_->freeze->setEnabled(true);
            ui_->shallower->setEnabled(true);
            ui_->deeper->setEnabled(true);
        }
    }
    else
    {
        if (clariusDisconnect(nullptr) < 0)
            ui_->status->showMessage("Could not disconnect");
        else
        {
            ui_->status->showMessage("Disconnect successful");
            connected_ = false;
            ui_->connect->setText("Connect");
            ui_->freeze->setEnabled(false);
            ui_->shallower->setEnabled(false);
            ui_->deeper->setEnabled(false);
        }
    }
}

/// called when the freeze button is clicked
void Listener::onFreeze()
{
    if (!connected_)
        return;

    if (clariusUserFunction(USER_FN_TOGGLE_FREEZE, nullptr) < 0)
        ui_->status->showMessage("Could not freeze/unfreeze scanner");
}

/// called when the shallower button is clicked
void Listener::onShallower()
{
    if (!connected_)
        return;

    if (clariusUserFunction(USER_FN_DEPTH_DEC, nullptr) < 0)
        ui_->status->showMessage("Could not image shallower");
}

/// called when the deeper button is clicked
void Listener::onDeeper()
{
    if (!connected_)
        return;

    if (clariusUserFunction(USER_FN_DEPTH_INC, nullptr) < 0)
        ui_->status->showMessage("Could not image deeper");
}

/// called when the request raw data button is clicked
/// @note this can only be used while imaging is frozen
void Listener::onRequest()
{
    if (!connected_)
        return;

    int sz = clariusRequestRawData(0, 0, nullptr);
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

/// called when the request download button is clicked
/// @note this can only be used once a raw data request has been made
void Listener::onDownload()
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

        // don't use a blocking call here so we can update the gui with download progress
        clariusReadRawData((void**)(&rawData_.ptr_), [](int ret)
        {
            // call is complete, post event to manage actual storage
            QApplication::postEvent(_me, new event::RawData(ret < 0 ? false : true));
        });
    }
}

/// called when the raw data download is ready
/// @param[in] success the success of downloading the data
/// @return success of the call
bool Listener::rawDataReady(bool success)
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

/// default constructor
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(QWidget* parent) : QGraphicsView(parent)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);

    // initialize image to some arbitrary size
    image_ = QImage(320, 240, QImage::Format_ARGB32);
    image_.fill(Qt::black);
    setSceneRect(0, 0, image_.width(), image_.height());

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// loads a new image from raw data
/// @param[in] img the new image data
/// @param[in] w the image width
/// @param[in] h the image height
void UltrasoundImage::loadImage(const void* img, int w, int h, int bpp)
{
    // check for size match
    if (image_.width() != w || image_.height() != h)
        return;

    // set the image data
    lock_.lock();
    memcpy(image_.bits(), img, w * h * (bpp / 8));
    lock_.unlock();

    // redraw
    scene()->invalidate();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void UltrasoundImage::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    clariusSetOutputSize(w, h);

    lock_.lock();
    image_ = QImage(w, h, QImage::Format_ARGB32);
    image_.fill(Qt::black);
    lock_.unlock();

    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int UltrasoundImage::heightForWidth(int w) const
{
    // keep a proper aspect 4:3 ratio
    double ratio = 3.0 / 4.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize UltrasoundImage::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void UltrasoundImage::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the target image
/// @param[in] painter the drawing context
void UltrasoundImage::drawForeground(QPainter* painter, const QRectF& r)
{
    lock_.lock();
    if (!image_.isNull())
        painter->drawImage(r, image_);
    lock_.unlock();
}
