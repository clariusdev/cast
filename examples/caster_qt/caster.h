#pragma once

namespace Ui
{
    class Caster;
}

class ProbeRender;

#define IMAGE_EVENT     static_cast<QEvent::Type>(QEvent::User + 1)
#define PRESCAN_EVENT   static_cast<QEvent::Type>(QEvent::User + 2)
#define RF_EVENT        static_cast<QEvent::Type>(QEvent::User + 3)
#define SPECTRUM_EVENT  static_cast<QEvent::Type>(QEvent::User + 4)
#define FREEZE_EVENT    static_cast<QEvent::Type>(QEvent::User + 5)
#define BUTTON_EVENT    static_cast<QEvent::Type>(QEvent::User + 6)
#define ERROR_EVENT     static_cast<QEvent::Type>(QEvent::User + 7)
#define PROGRESS_EVENT  static_cast<QEvent::Type>(QEvent::User + 8)
#define RAWDATA_EVENT   static_cast<QEvent::Type>(QEvent::User + 9)
#define IMU_EVENT       static_cast<QEvent::Type>(QEvent::User + 10)

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
        /// @param[in] sz total size of the image
        Image(QEvent::Type evt, const void* data, long long int tm, int w, int h, int bpp, int sz, const QQuaternion& imu)
            : QEvent(evt), data_(data), tm_(tm), width_(w), height_(h), bpp_(bpp), size_(sz), imu_(imu) { }

        const void* data_;  ///< pointer to the image data
        long long int tm_;  ///< timestamp
        int width_;         ///< width of the image
        int height_;        ///< height of the image
        int bpp_ ;          ///< bits per pixel
        int size_;          ///< total size of the image
        QQuaternion imu_;   ///< latest imu position
    };

    /// wrapper for new rf events that can be posted from the api callbacks
    class RfImage : public Image
    {
    public:
        /// default constructor
        /// @param[in] data the rf data
        /// @param[in] l # of rf lines
        /// @param[in] s # of samples per line
        /// @param[in] bps bits per sample
        /// @param[in] sz size of data in bytes
        /// @param[in] lateral lateral spacing between lines
        /// @param[in] axial sample size
        RfImage(const void* data, long long int tm, int l, int s, int bps, int sz, double lateral, double axial) : Image(RF_EVENT, data, tm, l, s, bps, sz, {}), lateral_(lateral), axial_(axial) { }

        double lateral_;    ///< spacing between each line
        double axial_;      ///< sample size
    };

    /// wrapper for new spectrum events that can be posted from the api callbacks
    class Spectrum : public Image
    {
    public:
        /// default constructor
        /// @param[in] data the image data
        /// @param[in] l the # of spectrum lines
        /// @param[in] s # of samples per line
        /// @param[in] bps bits per sample
        /// @param[in] sz size of the image in bytes
        Spectrum(const void* data, int l, int s, int bps, int sz, double period, double mps, double vps, bool pw) : Image(SPECTRUM_EVENT, data, 0, l, s, bps, sz, {}),
            period_(period), micronsPerSample_(mps), velocityPerSample_(vps), pw_(pw) { }

        double period_;             ///< line acquisition period in seconds
        double micronsPerSample_;   ///< microns per pixel/sample in an m spectrum
        double velocityPerSample_;  ///< velocity in m/s per pixel/sample in a pw spectrum
        bool pw_;                   ///< flag specifying the data is pw and not m
    };

    /// wrapper for new imu data events that can be posted from the api callbacks
    class Imu : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] imu latest imu data
        explicit Imu(const QQuaternion& imu) : QEvent(IMU_EVENT), imu_(imu) { }

        QQuaternion imu_;   ///< latest imu position
    };

    /// wrapper for freeze events that can be posted from the api callbacks
    class Freeze : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] frozen the freeze state
        explicit Freeze(bool frozen) : QEvent(FREEZE_EVENT), frozen_(frozen) { }

        bool frozen_;   ///< the freeze state
    };

    /// wrapper for button press events that can be posted from the api callbacks
    class Button : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] btn the button pressed
        /// @param[in] clicks # of clicks
        Button(int btn, int clicks) : QEvent(BUTTON_EVENT), button_(btn), clicks_(clicks) { }

        int button_;    ///< button pressed, 0 = up, 1 = down
        int clicks_;    ///< # of clicks
    };

    /// wrapper for error events that can be posted from the api callbacks
    class Error : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] err the error message
        explicit Error(const QString& err) : QEvent(ERROR_EVENT), error_(err) { }

        QString error_;     ///< the error message
    };

    /// wrapper for progress events that can be posted from the api callbacks
    class Progress : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] progress the current progress
        explicit Progress(int progress) : QEvent(PROGRESS_EVENT), progress_(progress) { }

        int progress_;  ///< the current progress
    };

    /// wrapper for raw data completion events that can be posted from the api callbacks
    class RawData : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] success success of downloading raw data
        explicit RawData(bool success) : QEvent(RAWDATA_EVENT), success_(success) { }

        bool success_;  ///< the current progress
    };
}

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

class UltrasoundImage;
class RfSignal;

/// caster gui application
class Caster : public QMainWindow
{
    Q_OBJECT

public:
    explicit Caster(QWidget *parent = nullptr);
    ~Caster() override;

protected:
    virtual bool event(QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void newProcessedImage(const void* img, int w, int h, int bpp, int sz, const QQuaternion& imu);
    void newPrescanImage(const void* img, int w, int h, int bpp, int sz);
    void newRfData(const void* rfdata, int l, int s, int bps, double lateral, double axial);
    void newMSpectrum(const void* rfdata, int l, int s, int bps, double period, double micronsPerSample);
    void newPwSpectrum(const void* rfdata, int l, int s, int bps, double period, double velocityPerSample);
    void setFreeze(bool en);
    void onButton(int btn, int clicks);
    void setProgress(int progress);
    void setError(const QString& err);
    bool rawDataReady(bool success);
    void rawData(int sz);
    void connected(int imagePort, int imuPort);
    void disconnected(bool res);
    void newImuData(const QQuaternion& imu);

public slots:
    void onConnect();
    void onFreeze();
    void onShallower();
    void onDeeper();
    void onRequest();
    void onDownload();
    void onAddLabel();
    void onAddTrace();
    void onCaptureImage();
    void onClearScreen();

private:
    void updateCaptureButtons();
    bool connected_;            ///< connection state
    bool frozen_;               ///< freeze state
    long long int lasttime_;    ///< timesetamp of last received frame
    uint32_t imuSamples_;       ///< keeps track of samples collected
    RawDataInfo rawData_;       ///< raw data attributes
    Ui::Caster *ui_;            ///< ui controls, etc.
    UltrasoundImage* image_;    ///< image display
    ProbeRender* render_;           ///< probe renderer
    RfSignal* signal_;          ///< rf signal display
    QImage prescan_;            ///< pre-scan converted image
    QTimer imageTimer_;         ///< timer to warn the user about the firewall
    std::unique_ptr<QSettings> settings_;   ///< persistent settings
};
