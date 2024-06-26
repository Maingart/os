#pragma once
#include <string>
#include <chrono>

namespace settings
{
	const std::string LOG_FILE_TOTAL = "total";
	const std::string LOG_FILE_DAY = "day";
	const std::string LOG_FILE_HOUR = "hour";
	const std::string time_format = "%Y/%m/%d %H:%M:%S";

	inline const std::chrono::hours HOUR{ 1 };
	inline const std::chrono::hours DAY{ 24 };

	inline std::chrono::system_clock::time_point lastHoursUpdate = std::chrono::system_clock::now();
	inline std::chrono::system_clock::time_point lastDayUpdate = std::chrono::system_clock::now();
	inline std::chrono::system_clock::time_point lastMonthUpdate = std::chrono::system_clock::now();
	inline std::chrono::system_clock::time_point lastYearUpdate = std::chrono::system_clock::now();
	inline std::chrono::system_clock::time_point totalLogUpdate = std::chrono::system_clock::now();
}