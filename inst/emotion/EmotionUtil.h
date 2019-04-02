#ifndef _EMOTIONUTIL_H_
#define _EMOTIONUTIL_H_

#include <QStringList>
#include <QPoint>
#include <QScopedPointer>
#include "emotionconsts.h"

class QObject;
class QWidget;
class EmotionCallback
{
public:
	virtual QObject *instance() = 0;
	virtual QWidget *instanceWindow() = 0;
	virtual void onEmotionSelected(bool defaultEmotion, const QString &emotionId) = 0;
	virtual void emotionClosed() = 0;
};

class EmotionDialog;
class FavoriteEmotionSettings;
class EmotionUtil
{
public:
	static EmotionUtil &instance();

	void init();
	void uninit();

	QStringList emotionNames() const { return m_faceNames; }
	QStringList emotionCodeNames() const { return m_faceCodeNames; }
	QStringList emotionFilePathes() const { return m_faceFileNames; }

	void showEmotion(EmotionCallback *cb, const QPoint &pos);
	bool isEmotionShown() const;
	void closeEmotion();

	QString favoriteEmotionFilePath(const QString &emotionId) const;
	QString favoriteEmotionName(const QString &emotionId) const;
	bool setFavoriteEmotionName(const QString &emotionId, const QString &emotionName);
	bool addFavoriteEmotion(const QString &filePath, QWidget *parent);
	bool delFavoriteEmotion(const QString &emotionId, const QString &groupId);
	bool moveFirstFavoriteEmotion(const QString &emotionId, const QString &groupId);
	bool moveFavoriteEmotionToOtherGroup(const QString &emotionId, const QString &fromGroupId, const QString &toGroupId);
	bool moveFavoriteEmotionGroup(const QString &fromGroupId, const QString &toGroupId);

	void getFavoriteGroups(QStringList &groupIds, QStringList &groupNames) const;
	bool addFavoriteGroup(const QString &groupName);
	bool delFavoriteGroup(const QString &groupId);
	QString favoriteGroupName(const QString &groupId) const;
	bool setFavoriteGroupName(const QString &groupId, const QString &groupName);
	bool moveFavoriteGroupBefore(const QString &groupId);
	bool moveFavoriteGroupAfter(const QString &groupId);
	bool moveFavoriteGroupToTop(const QString &groupId);
	bool hasFavoriteEmotionInGroup(const QString &groupId);

	bool hasFavoriteEmotion() const;

private:
	EmotionUtil();
	void loadDefaultEmotions();
	void loadFavoriteEmotions();
	void loadFavoriteEmotions(const QString &groupId);

private:
	QStringList                             m_faceNames;
	QStringList								m_faceCodeNames;
	QStringList                             m_faceFileNames;
	QScopedPointer<EmotionDialog>           m_pPopup;
	QScopedPointer<FavoriteEmotionSettings> m_pFavoriteEmotionSettings;
	bool                                    m_favoriteLoaded;
};

#endif //_EMOTIONUTIL_H_
