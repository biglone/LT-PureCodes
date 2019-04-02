#include "favoriteemotionsettings.h"
#include <QSettings>
#include <QUuid>
#include <QStringList>

static const char *kszFileName   = "fileName";
static const char *kszShortcut   = "shortcut";
static const char *kszMd5Hex     = "md5Hex";
static const char *kszId         = "id";
static const char *kszIds        = "ids";
static const char *kszGroups     = "groups";
static const char *kszGroup      = "group";
static const char *kszGroupName  = "groupName";
static const char *kszRemovable  = "removable";

FavoriteEmotionSettings::FavoriteEmotionSettings(const QString &fileName, QObject *parent /*= 0*/)
	: QObject(parent)
{
	m_settings = new QSettings(fileName, QSettings::IniFormat, this);
	
	// get all groups
	int i = 0;
	int size = 0;
	FavoriteEmotionSettings::EmotionGroup emotionGroup;
	size = m_settings->beginReadArray(kszGroups);
	for (i = 0; i < size; ++i)
	{
		m_settings->setArrayIndex(i);
		emotionGroup.groupId = m_settings->value(kszId, "").toString();
		emotionGroup.groupName = m_settings->value(kszGroupName, "").toString();
		emotionGroup.removable = m_settings->value(kszRemovable, true).toBool();
		m_groups.append(emotionGroup);
	}
	m_settings->endArray();

	if (m_groups.isEmpty())
	{
		// add the first default group
		emotionGroup.groupId = QString::fromLatin1(kFirstGroupId);
		emotionGroup.groupName = QObject::tr(kFirstGroupName);
		m_groups.append(emotionGroup);
		setGroups(m_groups);

		m_emotions.insert(QString::fromLatin1(kFirstGroupId), QList<FavoriteEmotionSettings::EmotionItem>());
	}
	else
	{
		// get all emotions
		QString groupId;
		QList<FavoriteEmotionSettings::EmotionItem> emotions;
		FavoriteEmotionSettings::EmotionItem emotionItem;
		for (i = 0; i < m_groups.count(); ++i)
		{
			emotionGroup = m_groups[i];
			groupId = emotionGroup.groupId;
			emotions.clear();

			m_settings->beginGroup(groupId);
			size = m_settings->beginReadArray(kszIds);
			for (int j = 0; j < size; ++j)
			{
				m_settings->setArrayIndex(j);
				emotionItem.emotionId = m_settings->value(kszId, "").toString();
				emotionItem.fileName = m_settings->value(kszFileName, "").toString();
				emotionItem.shortcut = m_settings->value(kszShortcut, "").toString();
				emotionItem.md5Hex = m_settings->value(kszMd5Hex, "").toString();
				emotions.append(emotionItem);
			}
			m_settings->endArray();
			m_settings->endGroup();
			
			m_emotions.insert(groupId, emotions);
		}
	}
}

FavoriteEmotionSettings::~FavoriteEmotionSettings()
{

}

QString FavoriteEmotionSettings::addFavoriteEmotion(const QString &fileName, const QString &shortcut, const QString &md5Hex, const QString &groupId)
{
	QString emotionId;
	if (fileName.isEmpty() || md5Hex.isEmpty() || groupId.isEmpty())
		return emotionId;

	if (findGroup(groupId) < 0)
		return emotionId;

	QString uuidStr = QUuid::createUuid().toString();
	emotionId = uuidStr.mid(1, uuidStr.length()-2);

	FavoriteEmotionSettings::EmotionItem emotionItem;
	emotionItem.emotionId = emotionId;
	emotionItem.fileName = fileName;
	emotionItem.shortcut = shortcut;
	emotionItem.md5Hex = md5Hex;
	QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
	emotions.append(emotionItem);
	m_emotions[groupId] = emotions;
	setEmotions(emotions, groupId);

	return emotionId;
}

