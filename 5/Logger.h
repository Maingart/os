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
#include "sqlite3.h"
#include "prompts.h"

namespace Logger
{
	inline sqlite3* db;

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

	void logTemperature(const double temperature, const std::string& tableName) {
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm* ptm = std::localtime(&now);
		
		
		char timestamp[20];
		std::strftime(timestamp, sizeof(timestamp), "%Y/%m/%d %H:%M:%S", ptm);

		auto prompt = prompts::InsertPromptr(tableName, timestamp, temperature);

		sqlite3_exec(db, prompt.str().c_str(), 0, 0, 0);
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

	void clearOldEntries(const std::string& tableName, const std::chrono::system_clock::time_point& threshold) {
		char timestamp[20];
		auto time = std::chrono::system_clock::to_time_t(threshold);
		std::tm* ptm = std::localtime(&time);
		
		std::strftime(timestamp, sizeof(timestamp), "%Y/%m/%d %H:%M:%S", ptm);

		auto deletePrompt = prompts::DeletePrompt(tableName, timestamp);

		sqlite3_exec(db, deletePrompt.c_str(), 0, 0, 0);
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
					clearOldEntries(settings::LOG_FILE_DAY, curTime - std::chrono::hours(24 * 365));
					settings::lastYearUpdate = curTime;
				}

			}
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

	}


	void createDB() {
		if (auto status = sqlite3_open("DATABASE.db", &db); status) {
			std::cout << "Error opening DB\n";
			exit(-1);
		}

		if (auto status = sqlite3_exec(db, prompts::createTotalDB, 0, 0, 0); status) {
			std::cout << "Can't make prompt!\n";
			exit(-1);
		}

		if (auto status = sqlite3_exec(db, prompts::createHourDB, 0, 0, 0); status) {
			std::cout << "Can't make prompt!\n";
			exit(-1);
		}

		if (auto status = sqlite3_exec(db, prompts::createDayDB, 0, 0, 0); status) {
			std::cout << "Can't make prompt!\n";
			exit(-1);
		}
		
	}

}
#endif