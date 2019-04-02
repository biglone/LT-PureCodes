#include "messageparser.h"
#include <QDebug>

MessageParser::MessageParser(const QString &id, MessageParserObserver &observer)
	: m_observer(observer), m_id(id), m_parseStarted(false)
{
	m_xmlReader.reset(new QXmlSimpleReader());
	m_xmlSource.reset(new QXmlInputSource());
	m_xmlReader->setContentHandler(this);
	m_xmlReader->setErrorHandler(this);
}

MessageParser::~MessageParser()
{
}

void MessageParser::addData(const QByteArray &data)
{
	if (!m_parseStarted)
	{
		QByteArray parseData = QByteArray("<stream>") + data; // stream means start
		m_xmlSource->setData(parseData);
		m_xmlReader->parse(m_xmlSource.data(), true);
		m_parseStarted = true;
	}
	else
	{
		m_xmlSource->setData(data);
		m_xmlReader->parseContinue();
	}
}

bool MessageParser::startElement(const QString & /*namespaceURI*/, 
	                             const QString & /*localName*/,
	                             const QString &qName, 
	                             const QXmlAttributes &attributes)
{
	if (qName == "stream") 
	{
		qDebug() << "stream started";
	}
	else if (qName == "message")
	{
		m_currentType = attributes.value("type");
		m_currentId = attributes.value("id");
		m_currentText.clear();
	}
	else
	{
		m_errorStr = QString("error message format from %1: %2").arg(m_id).arg(qName);
		m_observer.onError(m_id, m_errorStr);
		return false;
	}

	return true;
}

bool MessageParser::endElement(const QString & /*namespaceURI*/, 
	                           const QString & /*localName*/,
	                           const QString &qName)
{
	if (qName == "stream")
	{
		qDebug() << "stream finished";
	}
	else if (qName == "message") 
	{
		m_observer.onMessage(m_id, m_currentId, m_currentType, m_currentText);
	}
	else
	{
		m_errorStr = QString("error message format from %1: %2").arg(m_id).arg(qName);
		m_observer.onError(m_id, m_errorStr);
		return false;
	}
	return true;
}

bool MessageParser::characters(const QString &str)
{
	m_currentText += str;
	return true;
}

bool MessageParser::fatalError(const QXmlParseException &exception)
{
	m_errorStr = exception.message();
	m_observer.onError(m_id, m_errorStr);
	return false;
}

QString MessageParser::errorString() const
{
	return m_errorStr;
}
