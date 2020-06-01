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

#include <listen/listen.h>

#define BLOCKINGCALL    nullptr
#define PRINT           std::cout << std::endl
#define ERROR           std::cerr << std::endl
#define ERRCODE         (-1)
#define SUCCESS         (0)

/// callback for error messages
/// @param[in] err the error message sent from the listener module
void errorFn(const char* err)
{
    ERROR << "error: " << err;
}

/// callback for freeze state change
/// @param[in] val the freeze state value, 1 = frozen, 0 = imaging
void freezeFn(int val)
{
    PRINT << (val ? "frozen" : "imaging");
}

/// callback for button press
/// @param[in] btn the button that was pressed, 0 = up, 1 = down
/// @param[in] clicks # of clicks used
void buttonFn(int btn, int clicks)
{
    PRINT << (btn ? "down" : "up") << " button pressed, clicks: " << clicks;
}

/// callback for readback progreess
/// @param[in] progress the readback progress
void progressFn(int progress)
{
    ERROR << "download: " << progress;
}

/// prints imu data
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void printImuData(int npos, const ClariusPosInfo* pos)
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
/// @param[in] newImage a pointer to the raw image bits of
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newRawImageFn(const void* newImage, const ClariusRawImageInfo* nfo, int npos, const ClariusPosInfo* pos)
{
    PRINT << "new pre-scan data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos << " jpeg size: " << (int)nfo->jpeg;

    if (npos)
        printImuData(npos, pos);
}

/// callback for a new image sent from the scanner
/// @param[in] newImage a pointer to the raw image bits of
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newProcessedImageFn(const void* newImage, const ClariusProcessedImageInfo* nfo, int npos, const ClariusPosInfo* pos)
{
    PRINT << "new image (" << newImage << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel
          << "bits. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos;

    if (npos)
        printImuData(npos, pos);
}

void processEventLoop(std::atomic_bool& quit)
{
    std::string cmd;
    int ret = 0;
    char* buffer = nullptr;
    FILE* fp = nullptr;

    while (std::getline(std::cin, cmd))
    {
        if (cmd == "Q" || cmd == "q")
        {
            quit = true;
            break;
        }
        else if (cmd == "F" || cmd == "f")
        {
            ret = clariusUserFunction(USER_FN_TOGGLE_FREEZE, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error toggling freeze" << std::endl;
        }
        else if (cmd == "D")
        {
            ret = clariusUserFunction(USER_FN_DEPTH_INC, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error incrementing depth" << std::endl;
        }
        else if (cmd == "d")
        {
            ret = clariusUserFunction(USER_FN_DEPTH_DEC, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error decrementing depth" << std::endl;
        }
        else if (cmd == "G")
        {
            ret = clariusUserFunction(USER_FN_GAIN_INC, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error incrementing gain" << std::endl;
        }
        else if (cmd == "g")
        {
            ret = clariusUserFunction(USER_FN_GAIN_DEC, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error decrementing gain" << std::endl;
        }
        else if (cmd == "R" || cmd == "r")
        {
            ret = clariusRequestRawData(0, 0, BLOCKINGCALL);
            if (ret < 0)
                ERROR << "error requesting raw data" << std::endl;
            else if (ret == 0)
                ERROR << "no raw data buffered" << std::endl;
            else
                PRINT << "raw data file of size " << ret << "B ready to download";
        }
        else if (cmd == "Y" || cmd == "y")
        {
            if (ret <= 0)
                ERROR << "no raw data to download" << std::endl;
            else
            {
                buffer = (char*)malloc(ret);

                if (clariusReadRawData((void**)(&buffer), BLOCKINGCALL) < 0)
                    ERROR << "error download raw data" << std::endl;
                else
                    PRINT << "successfully downloaded raw data" << std::endl;

                // save raw data to disk as a compressed file
                #ifdef _MSC_VER
                    fopen_s(&fp, "raw_data.tar", "wb+");
                #else
                    fp = fopen("raw_data.tar", "wb+");
                #endif
                fwrite(buffer, ret, 1, fp);
                fclose(fp);
                free(buffer);
                buffer = nullptr;
                ret = 0;
            }
        }
        else
        {
            PRINT << "valid commands: [q: quit]" << std::endl;
            PRINT << "       imaging: [f: freeze, d/D: depth, g/G: gain]" << std::endl;
            PRINT << "      raw data: [r: request, y: download]" << std::endl;
        }
    }
}

int init(int& argc, char** argv)
{
    const int width  = 640;
    const int height = 480;
    // ensure console buffers are flushed automatically
    setvbuf(stdout, nullptr, _IONBF, 0) != 0 || setvbuf(stderr, nullptr, _IONBF, 0);

    // Windows: Visual C++ doesn't have 'getopt' so use Boost's program_options instead
#ifdef _MSC_VER
    namespace po = boost::program_options;

    std::string keydir, ipAddr;
    unsigned int port = 0;

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
            return 1;
        }

        po::notify(vm);
    }
    catch(std::exception& e)
    {
        ERROR << "Error: " << e.what() << std::endl;
        return 2;
    }
    catch(...)
    {
        ERROR << "Unknown error!" << std::endl;
        return 3;
    }
#else // every other platform has 'getopt' which we're using so as to not pull in the Boost dependency
    int o;
    std::string keydir = "/tmp/", ipAddr;
    unsigned int port = 0;

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
        return ERRCODE;
    }

    if (!port)
    {
        ERROR << "no listen port provided. run with '-p [port]" << std::endl;
        return ERRCODE;
    }
#endif

    PRINT << "starting listener...";

    // initialize with callbacks
    if (clariusInitListener(argc, argv, keydir.c_str(), newProcessedImageFn, newRawImageFn, freezeFn, buttonFn, progressFn, errorFn, BLOCKINGCALL, width, height) < 0)
    {
        ERROR << "could not initialize listener" << std::endl;
        return 4;
    }
    if (clariusConnect(ipAddr.c_str(), port, BLOCKINGCALL) < 0)
    {
        ERROR << "could not connect to scanner" << std::endl;
        return 5;
    }
    PRINT << "...connected, streaming port: " << clariusGetUdpPort();

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

    clariusDestroyListener();
    return rcode;
}
