#ifndef DIALOGAVATAREDITOR_H
#define DIALOGAVATAREDITOR_H

#include "framelessdialog.h"

namespace Ui {
class DialogAvatarEditor;
}
class QNetworkAccessManager;
class QNetworkReply;

class DialogAvatarEditor : public FramelessDialog
{
    Q_OBJECT
    
public:
    explicit DialogAvatarEditor(const QString &uid, QWidget *parent = 0);
    ~DialogAvatarEditor();
    void setOrignalAvatar(const QPixmap& pixmap);
    QPixmap getAvatar();
	bool isModified() const {return m_bModified;}

public slots:
    void browse();
	void photo();
	void edit();
    void zoomIn();
    void zoomOut();
    void selectedPixmapChanged(const QPixmap& pixmap);
	virtual void setSkin();

private slots:
	void onPushButtonOKClicked();
	void onUpdateFinished(QNetworkReply *reply);
	void viewBigAvatar();

private:
	void setPixmap(const QPixmap& pixmap);
    
private:
    Ui::DialogAvatarEditor *ui;
	QString                 m_uid;
    QPixmap                 m_avatar;
	bool                    m_bModified;
	QNetworkAccessManager  *m_networkAccessManager;
};

#endif // DIALOGAVATAREDITOR_H
