#ifndef _PSGMANAGER_H_
#define _PSGMANAGER_H_
#include <vector>
#include <QList>
#include <QStringList>
#include "base/Base.h"

extern const char *OUT_ADDRESS_NAME;
extern const char *IN_ADDRESS_NAME;

class PsgManager
{
public:
	struct PSG 
	{
		PSG();
		base::Address outAddr;
		base::Address inAddr;
		bool main;
		bool isValid()
		{
			return outAddr.isValid() || inAddr.isValid();
		}
	};

private:
	PsgManager();
	virtual ~PsgManager();

public:
	static PsgManager& instance();

	void setPsgs(const QStringList& psgs);
	PsgManager::PSG getNextPsg() const;
	bool isEmpty() const;
	bool isEnd() const;
	void reset();
	void clear();
	QList<PSG> psgs() const {return m_psgs;}

private:
	void parse(const QStringList& psgs);

private:
	QList<PSG> m_psgs;
	int m_nNextPsg;
	QStringList m_slPsgs;
};

#endif // _PSGMANAGER_H_