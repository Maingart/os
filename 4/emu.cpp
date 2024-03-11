#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <string>
#include <thread>
#include <random>
#include <filesystem>
#include <string_view>

#include <boost/asio.hpp>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


class EmulatingDevice {
public:
    EmulatingDevice(const std::string_view& serialPort) : serialPort_(serialPort) {}

    double GetRandomTemp()
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        return distribution(generator) * 1000 / 100;
    }

    void StartTransmission() {
        boost::asio::io_service io;
        boost::asio::serial_port serial(io);

        
        serial.open(serialPort_);

        if (!serial.is_open()) {
            exit(-1);
        }

#ifdef _WIN32
        HANDLE serialHandle = serial.native_handle();

        DCB serialParams = { sizeof(serialParams) };
        if (!GetCommState(serialHandle, &serialParams)) {
            exit(-1);
        }

        serialParams.fOutxCtsFlow = false;
        serialParams.fOutxDsrFlow = false;
        serialParams.fDtrControl = DTR_CONTROL_DISABLE;
        serialParams.fRtsControl = RTS_CONTROL_DISABLE;

        if (!SetCommState(serialHandle, &serialParams)) {
            exit(1);
        }
#endif

        while (true) {
            std::string data = std::to_string(this->GetRandomTemp());
            data.resize(5, '0');
            data += "\n";

            if (SendDataToSerial(serial, data)) {
                WriteToLog("Written: " + data);
            }
            else {
                serial.close();
                exit(-1);
            }

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

private:
    std::string serialPort_;

    bool SendDataToSerial(boost::asio::serial_port& serial, const std::string& data) {
        boost::system::error_code error;
        size_t bytesWritten = write(serial, boost::asio::buffer(data), error);

        return bytesWritten == data.size();
    }

    

    void WriteToLog(const std::string& message) {
        std::ofstream of;
        of.open("emulator_logfile.txt", std::ios::app);
        if (of.is_open()) {
            auto now = std::chrono::system_clock::now();
            time_t currentTime = std::chrono::system_clock::to_time_t(now);

            struct std::tm localTime;
#ifdef _WIN32
            localtime_s(&localTime, &currentTime);
#else
            localtime_r(&currentTime, &localTime);
#endif

            of << "emulator_logfile.txt" << " at " << std::put_time(&localTime, "%Y-%m-%d %X") << message << "\n";

            of.close();
        }

    }
};



int main(int argc, char** argv) 
{
    std::string serialPort = argv[1];
    EmulatingDevice emulator(serialPort);
    emulator.StartTransmission();

    return 0;
}
