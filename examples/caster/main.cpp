#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
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

static char* buffer_ = nullptr;
static int szRawData_ = 0;
static int counter_ = 0;
static bool streamOutput_ = true;
static long long int lasttime_ = 0;
static int captureID_ = -1;

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
void buttonFn(CusButton btn, int clicks)
{
    PRINT << ((btn == ButtonDown) ? "down" : "up") << " button pressed, clicks: " << clicks;
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

/// @brief Receives the new imu data streamed from the scanner
/// @param pos the positional information data streamed
void newImuData(const CusPosInfo* pos)
{
    PRINT << "imu data streamed:";
    printImuData(1, pos);
}

/// callback for a new pre-scan converted data sent from the scanner
/// @param[in] newImage a pointer to the raw image bits
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newRawImageFn(const void* newImage, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos)
{
    lasttime_ = nfo->tm;
#ifdef PRINTRAW
    if (nfo->rf)
        PRINT << "new rf data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos;
    else
        PRINT << "new pre-scan data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos << " jpeg size: " << static_cast<int>(nfo->jpeg);

    if (npos)
        printImuData(npos, pos);
#else
    (void)newImage;
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
    if (streamOutput_)
        PRINTSL << "new image (" << counter_++ << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel << " bpp. @ "
                << nfo->imageSize << "bytes. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos << std::flush;
}

/// callback for a new spectral image sent from the scanner
/// @param[in] newImage a pointer to the raw image bits
/// @param[in] nfo the image properties
void newSpectralImageFn(const void* newImage, const CusSpectralImageInfo* nfo)
{
    (void)newImage;
    if (streamOutput_)
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

void doneCapture(int result)
{
    if (result < 0)
        ERROR << "failed to submit capture";
    else
        PRINT << "successfully submitted capture";
}

char getCommand(const std::string& line)
{
    if (line.empty() || (line.size() > 1 && line[1] != ' '))
        return '\0';
    return line[0];
}

std::vector<std::string> getParameters(const std::string& line, std::size_t max)
{
    std::vector<std::string> result;
    if (line.size() < 2 || line[1] != ' ')
        return result;
    std::size_t lastIndex = 2;
    std::size_t nextIndex = line.find(' ', lastIndex);
    while (lastIndex < line.size())
    {
        const bool addAll = (nextIndex == std::string::npos || result.size() + 1 == max);
        std::size_t count = (addAll ? line.length() : nextIndex + 1) - lastIndex;
        result.push_back(line.substr(lastIndex, addAll ? count : count - 1));
        lastIndex += count;
        nextIndex = line.find(' ', lastIndex);
    }
    return result;
}

bool parseDouble(double& val, const std::string& param)
{
    try
    {
        val = std::stod(param);
        return true;
    }
    catch (std::exception& e)
    {
        ERROR << e.what() << std::endl;
        return false;
    }
}

/// processes the user input
/// @param[out] quit the exit flag
void processEventLoop(std::atomic_bool& quit)
{
    std::string line;

    while (std::getline(std::cin, line))
    {
        const char cmd = getCommand(line);
        if (cmd == 'Q' || cmd == 'q')
        {
            quit = true;
            break;
        }
        else if (cmd == 'F' || cmd == 'f')
        {
            if (castUserFunction(Freeze, 0, nullptr) < 0)
                ERROR << "error toggling freeze" << std::endl;
        }
        else if (cmd == 'D')
        {
            if (castUserFunction(DepthInc, 0, nullptr) < 0)
                ERROR << "error incrementing depth" << std::endl;
        }
        else if (cmd == 'd')
        {
            if (castUserFunction(DepthDec, 0, nullptr) < 0)
                ERROR << "error decrementing depth" << std::endl;
        }
        else if (cmd == 'G')
        {
            if (castUserFunction(GainInc, 0, nullptr) < 0)
                ERROR << "error incrementing gain" << std::endl;
        }
        else if (cmd == 'g')
        {
            if (castUserFunction(GainDec, 0, nullptr) < 0)
                ERROR << "error decrementing gain" << std::endl;
        }
        else if (cmd == 'S' || cmd == 's')
        {
            streamOutput_ = !streamOutput_;
        }
        else if (cmd == 'R' || cmd == 'r')
        {
            if (castRequestRawData(0, 0, 1, [](int sz, const char*)
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
        else if (cmd == 'Y' || cmd == 'y')
        {
            if (szRawData_ <= 0)
                ERROR << "no raw data to download" << std::endl;
            else
            {
                buffer_ = reinterpret_cast<char*>(malloc(szRawData_));

                if (castReadRawData((void**)(&buffer_), [](int ret)
                {
                    if (ret == CUS_SUCCESS)
                    {
                        PRINT << "successfully downloaded raw data" << std::endl;
                        saveRawData();
                    }
                }) < 0)
                    ERROR << "error downloading raw data" << std::endl;
            }
        }
        else if (cmd == 'C' || cmd == 'c')
        {
            if (lasttime_ == 0)
            {
                ERROR << "no images received yet" << std::endl;
                continue;
            }
            if (captureID_ < 0)
            {
                captureID_ = castStartCapture(lasttime_);
                if (captureID_ < 0)
                    ERROR << "failed to start capture" << std::endl;
                else
                    PRINT << "started capture " << captureID_ << std::endl;
            }
            else
            {
                if (castFinishCapture(captureID_, &doneCapture) < 0)
                    ERROR << "failed to finish capture" << std::endl;
                else
                    PRINT << "finished capture " << captureID_ << std::endl;
                captureID_ = -1;
            }
        }
        else if (cmd == 'l' || cmd == 'L')
        {
            if (captureID_ < 0)
            {
                ERROR << "no capture in progress" << std::endl;
                continue;
            }
            const std::vector<std::string> prms = getParameters(line, 3);
            double x = 0;
            double y = 0;
            if (prms.size() < 3 || !parseDouble(x, prms[0]) || !parseDouble(y, prms[1]))
            {
                ERROR << "wrong label parameters provided";
                ERROR << "please give parameters as -> x y text";
                ERROR << "where x and y are the coordinates of the center of the label" << std::endl;
                continue;
            }
            if (castAddLabelOverlay(captureID_, prms.back().c_str(), x, y, 100.0, 100.0) < 0)
                ERROR << "failed to add label to capture" << std::endl;
            else
                PRINT << "added label '" << prms.back() << "' at (" << x << ", " << y << ") to capture" << std::endl;
        }
        else if (cmd == 'm' || cmd == 'M')
        {
            if (captureID_ < 0)
            {
                ERROR << "no capture in progress" << std::endl;
                continue;
            }
            const std::vector<std::string> prms = getParameters(line, 5);
            double x1 = 0;
            double y1 = 0;
            double x2 = 0;
            double y2 = 0;
            if (prms.size() < 5
                || !parseDouble(x1, prms[0]) || !parseDouble(y1, prms[1])
                || !parseDouble(x2, prms[2]) || !parseDouble(y2, prms[3]))
            {
                ERROR << "wrong measurement parameters provided";
                ERROR << "please give parameters as -> x1 y1 x2 y2 text";
                ERROR << "for a measurement from (x1,y1) to (x2,y2) with label 'text'" << std::endl;
                continue;
            }
            const double points[] = { x1, y1, x2, y2 };
            const int nDoubles = static_cast<int>(sizeof(points) / sizeof(points[0]));
            if (castAddMeasurement(captureID_, CusMeasurementTypeDistance, prms.back().c_str(), points, nDoubles) < 0)
                ERROR << "failed to add label to capture" << std::endl;
            else
                PRINT << "added measurement '" << prms.back() << "' from (" << x1 << ", " << y1 << ") "
                    << "to (" << x2 << ", " << y2 << ") to capture" << std::endl;
        }
        else if (cmd == 'p' || cmd == 'P')
        {
            const std::vector<std::string> prms = getParameters(line, 2);
            if (prms.size() != 2)
            {
                ERROR << "usage: p {param_name} {param_value}" << std::endl;
                continue;
            }
            std::string prm = prms[0];
            std::transform(prm.begin(), prm.end(), prm.begin(), ::tolower);
            if (prms[1] == "true" || prms[1] == "false")
            {
                if (castEnableParameter(prms[0].c_str(), (prms[1] == "true"), [](int ret)
                {
                    if (ret == CUS_FAILURE)
                        ERROR << "parameter enable/disable failed";
                }) < 0)
                {
                    ERROR << "parameter enable/disable failed";
                }
            }
            else if (prm.find("pulse") != std::string::npos)
            {
                if (castSetPulse(prms[0].c_str(), prms[1].c_str(), [](int ret)
                {
                    if (ret == CUS_FAILURE)
                        ERROR << "parameter pulse shape set failed";
                }) < 0)
                {
                    ERROR << "parameter pulse shape set failed";
                }
            }
            else
            {
                double val = 0;
                if (!parseDouble(val, prms[1]))
                {
                    ERROR << "could not convert parameter value to numeric value";
                    continue;
                }
                if (castSetParameter(prms[0].c_str(), val, [](int ret)
                {
                    if (ret == CUS_FAILURE)
                        ERROR << "parameter setting parameter";
                }) < 0)
                {
                    ERROR << "parameter setting parameter";
                }
            }
        }
        else
        {
            PRINT << "valid commands: [q: quit]";
            PRINT << "       display: [s: toggle stream outptu]";
            PRINT << "       imaging: [f: freeze, d/D: depth, g/G: gain]";
            PRINT << "        params: [p: change parameter]";
            PRINT << "      raw data: [r: request, y: download]";
            PRINT << "       capture: [c: start/end capture, l: add label, m: add measurement]" << std::endl;
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
            return CUS_FAILURE;
        }

        po::notify(vm);
    }
    catch (std::exception& e)
    {
        ERROR << "Error: " << e.what() << std::endl;
        return CUS_FAILURE;
    }
    catch (...)
    {
        ERROR << "Unknown error!" << std::endl;
        return CUS_FAILURE;
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
        return CUS_FAILURE;
    }

    if (!port)
    {
        ERROR << "no casting port provided. run with '-p [port]" << std::endl;
        return CUS_FAILURE;
    }
#endif

    PRINT << "starting caster...";

    auto initParams = castDefaultInitParams();
    initParams.args.argc = argc;
    initParams.args.argv = argv;
    initParams.storeDir = keydir.c_str();
    initParams.newProcessedImageFn = newProcessedImageFn;
    initParams.newRawImageFn = newRawImageFn;
    initParams.newSpectralImageFn = newSpectralImageFn;
    initParams.newImuDataFn = newImuData;
    initParams.freezeFn = freezeFn;
    initParams.buttonFn = buttonFn;
    initParams.progressFn = progressFn;
    initParams.errorFn = errorFn;
    initParams.width = width;
    initParams.height = height;
    // initialize with callbacks
    if (castInit(&initParams) < 0)
    {
        ERROR << "could not initialize caster" << std::endl;
        return CUS_FAILURE;
    }
    if (castConnect(ipAddr.c_str(), port, "research", [](int imagePort, int imuPort, int swRevMatch)
    {
        if (imagePort == CUS_FAILURE)
            ERROR << "could not connect to scanner" << std::endl;
        else
        {
            PRINT << "...connected, streaming port: " << imagePort << " -- check firewall settings if no image callback received";
            if (imuPort > 0)
            {
                PRINT << "imu now streaming at port: " << imuPort;
            }
            else
            {
                PRINT << "imu streaming off";
            }
            if (swRevMatch == CUS_FAILURE)
                ERROR << "software revisions do not match, that is not necessarily a problem" << std::endl;
        }

    }) < 0)
    {
        ERROR << "connection attempt failed" << std::endl;
        return CUS_FAILURE;
    }

    return 0;
}

/// main entry point
/// @param[in] argc # of program arguments
/// @param[in] argv list of arguments
int main(int argc, char* argv[])
{
    int rcode = init(argc, argv);

    if (rcode == CUS_SUCCESS)
    {
        std::atomic_bool quitFlag(false);
        std::thread eventLoop(processEventLoop, std::ref(quitFlag));
        eventLoop.join();
    }

    castDestroy();
    return rcode;
}
