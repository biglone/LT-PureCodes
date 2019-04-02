#include <QDebug>
#include <QString>
#include <QStringList>
#include <QMetaType>
#include "login/Account.h"
#include "PsgManager.h"

const char *OUT_ADDRESS_NAME = "outer"; // "外网"
const char *IN_ADDRESS_NAME  = "inner"; // "内网"

PsgManager::PSG::PSG() 
	: main(false) 
{
}


PsgManager::PsgManager()
{
	//read();
}

PsgManager::~PsgManager()
{
}

PsgManager& PsgManager::instance()
{
	static PsgManager inst;
	return inst;
}

void PsgManager::setPsgs(const QStringList& psgs)
{
	if (m_slPsgs.join(",") == psgs.join(","))
	{
		return;
	}

	clear();
	qWarning() << Q_FUNC_INFO << psgs;

	m_slPsgs = psgs;
	parse(psgs);
}

PsgManager::PSG PsgManager::getNextPsg() const
{
	PSG ret;
	do 
	{
		if (m_nNextPsg >= m_psgs.size())
			break;
		ret = m_psgs[m_nNextPsg];
		PsgManager* that = const_cast<PsgManager*>(this);
		++(that->m_nNextPsg);
	} while (0);

	return ret;
}

bool PsgManager::isEmpty() const
{
	return m_psgs.isEmpty();
}

bool PsgManager::isEnd() const
{
	if (m_nNextPsg == m_psgs.length())
		return true;
	return false;
}

void PsgManager::reset()
{
	m_nNextPsg = 0;
}

void PsgManager::clear()
{
	m_psgs.clear();
	m_nNextPsg = 0;
	m_slPsgs.clear();
}

void PsgManager::parse(const QStringList& psgs)
{
	QStringList addr;
	QString ip, port;

	bool bFirst = true;
	foreach (QString str, psgs)
	{
		if (str.isEmpty() || str.indexOf(':') < 0)
			continue;

		PSG psg;
		QStringList strlst = str.split('/');
		if (strlst.isEmpty())
		{
			addr = str.split(':');
			ip = addr.value(0);
			port = addr.value(1);

			psg.outAddr.ip = ip.toUtf8().constData();
			psg.outAddr.port = port.toInt();

			psg.inAddr.ip = ip.toUtf8().constData();
			psg.inAddr.port = port.toInt();
		}
		else
		{
			addr = strlst.value(0).split(':');
			ip = addr.value(0);
			port = addr.value(1);
			psg.outAddr.ip = ip.toUtf8().constData();
			psg.outAddr.port = port.toInt();

			addr = strlst.value(1).split(':');
			ip = addr.value(0);
			port = addr.value(1);

			psg.inAddr.ip = ip.toUtf8().constData();
			psg.inAddr.port = port.toInt();
		}

		psg.outAddr.name = OUT_ADDRESS_NAME;
		psg.inAddr.name = IN_ADDRESS_NAME;

		if (bFirst)
		{
			psg.main = true;
			m_psgs.insert(0, psg);
			bFirst = false;
		}
		else
		{
			m_psgs.append(psg);
		}
	}
}
