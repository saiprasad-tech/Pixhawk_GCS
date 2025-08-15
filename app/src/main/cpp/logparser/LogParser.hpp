#pragma once
#include <string>
#include <vector>

namespace pixhawk {

struct LogEntry {
    int64_t timestamp;
    std::string message;
    std::string level;
    std::string component;
};

class LogParser {
public:
    LogParser();
    ~LogParser();
    
    bool parseLogFile(const std::string& logData);
    std::vector<LogEntry> getEntries() const;
    size_t getEntryCount() const;
    std::string getSummary() const;
    
private:
    std::vector<LogEntry> entries;
    void parseLine(const std::string& line);
};

} // namespace pixhawk