bool FavoriteEmotionSettings::renameFavoriteEmotion(const QString &emotionId, const QString &shortcut)
{
	if (emotionId.isEmpty())
		return false;

	foreach (QString groupId, m_emotions.keys())
	{
		QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
		for (int i = 0; i < emotions.count(); ++i)
		{
			FavoriteEmotionSettings::EmotionItem em = emotions[i];
			if (em.emotionId == emotionId)
			{
				em.shortcut = shortcut;
				emotions[i] = em;
				m_emotions[groupId] = emotions;
				setEmotions(emotions, groupId);
				return true;
			}
		}
	}

	return false;
}

bool FavoriteEmotionSettings::delFavoriteEmotion(const QString &emotionId, const QString &groupId)
{
	if (emotionId.isEmpty() || groupId.isEmpty())
		return false;

	int index = findEmotion(emotionId, groupId);
	if (index < 0)
		return false;

	// remove and save
	QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
	emotions.removeAt(index);
	m_emotions[groupId] = emotions;
	setEmotions(emotions, groupId);
	
	return true;
}

bool FavoriteEmotionSettings::moveFirstFavoriteEmotion(const QString &emotionId, const QString &groupId)
{
	if (emotionId.isEmpty() || groupId.isEmpty())
		return false;

	int index = findEmotion(emotionId, groupId);
	if (index < 0)
		return false;

	if (index == 0)
		return true;

	// adjust the sequence and save
	QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
	FavoriteEmotionSettings::EmotionItem emotionItem = emotions[index];
	emotions.removeAt(index);
	emotions.insert(0, emotionItem);
	m_emotions[groupId] = emotions;
	setEmotions(emotions, groupId);

	return true;
}

FavoriteEmotionSettings::EmotionItem FavoriteEmotionSettings::emotion(const QString &emotionId) const
{
	FavoriteEmotionSettings::EmotionItem emotionItem;
	if (emotionId.isEmpty())
		return emotionItem;

	foreach (QString groupId, m_emotions.keys())
	{
		QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
		foreach (FavoriteEmotionSettings::EmotionItem em, emotions)
		{
			if (em.emotionId == emotionId)
			{
				return em;
			}
		}
	}

	return emotionItem;
}

QList<FavoriteEmotionSettings::EmotionItem> FavoriteEmotionSettings::allEmotions(const QString &groupId) const
{
	QList<FavoriteEmotionSettings::EmotionItem> emotions;
	emotions = m_emotions.value(groupId, QList<FavoriteEmotionSettings::EmotionItem>());
	return emotions;
}

QList<FavoriteEmotionSettings::EmotionGroup> FavoriteEmotionSettings::allGroups() const
{
	return m_groups;
}

QString FavoriteEmotionSettings::groupName(const QString &groupId) const
{
	QString name;
	int index = findGroup(groupId);
	if (index < 0)
		return name;

	FavoriteEmotionSettings::EmotionGroup groupItem = m_groups[index];
	return groupItem.groupName;
}

bool FavoriteEmotionSettings::setGroupName(const QString &groupId, const QString &name, QString &newGroupId)
{
	int index = findGroup(groupId);
	if (index < 0)
		return false;

	newGroupId = groupId;
	FavoriteEmotionSettings::EmotionGroup groupItem = m_groups[index];
	if (groupItem.groupName == name)
		return true;

	if (name == QObject::tr(kFirstGroupName))
	{
		newGroupId = QString::fromLatin1(kFirstGroupId);
		FavoriteEmotionSettings::EmotionGroup newGroup;
		newGroup.groupId = newGroupId;
		newGroup.groupName = name;
		m_groups.insert(index, newGroup);
		setGroups(m_groups);
		moveFavoriteEmotionGroup(groupId, newGroupId);
		delGroup(groupId);
	}
	else if (groupItem.groupName == QObject::tr(kFirstGroupName))
	{
		QString uuidStr = QUuid::createUuid().toString();
		newGroupId = uuidStr.mid(1, uuidStr.length()-2);
		FavoriteEmotionSettings::EmotionGroup newGroup;
		newGroup.groupId = newGroupId;
		newGroup.groupName = name;
		m_groups.insert(index, newGroup);
		setGroups(m_groups);
		moveFavoriteEmotionGroup(groupId, newGroupId);
		delGroup(groupId);
	}
	else
	{
		groupItem.groupName = name;
		m_groups[index] = groupItem;
		setGroups(m_groups);
	}
	
	return true;
}

