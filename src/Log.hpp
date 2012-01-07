#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <sstream>
#include <cstdio>


namespace sphere {

    class Log {
    public:
        enum ReportingLevel {
            RL_INFO = 0,
            RL_WARNING,
            RL_ERROR,
            RL_DEBUG,
        };

    private:
        class Message {
        public:
            Message(ReportingLevel level, const Log& log) : _reportingLevel(level), _log(log) { }
            ~Message() { _log.write(_reportingLevel, _msg.str()); }
            template<typename T> Message& operator<<(T t) { _msg << t; return *this; }
        private:
            std::ostringstream _msg;
            ReportingLevel _reportingLevel;
            const Log& _log;
        };

    public:
        Log();
        explicit Log(const std::string& filename);
        ~Log();

        Message info() const;
        Message warning() const;
        Message error() const;
        Message debug() const;

        bool isOpen() const;
        bool open(const std::string& filename);
        void close();
        void setReportingLevel(ReportingLevel level);
        void write(ReportingLevel level, const std::string& msg) const;

    private:
        FILE* _file;
        ReportingLevel _reportingLevel;
    };

    //-----------------------------------------------------------------
    inline
    Log::Log()
        : _file(0)
        , _reportingLevel(RL_DEBUG)
    {
    }

    //-----------------------------------------------------------------
    inline
    Log::Log(const std::string& filename)
        : _file(0)
        , _reportingLevel(RL_DEBUG)
    {
        open(filename);
    }

    //-----------------------------------------------------------------
    inline
    Log::~Log()
    {
    }

    //-----------------------------------------------------------------
    inline bool
    Log::isOpen() const
    {
        return _file != 0;
    }

    //-----------------------------------------------------------------
    inline void
    Log::setReportingLevel(ReportingLevel level)
    {
        _reportingLevel = level;
    }

    //-----------------------------------------------------------------
    inline Log::Message
    Log::info() const
    {
        return Message(RL_INFO, *this);
    }

    //-----------------------------------------------------------------
    inline Log::Message
    Log::warning() const
    {
        return Message(RL_WARNING, *this);
    }

    //-----------------------------------------------------------------
    inline Log::Message
    Log::error() const
    {
        return Message(RL_ERROR, *this);
    }

    //-----------------------------------------------------------------
    inline Log::Message
    Log::debug() const
    {
        return Message(RL_DEBUG, *this);
    }

} // namespace sphere


#endif
