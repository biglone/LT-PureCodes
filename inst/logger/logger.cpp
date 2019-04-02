/*
 * Logger implementation
 * Created: 2013-07-26
 */


#include <iostream>
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QTextStream>

#include "Logger.h"

#ifdef LOGGABLE_TRACE
#define loggable_trace(x) QString("%1(0x%2) %3").arg(metaObject()->className(), QString::number(reinterpret_cast<qint64>(this), 16), x)
#else
#define loggable_trace(x) (x)
#endif

Logger* Logger::m_logger = 0;

static const char *typeName(Logger::MessageType type)
{
    switch (type)
    {
    case Logger::DebugMessage:
        return "DEBUG";
    case Logger::InformationMessage:
        return "INFO";
    case Logger::WarningMessage:
        return "WARNING";
    case Logger::ReceivedMessage:
        return "RECEIVED";
    case Logger::SentMessage:
        return "SENT";
    default:
        return "";
    }
}

static QString formatted(Logger::MessageType type, const QString& text)
{
    return QDateTime::currentDateTime().toString() + " " +
        QString::fromLatin1(typeName(type)) + " " +
        text;
}

class LoggerPrivate
{
public:
    LoggerPrivate(Logger *qq);

    Logger::LoggingType loggingType;
    QFile *logFile;
    QString logFilePath;
    Logger::MessageTypes messageTypes;

private:
    Logger *q;
};

LoggerPrivate::LoggerPrivate(Logger *qq)
    : loggingType(Logger::NoLogging),
    logFile(0),
    logFilePath("ClientLog.log"),
    messageTypes(Logger::AnyMessage),
    q(qq)
{
}

/// Constructs a new Logger.
///
/// \param parent

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    d = new LoggerPrivate(this);

    // make it possible to pass Logger::MessageType between threads
    qRegisterMetaType< Logger::MessageType >("Logger::MessageType");
}

Logger::~Logger()
{
    delete d;
}

/// Returns the default logger.
///

Logger* Logger::getLogger()
{
    if(!m_logger)
        m_logger = new Logger();

    return m_logger;
}

/// Returns the handler for logging messages.
///

Logger::LoggingType Logger::loggingType()
{
    return d->loggingType;
}

/// Sets the handler for logging messages.
///
/// \param type

void Logger::setLoggingType(Logger::LoggingType type)
{
    if (d->loggingType != type) {
        d->loggingType = type;
        reopen();
    }
}

/// Returns the types of messages to log.
///

Logger::MessageTypes Logger::messageTypes()
{
    return d->messageTypes;
}

/// Sets the types of messages to log.
///
/// \param types

void Logger::setMessageTypes(Logger::MessageTypes types)
{
    d->messageTypes = types;
}

/// Add a debug logging message.
///
/// \param message

void Logger::debug(const QString &message)
{
	log(Logger::DebugMessage, loggable_trace(message));
}

/// Add a info logging message.
///
/// \param message

void Logger::info(const QString &message)
{
	log(Logger::InformationMessage, loggable_trace(message));
}

/// Add a warning logging message.
///
/// \param message

void Logger::warning(const QString &message)
{
	log(Logger::WarningMessage, loggable_trace(message));
}

/// Add a received logging message.
///
/// \param message

void Logger::logReceived(const QString &message)
{
	log(Logger::ReceivedMessage, loggable_trace(message));
}

/// Add a sent logging message.
///
/// \param message

void Logger::logSent(const QString &message)
{
	log(Logger::SentMessage, loggable_trace(message));
}

/// Add a logging message.
///
/// \param type
/// \param text

void Logger::log(Logger::MessageType type, const QString& text)
{
    // filter messages
    if (!d->messageTypes.testFlag(type))
        return;

    switch(d->loggingType)
    {
    case Logger::FileLogging:
        if (!d->logFile) {
            d->logFile = new QFile(d->logFilePath);
            d->logFile->open(QIODevice::WriteOnly | QIODevice::Append);
        }
        QTextStream(d->logFile) << formatted(type, text) << "\n";
        break;
    case Logger::StdoutLogging:
        std::cout << qPrintable(formatted(type, text)) << std::endl;
        break;
    case Logger::SignalLogging:
        emit message(type, text);
        break;
    default:
        break;
    }
}

/// Returns the path to which logging messages should be written.
///
/// \sa loggingType()

QString Logger::logFilePath()
{
    return d->logFilePath;
}

/// Sets the path to which logging messages should be written.
///
/// \param path
///
/// \sa setLoggingType()

void Logger::setLogFilePath(const QString &path)
{
    if (d->logFilePath != path) {
        d->logFilePath = path;
        reopen();
    }
}

/// If logging to a file, causes the file to be re-opened.
///

void Logger::reopen()
{
    if (d->logFile) {
        delete d->logFile;
        d->logFile = 0;
    }
}

