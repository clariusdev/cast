#include <stdio.h>
#include <string>
#include <iostream>
#include <atomic>
#include <thread>

#ifdef _MSC_VER
#include <boost/program_options.hpp>
#else
#include <unistd.h>
#endif

#include <cast/cast.h>

#define PRINT           std::cout << std::endl
#define PRINTSL         std::cout << "\r"
#define ERROR           std::cerr << std::endl
#define FAILURE         (-1)
#define SUCCESS         (0)

static char* buffer_ = nullptr;
static int szRawData_ = 0;
static int counter_ = 0;

/// callback for error messages
/// @param[in] err the error message sent from the casting module
void errorFn(const char* err)
{
    ERROR << "error: " << err;
}

/// callback for freeze state change
/// @param[in] val the freeze state value, 1 = frozen, 0 = imaging
void freezeFn(int val)
{
    PRINT << (val ? "frozen" : "imaging");
    counter_ = 0;
}

/// callback for button press
/// @param[in] btn the button that was pressed, 0 = up, 1 = down
/// @param[in] clicks # of clicks used
void buttonFn(int btn, int clicks)
{
    PRINT << (btn ? "down" : "up") << " button pressed, clicks: " << clicks;
}

/// callback for readback progress
/// @param[in] progress the readback progress
void progressFn(int progress)
{
    PRINTSL << "downloading: " << progress << "%" << std::flush;
}

/// prints imu data
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void printImuData(int npos, const CusPosInfo* pos)
{
    for (auto i = 0; i < npos; i++)
    {
        PRINT << "imu: " << i << ", time: " << pos[i].tm;
        PRINT << "accel: " << pos[i].ax << "," << pos[i].ay << "," << pos[i].az;
        PRINT << "gyro: " << pos[i].gx << "," << pos[i].gy << "," << pos[i].gz;
        PRINT << "magnet: " << pos[i].mx << "," << pos[i].my << "," << pos[i].mz;
    }
}

/// callback for a new pre-scan converted data sent from the scanner
/// @param[in] newImage a pointer to the raw image bits
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newRawImageFn(const void* newImage, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos)
{
#ifdef PRINTRAW
    if (nfo->rf)
        PRINT << "new rf data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos;
    else
        PRINT << "new pre-scan data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos << " jpeg size: " << (int)nfo->jpeg;

    if (npos)
        printImuData(npos, pos);
#else
    (void)newImage;
    (void)nfo;
    (void)npos;
    (void)pos;
#endif
}

/// callback for a new image sent from the scanner
/// @param[in] newImage a pointer to the raw image bits
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newProcessedImageFn(const void* newImage, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)
{
    (void)newImage;
    (void)pos;
    PRINTSL << "new image (" << counter_++ << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel << " bpp. @ "
            << nfo->imageSize << "bytes. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos << std::flush;
}

/// callback for a new spectral image sent from the scanner
/// @param[in] newImage a pointer to the raw image bits
/// @param[in] nfo the image properties
void newSpectralImageFn(const void* newImage, const CusSpectralImageInfo* nfo)
{
    (void)newImage;
    PRINTSL << "new spectrum: " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->period << " sec/line." << std::flush;
}

/// saves raw data from the current download buffer
/// @return success of the call
bool saveRawData()
{
    if (!szRawData_ || !buffer_)
        return false;

    auto cleanup = []()
    {
        free(buffer_);
        buffer_ = nullptr;
        szRawData_ = 0;
    };

    FILE* fp = nullptr;
    // save raw data to disk as a compressed file
    #ifdef _MSC_VER
        fopen_s(&fp, "raw_data.tar", "wb+");
    #else
        fp = fopen("raw_data.tar", "wb+");
    #endif
    if (!fp)
    {
        cleanup();
        return false;
    }

    fwrite(buffer_, szRawData_, 1, fp);
    fclose(fp);
    cleanup();
    return true;
}

