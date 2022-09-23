#include "display.h"
#include <cast/cast.h>

/// default constructor
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(QWidget* parent) : QGraphicsView(parent), noImage_(false)
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
/// @param[in] bpp bits per pixel
/// @param[in] sz size of image in bytes
void UltrasoundImage::loadImage(const void* img, int w, int h, int bpp, int sz)
{
    // check for size match
    if (image_.width() != w || image_.height() != h)
        return;

    // set the image data
    lock_.lock();
    // check that the size matches the dimensions (uncompressed)
    if (sz == (w * h * (bpp / 8)))
        memcpy(image_.bits(), img, w * h * (bpp / 8));
    // try to load jpeg
    else
        image_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");
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
    cusCastSetOutputSize(w, h);

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

    if (noImage_)
    {
        painter->setFont(QFont(QStringLiteral("Arial"), 12));
        painter->setPen(Qt::white);
        painter->drawText(r, Qt::AlignCenter | Qt::AlignBottom, NO_IMAGE_STATEMENT);
    }

    lock_.unlock();
}

/// default constructor
/// @param[in] parent the parent object
RfSignal::RfSignal(QWidget* parent) : QGraphicsView(parent), zoom_(0.1)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);
    setVisible(false);

    setSceneRect(0, 0, width(), height());

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// loads new rf signal
/// @param[in] rf the new rf data
/// @param[in] l # of rf lines
/// @param[in] s # of samples per line
/// @param[in] ss sample size in bytes
void RfSignal::loadSignal(const void* rf, int l, int s, int ss)
{
    if (!rf || !l || !s || ss != 2)
        return;

    if (!isVisible())
        setVisible(true);

    // pick the center line to display
    lock_.lock();
    signal_.clear();
    const int16_t* buf = static_cast<const int16_t*>(rf) + ((l / 2) * s);
    for (auto i = 0; i < s; i++)
        signal_.push_back(*buf++);
    lock_.unlock();

    // redraw
    scene()->invalidate();
}

/// sets the zoom scaling to display the rf signal
/// @param[in] zoom the zoom percentage
void RfSignal::setZoom(int zoom)
{
    zoom_ = (static_cast<qreal>(zoom) / 100.0);
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void RfSignal::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();
    setSceneRect(0, 0, w, h);
    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int RfSignal::heightForWidth(int w) const
{
    // keep 4:1 aspect ratio
    double ratio = 1.0 / 4.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize RfSignal::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void RfSignal::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the rf signal
/// @param[in] painter the drawing context
/// @param[in] r the view rectangle
void RfSignal::drawForeground(QPainter* painter, const QRectF& r)
{
    if (!signal_.isEmpty())
    {
        lock_.lock();
        painter->setPen(QColor(96, 96, 0));
        qreal x = 0, baseline = r.height() / 2;
        double sampleSize = static_cast<double>(r.width()) / static_cast<double>(signal_.size());
        QPointF p(x, baseline);
        for (auto s : signal_)
        {
            qreal y = s * zoom_;
            QPointF pt(x + sampleSize, baseline + y);
            painter->drawLine(p, pt);
            p = pt;
            x = x + sampleSize;
        }
        lock_.unlock();
    }
}
