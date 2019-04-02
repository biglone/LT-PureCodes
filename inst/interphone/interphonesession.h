#ifndef INTERPHONESESSION_H
#define INTERPHONESESSION_H

#include <QObject>
#include "recvbegingenerator.h"
#include <QScopedPointer>
#include <QTimer>

class AudioES;
class AudioRecv;

namespace interphone
{
	class InterphoneSession : public QObject, public RecvBeginGenerator
	{
		Q_OBJECT

	public:
		enum StartType
		{
			StartOK,
			InputError,
			OutputError
		};

	public:
		InterphoneSession(const QString &id, const QString &uid, const QString &addr, QObject *parent = 0);
		~InterphoneSession();

		QString id() const { return m_id; }
		QString uid() const { return m_uid; }
		QString addr() const { return m_addr; }

		StartType start();
		void stop();

		bool startTalk();
		bool stopTalk();

	Q_SIGNALS:
		void recvChannelFailed();

	public: // from RecvBeginGenerator
		QByteArray generateRecvBegin();

	private slots:
		void onAudioRecvBegin();
		void onRecvBeginTimeout();

	private:
		bool    m_startOK;

		QString m_id;
		QString m_uid;
		QString m_addr;

		QTimer  m_recvBeginTimer;

		QScopedPointer<AudioES>   m_audioES;
		QScopedPointer<AudioRecv> m_audioRecv;
	};

}

#endif // INTERPHONESESSION_H