/// processes the user input
/// @param[out] quit the exit flag
void processEventLoop(std::atomic_bool& quit)
{
    std::string cmd;

    while (std::getline(std::cin, cmd))
    {
        if (cmd == "Q" || cmd == "q")
        {
            quit = true;
            break;
        }
        else if (cmd == "F" || cmd == "f")
        {
            if (cusCastUserFunction(Freeze, 0, nullptr) < 0)
                ERROR << "error toggling freeze" << std::endl;
        }
        else if (cmd == "D")
        {
            if (cusCastUserFunction(DepthInc, 0, nullptr) < 0)
                ERROR << "error incrementing depth" << std::endl;
        }
        else if (cmd == "d")
        {
            if (cusCastUserFunction(DepthDec, 0, nullptr) < 0)
                ERROR << "error decrementing depth" << std::endl;
        }
        else if (cmd == "G")
        {
            if (cusCastUserFunction(GainInc, 0, nullptr) < 0)
                ERROR << "error incrementing gain" << std::endl;
        }
        else if (cmd == "g")
        {
            if (cusCastUserFunction(GainDec, 0, nullptr) < 0)
                ERROR << "error decrementing gain" << std::endl;
        }
        else if (cmd == "R" || cmd == "r")
        {
            if (cusCastRequestRawData(0, 0, [](int sz)
            {
                if (sz < 0)
                    ERROR << "error requesting raw data" << std::endl;
                else if (sz == 0)
                {
                    szRawData_ = 0;
                    ERROR << "no raw data buffered" << std::endl;
                }
                else
                {
                    szRawData_ = sz;
                    PRINT << "raw data file of size " << sz << "B ready to download";
                }

            }) < 0)
                ERROR << "error requesting raw data" << std::endl;
        }
        else if (cmd == "Y" || cmd == "y")
        {
            if (szRawData_ <= 0)
                ERROR << "no raw data to download" << std::endl;
            else
            {
                buffer_ = (char*)malloc(szRawData_);

                if (cusCastReadRawData((void**)(&buffer_), [](int ret)
                {
                    if (ret == SUCCESS)
                    {
                        PRINT << "successfully downloaded raw data" << std::endl;
                        saveRawData();
                    }
                }) < 0)
                    ERROR << "error downloading raw data" << std::endl;
            }
        }
        else
        {
            PRINT << "valid commands: [q: quit]";
            PRINT << "       imaging: [f: freeze, d/D: depth, g/G: gain]";
            PRINT << "      raw data: [r: request, y: download]";
        }
    }
}

int init(int& argc, char** argv)
{
    const int width  = 640;
    const int height = 480;
    std::string keydir, ipAddr;
    unsigned int port = 0;

    // ensure console buffers are flushed automatically
    setvbuf(stdout, nullptr, _IONBF, 0) != 0 || setvbuf(stderr, nullptr, _IONBF, 0);

    // Windows: Visual C++ doesn't have 'getopt' so use Boost's program_options instead
#ifdef _MSC_VER
    namespace po = boost::program_options;
    keydir = "c:/";

    try
    {
        po::options_description desc("Usage: 192.168.1.21", 12345);
        desc.add_options()
            ("help", "produce help message")
            ("address", po::value<std::string>(&ipAddr)->required(), "set the IP address of the host scanner")
            ("port", po::value<unsigned int>(&port)->required(), "set the port of the host scanner")
            ("keydir", po::value<std::string>(&keydir)->default_value("/tmp/"), "set the path containing the security keys")
        ;

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);

        if (vm.count("help"))
        {
            PRINT << desc << std::endl;
            return FAILURE;
        }

        po::notify(vm);
    }
    catch(std::exception& e)
    {
        ERROR << "Error: " << e.what() << std::endl;
        return FAILURE;
    }
    catch(...)
    {
        ERROR << "Unknown error!" << std::endl;
        return FAILURE;
    }
#else // every other platform has 'getopt' which we're using so as to not pull in the Boost dependency
    int o;
    keydir = "/tmp/";

    // check command line options
    while ((o = getopt(argc, argv, "k:a:p:")) != -1)
    {
        switch (o)
        {
        // security key directory
        case 'k': keydir = optarg; break;
        // ip address
        case 'a': ipAddr = optarg; break;
        // port
        case 'p':
            try { port = std::stoi(optarg); }
            catch (std::exception&) { PRINT << port; }
            break;
        // invalid argument
        case '?': PRINT << "invalid argument, valid options: -a [addr], -p [port], -k [keydir]"; break;
        default: break;
        }
    }

    if (!ipAddr.size())
    {
        ERROR << "no ip address provided. run with '-a [addr]" << std::endl;
        return FAILURE;
    }

    if (!port)
    {
        ERROR << "no casting port provided. run with '-p [port]" << std::endl;
        return FAILURE;
    }
#endif

    PRINT << "starting caster...";

    // initialize with callbacks
    if (cusCastInit(argc, argv, keydir.c_str(), newProcessedImageFn, newRawImageFn, newSpectralImageFn, freezeFn, buttonFn, progressFn, errorFn, width, height) < 0)
    {
        ERROR << "could not initialize caster" << std::endl;
        return FAILURE;
    }
    if (cusCastConnect(ipAddr.c_str(), port, [](int ret)
    {
        if (ret == FAILURE)
            ERROR << "could not connect to scanner" << std::endl;
        else
            PRINT << "...connected, streaming port: " << ret << " -- check firewall settings if no image callback received";

    }) < 0)
    {
        ERROR << "connection attempt failed" << std::endl;
        return FAILURE;
    }

    return 0;
}

/// main entry point
/// @param[in] argc # of program arguments
/// @param[in] argv list of arguments
int main(int argc, char* argv[])
{
    int rcode = init(argc, argv);

    if (rcode == SUCCESS)
    {
        std::atomic_bool quitFlag(false);
        std::thread eventLoop(processEventLoop, std::ref(quitFlag));
        eventLoop.join();
    }

    cusCastDestroy();
    return rcode;
}
