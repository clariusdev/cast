#include "display.h"
#include <cast/cast.h>

namespace
{
    enum class AddTo
    {
        front,
        back
    };
}

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
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    overlayColor_ = QColor(255, 127, 80, 100);

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
    // check that the size matches the dimensions (uncompressed)
    if (sz == (w * h * (bpp / 8)))
        std::memcpy(image_.bits(), img, w * h * (bpp / 8));
    // try to load jpeg
    else
        image_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");

    // redraw
    scene()->invalidate();
}

namespace
{
    QGraphicsItem* createLabel(const QString& text, QGraphicsScene* scenePtr, const QPointF& startPos)
    {
        QGraphicsSimpleTextItem* childText = scenePtr->addSimpleText(text);
        childText->setBrush(Qt::white);
        const QSizeF textSize = childText->boundingRect().size();
        const qreal halfHeight = textSize.height() / 2.0;
        const QSizeF rectSize = textSize + QSizeF(halfHeight, halfHeight);
        const QPointF topLeft = startPos - QPointF(rectSize.width(), rectSize.height()) / 2.0;
        QGraphicsRectItem* child = scenePtr->addRect(QRectF(topLeft, rectSize), QPen(), QBrush(QColor(0, 0, 0, 127)));
        childText->setParentItem(child);
        childText->setPos(topLeft + QPointF(halfHeight / 2.0, halfHeight / 2.0));
        return child;
    }
    QPointF getCenterPos(QGraphicsItem* item)
    {
        return item->boundingRect().center() + item->scenePos();
    }
    class CaliperItem : public QGraphicsRectItem
    {
    public:
        CaliperItem(const QRectF& rect, AddTo addto, std::shared_ptr<PointList> trace)
            : QGraphicsRectItem(rect)
            , addto_{addto}
            , trace_{std::move(trace)}
        {}
    protected:
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override
        {
            const QVariant result = QGraphicsRectItem::itemChange(change, value);
            if (change != QGraphicsItem::ItemPositionHasChanged)
                return result;

            const QPointF center = getCenterPos(this);
            QGraphicsItem* compareNearest = nullptr;
            QPointF pointNearest;
            if (!trace_->empty())
            {
                compareNearest = (addto_ == AddTo::front ? trace_->front() : trace_->back());
                pointNearest = getCenterPos(compareNearest);
            }
            QGraphicsScene* scenePtr = scene();
            if (trace_->size() > 1)
            {
                QGraphicsItem* compareNext = (addto_ == AddTo::front ? trace_->at(1) : trace_->at(trace_->size() - 2));
                const QPointF pointNext = getCenterPos(compareNext);
                if (QLineF(center, pointNext).length() < 20.0 &&
                    QLineF(center, pointNearest).length() < 20.0)
                {
                    scenePtr->removeItem(compareNearest);
                    delete compareNearest;
                    compareNearest = nullptr;
                    if (addto_ == AddTo::front)
                    {
                        trace_->pop_front();
                    }
                    else
                    {
                        trace_->pop_back();
                    }
                    return result;
                }
            }
            if (compareNearest && QLineF(center, pointNearest).length() < 20.0)
                return result;
            QGraphicsItem* newItem = scenePtr->addRect(QRectF(center - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), QPen(QColor(0, 255, 0)), QBrush(QColor(0, 255, 0)));
            if (addto_ == AddTo::front)
            {
                trace_->push_front(newItem);
            }
            else
            {
                trace_->push_back(newItem);
            }
            scenePtr->invalidate();
            return result;
        }
    private:
        AddTo addto_;
        std::shared_ptr<PointList> trace_;
    };
    QGraphicsRectItem* createCaliper(QGraphicsScene* scenePtr, const QSizeF& mainSize, AddTo addto, std::shared_ptr<PointList> trace)
    {
        const qreal height = qMin(mainSize.height(), mainSize.width()) / 10.0;
        const QPointF startPos(mainSize.width() / 2.0, mainSize.height() / 2.0);
        const QPointF topLeft = startPos - QPointF(height, height) / 2.0;
        const QRectF rect(topLeft, QSizeF(height, height));
        QGraphicsRectItem* child = new CaliperItem(rect, addto, trace);
        child->setPen(QPen(QColor(0, 0, 0, 0)));
        scenePtr->addItem(child);
        const QPointF center = rect.center();
        const qreal y1 = rect.top() + rect.height() / 3.0;
        const qreal y2 = rect.top() + rect.height() * 2.0 / 3.0;
        const QPen linePen(QColor(0, 255, 0));
        QGraphicsLineItem* line = scenePtr->addLine(QLineF(QPointF(center.x(), rect.top()), QPointF(center.x(), y1)), linePen);
        line->setParentItem(child);
        line = scenePtr->addLine(QLineF(QPointF(center.x(), y2), QPointF(center.x(), rect.bottom())), linePen);
        line->setParentItem(child);
        const qreal x1 = rect.left() + rect.width() / 3.0;
        const qreal x2 = rect.left() + rect.width() * 2.0 / 3.0;
        line = scenePtr->addLine(QLineF(QPointF(rect.left(), center.y()), QPointF(x1, center.y())), linePen);
        line->setParentItem(child);
        line = scenePtr->addLine(QLineF(QPointF(x2, center.y()), QPointF(rect.right(), center.y())), linePen);
        line->setParentItem(child);
        child->setFlag(QGraphicsItem::ItemIsMovable);
        child->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        return child;
    }
}

