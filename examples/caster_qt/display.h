#pragma once

#define NO_IMAGE_STATEMENT QStringLiteral("No Image? Check the O/S Firewall Settings")

#include <deque>

struct LabelInfo
{
    explicit LabelInfo(const QString& text, const QRectF& rect)
        : text_(text), rect_(rect)
    {}
    QString text_;
    QRectF rect_;
};

struct TraceInfo
{
    explicit TraceInfo(const QString& text, const QPolygonF& points)
        : text_(text), points_(points)
    {}
    QString text_;
    QPolygonF points_;
};

using PointList = std::deque<QGraphicsItem*>;

/// ultrasound image display
class UltrasoundImage : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UltrasoundImage(QWidget*);

    void loadImage(const void* img, int w, int h, int bpp, int sz);
    void setNoImage(bool en) { noImage_ = en; }
    void addLabel(const QString& text);
    void addTrace(const QString& text);
    void clearOverlays();
    QImage overlayImage() const;
    QColor overlayColor() const { return overlayColor_; }
    std::vector<LabelInfo> getLabels() const;
    std::vector<TraceInfo> getTraces() const;

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* e) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    struct Trace
    {
        QGraphicsRectItem* first_;
        QGraphicsRectItem* second_;
        QString text_;
        std::shared_ptr<PointList> points_;
    };

    bool noImage_;  ///< no image flag for potential firewall issues
    QImage image_;  ///< the image buffer
    QPainterPath overlay_; ///< user overlay
    QColor overlayColor_; ///< overlay color
    QPoint lastPoint_;
    std::vector<QGraphicsItem*> labels_;
    std::vector<Trace> traces_;
};

/// rf signal display
class RfSignal : public QGraphicsView
{
    Q_OBJECT
public:
    explicit RfSignal(QWidget*);

    void loadSignal(const void* rf, int l, int s, int ss);
    void setZoom(int zoom);

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;

    virtual void resizeEvent(QResizeEvent* e) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    QVector<int16_t> signal_;   ///< the rf signal
    qreal zoom_;                ///< zoom level
};