QString FavoriteEmotionSettings::addGroup(const QString &groupName)
{
	QString groupId;
	if (groupName.isEmpty())
		return groupId;

	if (groupName == QObject::tr(kFirstGroupName))
	{
		groupId = QString::fromLatin1(kFirstGroupId);
	}
	else
	{
		QString uuidStr = QUuid::createUuid().toString();
		groupId = uuidStr.mid(1, uuidStr.length()-2);
	}
	FavoriteEmotionSettings::EmotionGroup groupItem;
	groupItem.groupId = groupId;
	groupItem.groupName = groupName;
	m_groups.append(groupItem);
	setGroups(m_groups);

	return groupId;
}

bool FavoriteEmotionSettings::delGroup(const QString &groupId)
{
	int index = findGroup(groupId);
	if (index < 0)
		return false;

	m_groups.removeAt(index);
	setGroups(m_groups);

	m_emotions.remove(groupId);
	setEmotions(QList<FavoriteEmotionSettings::EmotionItem>(), groupId);

	return true;
}

bool FavoriteEmotionSettings::moveFavoriteEmotionGroup(const QString &fromGroupId, const QString &toGroupId)
{
	int fromIndex = findGroup(fromGroupId);
	int toIndex = findGroup(toGroupId);
	if (fromIndex < 0 || toIndex < 0)
		return false;

	QList<FavoriteEmotionSettings::EmotionItem> fromEmotions;
	fromEmotions = m_emotions.value(fromGroupId, QList<FavoriteEmotionSettings::EmotionItem>());
	QList<FavoriteEmotionSettings::EmotionItem> toEmotions;
	toEmotions = m_emotions.value(toGroupId, QList<FavoriteEmotionSettings::EmotionItem>());

	toEmotions.append(fromEmotions);
	fromEmotions.clear();
	m_emotions[toGroupId] = toEmotions;
	m_emotions[fromGroupId] = fromEmotions;
	setEmotions(toEmotions, toGroupId);
	setEmotions(fromEmotions, fromGroupId);
	return true;
}

bool FavoriteEmotionSettings::moveFavoriteEmotionNewGroup(const QString &fromGroupId, const QString &emotionId, const QString &toGroupId)
{
	int fromIndex = findGroup(fromGroupId);
	int toIndex = findGroup(toGroupId);
	if (fromIndex < 0 || toIndex < 0)
		return false;

	int emotionIndex = findEmotion(emotionId, fromGroupId);
	if (emotionIndex < 0)
		return false;

	QList<FavoriteEmotionSettings::EmotionItem> fromEmotions = m_emotions[fromGroupId];
	QList<FavoriteEmotionSettings::EmotionItem> toEmotions = m_emotions[toGroupId];
	FavoriteEmotionSettings::EmotionItem emotionItem = fromEmotions[emotionIndex];
	toEmotions.append(emotionItem);
	fromEmotions.removeAt(emotionIndex);
	m_emotions[fromGroupId] = fromEmotions;
	m_emotions[toGroupId] = toEmotions;
	setEmotions(toEmotions, toGroupId);
	setEmotions(fromEmotions, fromGroupId);
	return true;
}

