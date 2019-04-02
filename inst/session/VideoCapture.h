#ifndef _VIDEOCAPTURE_H_
#define _VIDEOCAPTURE_H_

#include <QObject>
#include <QImage>
#include <QSize>
#include <QMap>
#include <QCamera>
#include <QReadWriteLock>
#include "VideoFrameCallback.h"

//class QCamera;
class CameraSurface;

namespace session
{
    class IVideoFrameCallback;
    class VideoCapture : public QObject
    {
        Q_OBJECT
	public:
		enum CameraError {
			Error_NoError,
			Error_NoCamera,
			Error_CameraOpened
		};

    public:
        explicit VideoCapture(QObject *parent = 0);
        ~VideoCapture();

    public:
        void setDevice(const QByteArray &ba);

        void startCapture();
        void stopCapture();

        void appendVideoFrameCB(const QString &sid, IVideoFrameCallback *cb);
        void removeVideoFrameCB(const QString &sid);

        void getframe(QImage &frame) const;

		CameraError errorCode() const;

    private slots:
        void onFrameReady(const QImage &f);
        void onFrameSizeChanged(const QSize &s);
        void onCBDestoryed(QObject *obj);
		void onCameraStateChanged(QCamera::State state);
		void onCameraStatusChanged(QCamera::Status status);
		void onCameraError(QCamera::Error errcode);

    private:
        void initCamera();
        void releaseCamera();

    private:
        QScopedPointer<QCamera>             m_pCamera;
        QScopedPointer<CameraSurface>       m_pCameraSurface;

        QMap<QString, IVideoFrameCallback*> m_mapCB;

        bool           m_bInitCamera;
        bool           m_bStartCapture;
        QByteArray     m_CameraDevice;
		CameraError    m_CameraError;
		QCamera::State m_CameraState;

        QSize                  m_frameSize;
		QImage                 m_curFrame;
		mutable QReadWriteLock m_lock;

		friend class CameraSurface;
    };
}

#endif //_VIDEOCAPTURE_H_
