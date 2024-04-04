#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "settings.h"

namespace Logger
{

#ifdef _WIN32
    inline HANDLE hSerial;
#else
    inline int hSerial;
#endif

    inline std::string portName;
    inline std::vector<double> hourTemp;
    inline double dayTemp = 0.0;
    inline int timesInDay = 0;

    enum TYPE_OF_TIME {
        HOUR = 0,
        DAY = 1
    };


    void OpenPort(const std::string& portName) {
#ifdef _WIN32
        hSerial = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#else
        hSerial = open(portName.c_str(), O_RDWR | O_NOCTTY);
#endif

        //configure for win & linux
        auto confPort = [](auto hSerial)
        {
#ifdef _WIN32
            DCB portParams = { 0 };
				portParams.DCBlength = sizeof(portParams);
				if (!GetCommState(hSerial, &portParams)) {
					std::cout << "Failed to get serial port state" << std::endl;
					exit(-1);
				}

				portParams.fOutxCtsFlow = false;
				portParams.fOutxDsrFlow = false;
				portParams.fDtrControl = DTR_CONTROL_DISABLE;
				portParams.fRtsControl = RTS_CONTROL_DISABLE;

				if (!SetCommState(hSerial, &portParams)) {
					std::cout << "Failed to set serial port state" << std::endl;
					exit(-1);
				}
#else
            termios tty;
            tcgetattr(hSerial, &tty);

            tty.c_cflag &= ~CRTSCTS;
            tty.c_cflag &= ~CLOCAL;
            tty.c_cflag &= ~HUPCL;

            if (tcsetattr(hSerial, TCSANOW, &tty) != 0) {
                exit(-1);
            }
#endif
        };
        confPort(hSerial);

    }

    void ClosePort() {
#ifdef _WIN32
        CloseHandle(hSerial);
#else
        close(hSerial);
#endif
    }

    void logTemperature(const double temperature, const std::string& filename) {
        std::ofstream logfile(filename, std::ios::app);
        if (logfile.is_open()) {
            time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            struct std::tm localTime;
#ifdef _WIN32
            localtime_s(&localTime, &curTime);
#else
            localtime_r(&curTime, &localTime);
#endif
            logfile << std::put_time(&localTime, settings::time_format.c_str()) << temperature << " C\n";
        }
    }

    void updateTime(TYPE_OF_TIME tot /*param1 = Enum of types: HOUR/DAY etc*/) {
        switch (tot)
        {
            case Logger::HOUR:
                if (!hourTemp.empty()) {
                    double sum = 0.0;
                    for (double temp : hourTemp) {
                        sum += temp;
                    }
                    double average = sum / hourTemp.size();
                    logTemperature(average, settings::LOG_FILE_HOUR);
                    hourTemp.clear();
                }
                break;
            case Logger::DAY:
                if (timesInDay > 0) {
                    double average = dayTemp / timesInDay;
                    logTemperature(average, settings::LOG_FILE_DAY);
                    dayTemp = 0.0;
                    dayTemp = 0;
                }
                break;
            default:
                break;
        }
    }

    void clearOldEntries(const std::string& filename, const std::chrono::system_clock::time_point& threshold) {
        std::ifstream inputFile(filename);

        // replace with temp file
        std::string prevFileName = "old" + filename;
        std::ofstream outputFile(prevFileName);

        std::string line;
        while (std::getline(inputFile, line)) {
            auto time = line.substr(1, 19);
            std::tm old_time = {};
            std::istringstream(time) >> std::get_time(&old_time, settings::time_format.c_str());
            auto time2 = std::chrono::system_clock::from_time_t(std::mktime(&old_time));

            if (time2 >= threshold) {
                outputFile << line << '\n';
            }
        }

        inputFile.close();
        outputFile.close();

        std::remove(filename.c_str());
        std::rename(prevFileName.c_str(), filename.c_str());
    }

    void StartLogging() {
        while (true) {
            char data[512];
            size_t bytesRead = 0;
#ifdef _WIN32
            ReadFile(hSerial, data, sizeof(data), (LPDWORD)&bytesRead, nullptr);
#else
            bytesRead = read(hSerial, data, sizeof(data));
#endif
            if (bytesRead > 0) {
                double temperature = std::stod(data);
                logTemperature(temperature, settings::LOG_FILE_TOTAL);
                auto curTime = std::chrono::system_clock::now();

                if (curTime - settings::totalLogUpdate > std::chrono::hours(24)) {
                    clearOldEntries(settings::LOG_FILE_TOTAL, std::chrono::system_clock::now() - std::chrono::hours(24));
                    settings::totalLogUpdate = curTime;
                }

                hourTemp.push_back(temperature);

                if (curTime - settings::lastHoursUpdate > settings::HOUR) {
                    updateTime(TYPE_OF_TIME::HOUR);
                    settings::lastHoursUpdate = curTime;
                }

                if (curTime - settings::lastDayUpdate > settings::DAY) {
                    updateTime(TYPE_OF_TIME::DAY);
                    settings::lastDayUpdate = curTime;
                }

                if (curTime - settings::lastMonthUpdate > std::chrono::hours(24 * 30)) {
                    clearOldEntries(settings::LOG_FILE_HOUR, curTime - std::chrono::hours(24 * 30));
                    settings::lastMonthUpdate = curTime;
                }

                dayTemp += temperature;
                timesInDay++;

                if (curTime - settings::lastYearUpdate > std::chrono::hours(24 * 365)) {
                    clearOldEntries(settings::LOG_FILE_HOUR, curTime - std::chrono::hours(24 * 365));
                    settings::lastYearUpdate = curTime;
                }

            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }

    }

}
#endif