bool FavoriteEmotionSettings::hasDuplicateEmotion(const QString &md5Hex) const
{
	foreach (QString groupId, m_emotions.keys())
	{
		QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
		foreach (FavoriteEmotionSettings::EmotionItem emotionItem, emotions)
		{
			if (emotionItem.md5Hex == md5Hex)
			{
				return true;
			}
		}
	}
	return false;
}

bool FavoriteEmotionSettings::hasEmotion() const
{
	foreach (QString groupId, m_emotions.keys())
	{
		QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
		if (!emotions.isEmpty())
			return true;
	}
	return false;
}

bool FavoriteEmotionSettings::moveGroupBefore(const QString &groupId)
{
	if (groupId.isEmpty())
		return false;

	int index = findGroup(groupId);
	if (index < 0)
		return false;

	if (index == 0)
		return true;

	FavoriteEmotionSettings::EmotionGroup group = m_groups[index];
	m_groups.removeAt(index);
	m_groups.insert(index-1, group);
	setGroups(m_groups);
	return true;
}

bool FavoriteEmotionSettings::moveGroupToTop(const QString &groupId)
{
	if (groupId.isEmpty())
		return false;

	int index = findGroup(groupId);
	if (index < 0)
		return false;

	if (index == 0)
		return true;

	FavoriteEmotionSettings::EmotionGroup group = m_groups[index];
	m_groups.removeAt(index);
	m_groups.insert(0, group);
	setGroups(m_groups);
	return true;
}

bool FavoriteEmotionSettings::moveGroupAfter(const QString &groupId)
{
	if (groupId.isEmpty())
		return false;

	int index = findGroup(groupId);
	if (index < 0)
		return false;

	if (index == m_groups.count()-1)
		return true;

	FavoriteEmotionSettings::EmotionGroup group = m_groups[index];
	m_groups.removeAt(index);
	m_groups.insert(index+1, group);
	setGroups(m_groups);
	return true;
}

void FavoriteEmotionSettings::setEmotions(const QList<FavoriteEmotionSettings::EmotionItem> &emotions, const QString &groupId)
{
	m_settings->beginGroup(groupId);
	m_settings->beginWriteArray(kszIds);
	int index = 0;
	foreach (FavoriteEmotionSettings::EmotionItem emotionItem, emotions)
	{
		m_settings->setArrayIndex(index);
		m_settings->setValue(kszId, emotionItem.emotionId);
		m_settings->setValue(kszFileName, emotionItem.fileName);
		m_settings->setValue(kszShortcut, emotionItem.shortcut);
		m_settings->setValue(kszMd5Hex, emotionItem.md5Hex);
		++index;
	}
	m_settings->endArray();
	m_settings->endGroup();
}

int FavoriteEmotionSettings::findEmotion(const QString &emotionId, const QString &groupId) const
{
	if (!m_emotions.contains(groupId))
		return -1;

	QList<FavoriteEmotionSettings::EmotionItem> emotions = m_emotions[groupId];
	int index = 0;
	foreach (FavoriteEmotionSettings::EmotionItem emotionItem, emotions)
	{
		if (emotionItem.emotionId == emotionId)
			return index;

		++index;
	}
	return -1;
}

void FavoriteEmotionSettings::setGroups(const QList<FavoriteEmotionSettings::EmotionGroup> &groups)
{
	m_settings->beginWriteArray(kszGroups);
	int index = 0;
	foreach (FavoriteEmotionSettings::EmotionGroup groupItem, groups)
	{
		m_settings->setArrayIndex(index);
		m_settings->setValue(kszId, groupItem.groupId);
		m_settings->setValue(kszGroupName, groupItem.groupName);
		m_settings->setValue(kszRemovable, groupItem.removable);
		++index;
	}
	m_settings->endArray();
}

int FavoriteEmotionSettings::findGroup(const QString &groupId) const
{
	int index = 0;
	foreach (FavoriteEmotionSettings::EmotionGroup groupItem, m_groups)
	{
		if (groupItem.groupId == groupId)
			return index;

		++index;
	}
	return -1;
}