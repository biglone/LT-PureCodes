/*
 * Logger implementation
 * Created: 2013-07-26
 */


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <QObject>

class LoggerPrivate;

/// \brief The Logger class represents a sink for logging messages.
///

class Logger : public QObject
{
    Q_OBJECT
    Q_ENUMS(LoggingType)
    Q_FLAGS(MessageType MessageTypes)
    Q_PROPERTY(QString logFilePath READ logFilePath WRITE setLogFilePath)
    Q_PROPERTY(LoggingType loggingType READ loggingType WRITE setLoggingType)
    Q_PROPERTY(MessageTypes messageTypes READ messageTypes WRITE setMessageTypes)

public:
    /// This enum describes how log message are handled.
    enum LoggingType
    {
        NoLogging = 0,      ///< Log messages are discarded
        FileLogging = 1,    ///< Log messages are written to a file
        StdoutLogging = 2,  ///< Log messages are written to the standard output
        SignalLogging = 4,  ///< Log messages are emitted as a signal
    };

    /// This enum describes a type of log message.
    enum MessageType
    {
        NoMessage = 0,          ///< No message type
        DebugMessage = 1,       ///< Debugging message
        InformationMessage = 2, ///< Informational message
        WarningMessage = 4,     ///< Warning message
        ReceivedMessage = 8,    ///< Message received from server
        SentMessage = 16,       ///< Message sent to server
        AnyMessage = 31,        ///< Any message type
    };
    Q_DECLARE_FLAGS(MessageTypes, MessageType)

    Logger(QObject *parent = 0);
    ~Logger();

    static Logger* getLogger();

    Logger::LoggingType loggingType();
    void setLoggingType(Logger::LoggingType type);

    QString logFilePath();
    void setLogFilePath(const QString &path);

    Logger::MessageTypes messageTypes();
    void setMessageTypes(Logger::MessageTypes types);

public slots:
	void debug(const QString &message);
	void info(const QString &message);
	void warning(const QString &message);
	void logReceived(const QString &message);
	void logSent(const QString &message);
    void log(Logger::MessageType type, const QString& text);
    void reopen();

signals:
    /// This signal is emitted whenever a log message is received.
    void message(Logger::MessageType type, const QString &text);

private:
    static Logger* m_logger;
    LoggerPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Logger::MessageTypes)
#endif // __LOGGER_H__
