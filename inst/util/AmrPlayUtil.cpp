#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUrl>
#include "wave/amrPlay.h"
#include "AmrPlayUtil.h"

class AmrPlayMonitor : public CAmrPlayMonitor
{
public:
	explicit AmrPlayMonitor(AmrPlayUtil *pAmrPlayUtil)
		: m_pAmrPlayUtil(pAmrPlayUtil)
	{
	}

	virtual ~AmrPlayMonitor()
	{
		m_pAmrPlayUtil = 0;
	}

public:
	virtual void onPlayOver()
	{
		if (m_pAmrPlayUtil)
			m_pAmrPlayUtil->onStopped();
	}

	virtual void onPlayProgress(qint64 ms, qint64 allms)
	{
		Q_UNUSED(ms);
		Q_UNUSED(allms);
	}

private:
	AmrPlayUtil *m_pAmrPlayUtil;
};

//////////////////////////////////////////////////////////////////////////
AmrPlayUtil::AmrPlayUtil(QObject *parent /*= 0*/)
: QObject(parent)
, m_handleId(0)
{
	m_pAmrPlayMonitor.reset(new AmrPlayMonitor(this));
	m_pAmrPlayer.reset(new CAmrPlay(m_pAmrPlayMonitor.data()));
}

AmrPlayUtil::~AmrPlayUtil()
{
}

Q_INVOKABLE quint64 AmrPlayUtil::play(const QString &uuid, const QString &url)
{
	// stop previous audio
	if (!m_uuid.isEmpty())
	{
		m_pAmrPlayer->Stop();
	}

	// play new one
	QUrl u = QUrl::fromEncoded(url.toLatin1());
	QString path = u.toLocalFile();
	
	qWarning() << __FUNCTION__ << uuid << " " << path;

	if (m_pAmrPlayer->Play(path))
	{
		m_uuid = uuid;
		return ++m_handleId;
	}
	else
	{
		emit error(uuid, tr("Can't play this file, may be deleted or moved to other place"));
		return 0;
	}
}

void AmrPlayUtil::stop(const QString &uuid)
{
	if (m_uuid != uuid)
		return;

	qWarning() << __FUNCTION__ << uuid;

	m_pAmrPlayer->Stop();
	m_uuid = "";
}

void AmrPlayUtil::onStopped()
{
	if (!m_uuid.isEmpty())
	{
		qWarning() << __FUNCTION__ << m_uuid;

		emit stopped(m_uuid, m_handleId);

		m_uuid = "";
	}
}

AmrPlayUtil& AmrPlayUtil::instance()
{
	static AmrPlayUtil ins;
	return ins;
}
