#pragma once

#ifdef __clang__
    #pragma clang diagnostic push
#elif _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4512)
    #pragma warning(disable: 4510)
    #pragma warning(disable: 4610)
    #pragma warning(disable: 4458)
    #pragma warning(disable: 4800)
#endif

#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif

namespace Ui
{
    class Listener;
}

#define IMAGE_EVENT     static_cast<QEvent::Type>(QEvent::User + 1)
#define PRESCAN_EVENT   static_cast<QEvent::Type>(QEvent::User + 2)
#define FREEZE_EVENT    static_cast<QEvent::Type>(QEvent::User + 3)
#define BUTTON_EVENT    static_cast<QEvent::Type>(QEvent::User + 4)
#define ERROR_EVENT     static_cast<QEvent::Type>(QEvent::User + 5)
#define PROGRESS_EVENT  static_cast<QEvent::Type>(QEvent::User + 6)
#define RAWDATA_EVENT   static_cast<QEvent::Type>(QEvent::User + 7)

namespace event
{
    /// wrapper for new image events that can be posted from the api callbacks
    class Image : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] data the image data
        /// @param[in] w the image width
        /// @param[in] h the image height
        /// @param[in] bpp the image bits per pixel
        Image(const void* data, int w, int h, int bpp) : QEvent(customType()), data_(data), width_(w), height_(h), bpp_(bpp)  { }
        /// retrieves the image data from the event
        /// @return the event's encapsulated image data
        const void* data() const { return data_; }
        /// retrieves the image width from the event
        /// @return the event's encapsulated image width
        int width() const { return width_; }
        /// retrieves the image height from the event
        /// @return the event's encapsulated image height
        int height() const { return height_; }
        /// retrieves the bits per pixel from the event
        /// @return the event's encapsulated bits per pixel
        int bpp() const { return bpp_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        virtual QEvent::Type customType() { return IMAGE_EVENT; }

    protected:
        const void* data_;  ///< pointer to the image data
        int width_;         ///< width of the image
        int height_;        ///< height of the image
        int bpp_;           ///< bits per pixel of the image (should always be 32)
    };

    /// wrapper for new data events that can be posted from the api callbacks
    class PreScanImage : public Image
    {
    public:
        /// default constructor
        /// @param[in] data the image data
        /// @param[in] w the image width
        /// @param[in] h the image height
        /// @param[in] bpp the image bits per sample
        /// @param[in] jpg the jpeg compression flag for the data
        PreScanImage(const void* data, int w, int h, int bpp, int jpg) : Image(data, w, h, bpp), jpeg_(jpg)  { }
        /// retrieves the jpeg compression flag for the image
        /// @return the event's encapsulated jpeg compression flag for the image
        bool jpeg() const { return jpeg_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        virtual QEvent::Type customType() override { return PRESCAN_EVENT; }

    private:
        bool jpeg_; ///< size of jpeg compressed image
    };

    /// wrapper for freeze events that can be posted from the api callbacks
    class Freeze : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] frozen the freeze state
        Freeze(bool frozen) : QEvent(customType()), frozen_(frozen)  { }
        /// retrieves the freeze state from the event
        /// @return the event's encapsulated freeze state
        bool frozen() const { return frozen_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        QEvent::Type customType() { return FREEZE_EVENT; }

    private:
        bool frozen_;   ///< the freeze state
    };

    /// wrapper for button press events that can be posted from the api callbacks
    class Button : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] btn the button pressed
        /// @param[in] clicks # of clicks
        Button(int btn, int clicks) : QEvent(customType()), button_(btn), clicks_(clicks)  { }
        /// retrieves the button pressed
        /// @return the event's encapsulated which button was pressed
        int button() const { return button_; }
        /// retrieves the # of clicks used
        /// @return the event's encapsulated # of clicks used
        int clicks() const { return clicks_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        QEvent::Type customType() { return BUTTON_EVENT; }

    private:
        int button_;    ///< button pressed, 0 = up, 1 = down
        int clicks_;    ///< # of clicks
    };

    /// wrapper for error events that can be posted from the api callbacks
    class Error : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] err the error message
        Error(const QString& err) : QEvent(customType()), error_(err)  { }
        /// retrieves the error message from the event
        /// @return the event's encapsulated error message
        QString error() const { return error_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        QEvent::Type customType() { return ERROR_EVENT; }

    private:
        QString error_;     ///< the error message
    };

    /// wrapper for progress events that can be posted from the api callbacks
    class Progress : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] progress the current progress
        Progress(int progress) : QEvent(customType()), progress_(progress)  { }
        /// retrieves the current progress from the event
        /// @return the event's encapsulated progress
        int progress() const { return progress_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        QEvent::Type customType() { return PROGRESS_EVENT; }

    private:
        int progress_;  ///< the current progress
    };

    /// wrapper for raw data completion events that can be posted from the api callbacks
    class RawData : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] success success of downloading raw data
        RawData(bool success) : QEvent(customType()), success_(success)  { }
        /// retrieves the sucess from the event
        /// @return the event's encapsulated success
        bool success() const { return success_; }
        /// retrieves the event's custom user type
        /// @return the event's custom user type
        QEvent::Type customType() { return RAWDATA_EVENT; }

    private:
        bool success_;  ///< the current progress
    };
}

/// ultrasound image display
class UltrasoundImage : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UltrasoundImage(QWidget*);

    void loadImage(const void* img, int w, int h, int bpp);

protected:
    virtual void drawForeground(QPainter*, const QRectF&) override;
    virtual void drawBackground(QPainter*, const QRectF&) override;

    virtual void resizeEvent(QResizeEvent* e) override;
    virtual int heightForWidth(int w) const override;
    virtual QSize sizeHint() const override;

private:
    QImage image_;  ///< the image buffer
    QMutex lock_;   ///< locking mechanism
};

/// holds raw data information
class RawDataInfo
{
public:
    RawDataInfo() : size_(0), ptr_(nullptr) { }

    QString file_;
    int size_;
    QByteArray data_;
    char* ptr_;
};

/// listener gui application
class Listener : public QMainWindow
{
    Q_OBJECT

public:
    explicit Listener(QWidget *parent = nullptr);
    ~Listener();

protected:
    virtual bool event(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    void newProcessedImage(const void* img, int w, int h, int bpp);
    void newRawImage(const void* img, int w, int h, int bpp, bool jpg);
    void setFreeze(bool en);
    void onButton(int btn, int clicks);
    void setProgress(int progress);
    void setError(const QString& err);
    bool rawDataReady(bool success);

public slots:
    void onConnect();
    void onFreeze();
    void onShallower();
    void onDeeper();
    void onRequest();
    void onDownload();

private:
    bool connected_;            ///< connection state
    RawDataInfo rawData_;       ///< raw data attributes
    Ui::Listener *ui_;          ///< ui controls, etc.
    UltrasoundImage* image_;    ///< image display
    QImage prescan_;            ///< pre-scan converted image
};
