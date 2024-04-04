#ifndef PROMPTS_H
#define PROMPTS_H

#include <string>
#include <sstream>

namespace prompts
{
    inline auto createTotalDB =
        "CREATE TABLE IF NOT EXISTS total (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        timestamp DATETIME,\
        temperature REAL);";
    
    inline auto createDayDB =
        "CREATE TABLE IF NOT EXISTS day (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        timestamp DATETIME,\
        temperature REAL);";

    inline auto createHourDB =
        "CREATE TABLE IF NOT EXISTS hour (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        timestamp DATETIME,\
        temperature REAL);";

    inline auto InsertPromptr(const std::string& tableName, const std::string& time, double temp) {
        std::stringstream insertQuery;
        insertQuery << "INSERT INTO " << tableName << " (timestamp, temperature) VALUES ('" << time << "', " << temp << ");";
        return insertQuery;
    }

    inline auto DeletePrompt(const std::string& tableName, const std::string& time) {
        std::string deletePrompt = "DELETE FROM " + tableName + " WHERE timestamp < '" + time + "';";
        return deletePrompt;
    }

}

#endif