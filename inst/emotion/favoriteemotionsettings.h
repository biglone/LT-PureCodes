#ifndef FAVORITEEMOTIONSETTINGS_H
#define FAVORITEEMOTIONSETTINGS_H

#include <QObject>
#include <QList>
#include <QMap>
#include "emotionconsts.h"

class QSettings;

class FavoriteEmotionSettings : public QObject
{
	Q_OBJECT

public:
	class EmotionItem
	{
	public:
		EmotionItem() {}
		bool isValid() const {
			return !emotionId.isEmpty() && !fileName.isEmpty() && !md5Hex.isEmpty();
		}
		QString emotionId;
		QString fileName;
		QString shortcut;
		QString md5Hex;
	};

	class EmotionGroup
	{
	public:
		EmotionGroup() : removable(true) {}
		QString groupId;
		QString groupName;
		bool    removable;
	};

public:
	FavoriteEmotionSettings(const QString &fileName, QObject *parent = 0);
	~FavoriteEmotionSettings();

	// add the emotion to settings, return emotion id
	QString addFavoriteEmotion(const QString &fileName, const QString &shortcut, const QString &md5Hex, const QString &groupId);

	// rename favorite emotion
	bool renameFavoriteEmotion(const QString &emotionId, const QString &shortcut);

	// delete the emotion according to its id
	bool delFavoriteEmotion(const QString &emotionId, const QString &groupId);

	// move the emotion to first
	bool moveFirstFavoriteEmotion(const QString &emotionId, const QString &groupId);

	// get the emotion according to its id
	EmotionItem emotion(const QString &emotionId) const;

	// all emotions
	QList<FavoriteEmotionSettings::EmotionItem> allEmotions(const QString &groupId) const;

	// all groups
	QList<FavoriteEmotionSettings::EmotionGroup> allGroups() const;

	// group name
	QString groupName(const QString &groupId) const;

	// set group name, the group id may change, returned by newGroupId
	bool setGroupName(const QString &groupId, const QString &name, QString &newGroupId);

	// add group
	QString addGroup(const QString &groupName);

	// delete group
	bool delGroup(const QString &groupId);

	// move emotions
	bool moveFavoriteEmotionGroup(const QString &fromGroupId, const QString &toGroupId);

	// move emotion to another group
	bool moveFavoriteEmotionNewGroup(const QString &fromGroupId, const QString &emotionId, const QString &toGroupId);

	// check if emotion duplicate
	bool hasDuplicateEmotion(const QString &md5Hex) const;

	// check if has any emotion
	bool hasEmotion() const;

	// move group before one slot
	bool moveGroupBefore(const QString &groupId);

	// move group to top
	bool moveGroupToTop(const QString &groupId);

	// move group after one slot
	bool moveGroupAfter(const QString &groupId);

private:
	// set emotions
	void setEmotions(const QList<FavoriteEmotionSettings::EmotionItem> &emotions, const QString &groupId);

	// find emotion with id, return the index
	int findEmotion(const QString &emotionId, const QString &groupId) const;

	// set groups
	void setGroups(const QList<FavoriteEmotionSettings::EmotionGroup> &groups);

	// find group with id, return the index
	int findGroup(const QString &groupId) const;

private:
	QSettings                                                   *m_settings;
	QMap<QString, QList<FavoriteEmotionSettings::EmotionItem>>   m_emotions;
	QList<FavoriteEmotionSettings::EmotionGroup>                 m_groups;
};

#endif // FAVORITEEMOTIONSETTINGS_H
