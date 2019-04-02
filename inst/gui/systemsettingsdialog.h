#ifndef SYSTEMSETTINGSDIALOG_H
#define SYSTEMSETTINGSDIALOG_H

#include "framelessdialog.h"
#include <QModelIndex>
namespace Ui {class SystemSettingsDialog;};
class QStandardItemModel;

class SystemSettingsDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum SettingIndex
	{
		IndexNormal = 0,
		IndexChatWindow,
		IndexFile,
		IndexShortcut,
		IndexSound
	};

public:
	static SystemSettingsDialog *getInstance();
	SystemSettingsDialog(QWidget *parent = 0);
	~SystemSettingsDialog();

	void setSettingIndex(SettingIndex index);
	void setShotKeyConflict(bool conflict);
	void setTakeMsgKeyConflict(bool conflict);

public slots:
	void setSkin();

signals:
	void mainPanelTopmost(bool topmost);
	void shortcutKeyApplied();

private slots:
	void onPageChanged(const QModelIndex &index);
	void saveSettings();
	void applySettings();
	void openFileSaveDir();
	void changeFileSaveDir();
	void onShotKeyChanged(const QString &text);
	void onTakeMsgKeyChanged(const QString &text);
	void onPushButtonRecoverChatCloseClicked();
	void onMuteStateChanged(int state);
	void onBuddyMuteStateChanged(int state);
	void onSubscriptionMuteStateChanged(int state);
	void onBuddyMsgAuditionClicked();
	void onBuddyMsgChangerClicked();
	void onSubscriptionMsgAudtionClicked();
	void onSubscriptionMsgChangerClicked();

private:
	void initUI();
	void initSignals();

	void loadNormalSettings();
	void loadChatWindowSettings();
	void loadFileSettings();
	void loadShortcutSettings();
	void loadSoundSettings();

	void applyNormalSettings();
	void applyChatWindowSettings();
	void applyFileSettings();
	void applyShortcutSettings();
	void applySoundSettings();

	QString shotKey() const;
	QString takeMsgKey() const;

	void audition(const QString &file) const;

private:
	Ui::SystemSettingsDialog *ui;
	static SystemSettingsDialog *s_instance;
};

#endif // SYSTEMSETTINGSDIALOG_H