void UltrasoundImage::addLabel(const QString& text)
{
    const QSizeF mainSize = sceneRect().size();
    const QPointF startPos(mainSize.width() / 2.0, mainSize.height() / 2.0);
    QGraphicsItem* child = createLabel(text, scene(), startPos);
    child->setFlag(QGraphicsItem::ItemIsMovable);
    labels_.push_back(child);
    scene()->invalidate();
}

void UltrasoundImage::addTrace(const QString& text)
{
    Q_UNUSED(text)
    Trace trace;
    trace.points_ = std::make_shared<PointList>();
    const QSizeF mainSize = sceneRect().size();
    QGraphicsScene* scenePtr = scene();
    trace.first_ = createCaliper(scenePtr, mainSize, AddTo::front, trace.points_);
    trace.second_ = createCaliper(scenePtr, mainSize, AddTo::back, trace.points_);
    trace.text_ = text;
    const QRectF rect = trace.second_->boundingRect();
    QGraphicsItem* label = createLabel(text, scenePtr, QPointF(0, 0));
    label->setPos(QPointF(rect.right(), rect.top()) + QPointF(label->boundingRect().width() / 2.0, 0));
    label->setParentItem(trace.second_);
    traces_.push_back(trace);
    scenePtr->invalidate();
}

std::vector<LabelInfo> UltrasoundImage::getLabels() const
{
    std::vector<LabelInfo> result;
    for (QGraphicsItem* child : labels_)
    {
        const QList<QGraphicsItem*> textItems = child->childItems();
        if (textItems.isEmpty())
            continue;
        QGraphicsItem* textItem = textItems.front();
        if (textItem->type() != QGraphicsSimpleTextItem::Type)
            continue;
        const QRectF rect = child->boundingRect().translated(child->scenePos());
        result.emplace_back(static_cast<QGraphicsSimpleTextItem*>(textItem)->text(), rect);
    }
    return result;
}

std::vector<TraceInfo> UltrasoundImage::getTraces() const
{
    std::vector<TraceInfo> result;
    for (const Trace& trace : traces_)
    {
        QPolygonF points;
        points << getCenterPos(trace.first_);
        for (QGraphicsItem* pointItem : *trace.points_)
        {
            points << getCenterPos(pointItem);
        }
        points << getCenterPos(trace.second_);
        result.emplace_back(trace.text_, points);
    }
    return result;
}

QImage UltrasoundImage::overlayImage() const
{
    if (overlay_.isEmpty() || image_.size().isNull())
        return QImage{};

    QImage result(image_.size(), QImage::Format_Grayscale8);
    result.fill(Qt::black);
    QPen pen(Qt::white);
    pen.setWidth(20);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    QPainter painter(&result);
    painter.setPen(pen);
    painter.drawPath(overlay_);
    return result;
}

void UltrasoundImage::clearOverlays()
{
    QGraphicsScene* scenePtr = scene();
    for (QGraphicsItem* child : labels_)
    {
        scenePtr->removeItem(child);
        delete child;
    }
    labels_.clear();
    for (const Trace& trace : traces_)
    {
        scenePtr->removeItem(trace.first_);
        scenePtr->removeItem(trace.second_);
        delete trace.first_;
        delete trace.second_;
    }
    traces_.clear();
    overlay_.clear();
    // redraw
    scene()->invalidate();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void UltrasoundImage::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    castSetOutputSize(w, h);

    image_ = QImage(w, h, QImage::Format_ARGB32);
    image_.fill(Qt::black);

    overlay_.clear();

    QGraphicsView::resizeEvent(e);
}

void UltrasoundImage::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);

    if (event->buttons() == Qt::RightButton)
    {
        overlay_.moveTo(lastPoint_);
        overlay_.lineTo(event->pos());
        lastPoint_ = event->pos();
        // redraw
        scene()->invalidate();
    }
}

void UltrasoundImage::mousePressEvent(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);

    lastPoint_ = event->pos();
}

void UltrasoundImage::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if (event->buttons() == Qt::RightButton)
    {
        overlay_.moveTo(lastPoint_);
        overlay_.lineTo(event->pos());
    }
    lastPoint_ = event->pos();

    // redraw
    scene()->invalidate();
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
    QGraphicsView::drawBackground(painter, r);

    painter->fillRect(r, QBrush(Qt::black));

    if (!image_.isNull())
        painter->drawImage(r, image_);

    QColor overlayColor(overlayColor_);
    overlayColor.setAlpha(255);
    QPen pen(overlayColor);
    pen.setWidth(20);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setOpacity(overlayColor_.alphaF());
    painter->setPen(pen);
    painter->drawPath(overlay_);
    painter->setOpacity(1.0);

    if (noImage_)
    {
        painter->setFont(QFont(QStringLiteral("Arial"), 12));
        painter->setPen(Qt::white);
        painter->drawText(r, Qt::AlignCenter | Qt::AlignBottom, NO_IMAGE_STATEMENT);
    }
}

/// draws the target image
/// @param[in] painter the drawing context
void UltrasoundImage::drawForeground(QPainter* painter, const QRectF& r)
{
    QGraphicsView::drawForeground(painter, r);
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
    signal_.clear();
    const int16_t* buf = static_cast<const int16_t*>(rf) + ((l / 2) * s);
    for (auto i = 0; i < s; i++)
        signal_.push_back(*buf++);

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
    }
}
