#ifndef __MESSAGE_PARSER_H__
#define __MESSAGE_PARSER_H__

#include <QXmlDefaultHandler>
#include <QScopedPointer>
#include <QXmlSimpleReader>
#include <QXmlInputSource>

class MessageParserObserver
{
public:
	virtual void onMessage(const QString &sessionId, const QString &id, const QString &typeStr, const QString &json) = 0;
	virtual void onError(const QString &sessionId, const QString &desc) = 0;
};

class MessageParser : public QXmlDefaultHandler
{
public:
	MessageParser(const QString &id, MessageParserObserver &observer);
	~MessageParser();

	void addData(const QByteArray &data);

	bool startElement(const QString &namespaceURI, const QString &localName,
		const QString &qName, const QXmlAttributes &attributes) Q_DECL_OVERRIDE;
	bool endElement(const QString &namespaceURI, const QString &localName,
		const QString &qName) Q_DECL_OVERRIDE;
	bool characters(const QString &str) Q_DECL_OVERRIDE;
	bool fatalError(const QXmlParseException &exception) Q_DECL_OVERRIDE;
	QString errorString() const Q_DECL_OVERRIDE;

private:
	MessageParserObserver           &m_observer;
	QString                          m_id;
	QScopedPointer<QXmlSimpleReader> m_xmlReader;
	QScopedPointer<QXmlInputSource>  m_xmlSource;
	bool                             m_parseStarted;
	QString                          m_currentText;
	QString                          m_currentType;
	QString                          m_currentId;
	QString                          m_errorStr;
};

#endif // __MESSAGE_PARSER_H__
