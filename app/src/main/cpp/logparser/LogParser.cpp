#include "LogParser.hpp"
#include <sstream>
#include <algorithm>

namespace pixhawk {

LogParser::LogParser() = default;
LogParser::~LogParser() = default;

bool LogParser::parseLogFile(const std::string& logData) {
    entries.clear();
    
    std::istringstream stream(logData);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            parseLine(line);
        }
    }
    
    return !entries.empty();
}

std::vector<LogEntry> LogParser::getEntries() const {
    return entries;
}

size_t LogParser::getEntryCount() const {
    return entries.size();
}

std::string LogParser::getSummary() const {
    if (entries.empty()) {
        return "No log entries parsed";
    }
    
    std::ostringstream summary;
    summary << "Total entries: " << entries.size() << "\n";
    
    // Count by level
    int infoCount = 0, warnCount = 0, errorCount = 0;
    for (const auto& entry : entries) {
        if (entry.level == "INFO") infoCount++;
        else if (entry.level == "WARN") warnCount++;
        else if (entry.level == "ERROR") errorCount++;
    }
    
    summary << "INFO: " << infoCount << ", WARN: " << warnCount << ", ERROR: " << errorCount;
    
    return summary.str();
}

void LogParser::parseLine(const std::string& line) {
    // Simple log parsing - expects format like:
    // [TIMESTAMP] LEVEL COMPONENT: MESSAGE
    
    LogEntry entry;
    entry.timestamp = 0;
    entry.level = "INFO";
    entry.component = "UNKNOWN";
    entry.message = line;
    
    // Try to extract timestamp
    size_t timestampEnd = line.find(']');
    if (line[0] == '[' && timestampEnd != std::string::npos) {
        std::string timestampStr = line.substr(1, timestampEnd - 1);
        entry.timestamp = std::atoll(timestampStr.c_str());
        
        // Extract rest of line after timestamp
        std::string remainder = line.substr(timestampEnd + 1);
        
        // Extract level
        std::istringstream stream(remainder);
        if (stream >> entry.level) {
            // Extract component
            std::string componentWithColon;
            if (stream >> componentWithColon) {
                if (componentWithColon.back() == ':') {
                    entry.component = componentWithColon.substr(0, componentWithColon.length() - 1);
                    
                    // Rest is message
                    std::getline(stream, entry.message);
                    // Remove leading whitespace
                    entry.message.erase(0, entry.message.find_first_not_of(" \t"));
                }
            }
        }
    }
    
    entries.push_back(entry);
}

} // namespace pixhawk