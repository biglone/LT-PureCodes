#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QObject>
#include <QScopedPointer>

class QPixmap;
class ScreenShotDlg;

class SnapShot : public QObject
{
    Q_OBJECT

public:
    explicit SnapShot(QObject *parent = 0);
    ~SnapShot();

public slots:
    void shot();
	void onSnapshotOK(const QPixmap &pixmap);
	void onSnapshotCancelled();
    
signals:
    void snapShotted(const QString &imagePath);
	void snapShotCancelled();

private:
	bool                          m_snapshoting;
	QScopedPointer<ScreenShotDlg> m_screenShotDlg;
};

#endif // SNAPSHOT_H
