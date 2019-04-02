#include <QCamera>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QReadLocker>
#include <QWriteLocker>
#include "VideoCapture.h"
#include "PmApp.h"
#include "logger/logger.h"

//////////////////////////////////////////////////////////////////////////
// class CameraSurface
class CameraSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit CameraSurface(QObject *parent = 0);
    virtual ~CameraSurface();

signals:
    void frameReady(const QImage &f);
    void frameSizeChanged(const QSize &s);

private:
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    virtual bool present(const QVideoFrame &frame);
    virtual bool start(const QVideoSurfaceFormat &format);
    virtual void stop();

private:
    QImage::Format         imageFormat;
    QImage                 curImage;
};

CameraSurface::CameraSurface( QObject *parent /*= 0*/ )
: QAbstractVideoSurface(parent)
{
}

CameraSurface::~CameraSurface()
{
}

QList<QVideoFrame::PixelFormat> CameraSurface::supportedPixelFormats( QAbstractVideoBuffer::HandleType handleType /*= QAbstractVideoBuffer::NoHandle*/ ) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool CameraSurface::present( const QVideoFrame &frame )
{
	// tested result: this present method is called in main thread, and about 60ms to call once

    QVideoFrame myFrame = frame;
    myFrame.map(QAbstractVideoBuffer::ReadOnly);
	memcpy(curImage.bits(), frame.bits(), curImage.byteCount());
    myFrame.unmap();

	curImage = curImage.mirrored();
    emit frameReady(curImage);

    return true;
}

bool CameraSurface::start( const QVideoSurfaceFormat &format )
{
    qDebug() << Q_FUNC_INFO << format;
    if (isActive()) 
	{
        stop();
    }
	
	if (!format.frameSize().isEmpty()) 
	{
        imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
		curImage = QImage(format.frameSize(), imageFormat);
		curImage.fill(Qt::black);

        emit frameSizeChanged(format.frameSize());

		emit frameReady(curImage);

        return QAbstractVideoSurface::start(format);
    }
    return false;
}

void CameraSurface::stop()
{
	curImage.fill(Qt::black);
    QAbstractVideoSurface::stop();
}

namespace session 
{
//////////////////////////////////////////////////////////////////////////
// class VideoCapture
    VideoCapture::VideoCapture( QObject *parent /*= 0*/ )
        : QObject(parent)
        , m_bInitCamera(false)
        , m_bStartCapture(false)
		, m_CameraError(Error_NoError)
		, m_lock()
		, m_curFrame()
    {
        m_pCameraSurface.reset(new CameraSurface);
        connect(m_pCameraSurface.data(), SIGNAL(frameReady(QImage)), this, SLOT(onFrameReady(QImage)));
        connect(m_pCameraSurface.data(), SIGNAL(frameSizeChanged(QSize)), this, SLOT(onFrameSizeChanged(QSize)));
    }

    VideoCapture::~VideoCapture()
    {
        stopCapture();
    }

    void VideoCapture::startCapture()
    {
		qPmApp->getLogger()->debug("VideoCapture::startCapture begin");

        initCamera();
		if (m_pCamera)
		{
			m_bStartCapture = true;
			m_pCamera->start();
		}

		qPmApp->getLogger()->debug("VideoCapture::startCapture end");
    }

    void VideoCapture::stopCapture()
    {
		qPmApp->getLogger()->debug("VideoCapture::stopCapture begin");

        if (m_pCamera)
        {
            m_pCamera->stop();
			m_pCamera->unload();
			m_bStartCapture = false;
        }
        releaseCamera();

		qPmApp->getLogger()->debug("VideoCapture::stopCapture end");
    }

    void VideoCapture::appendVideoFrameCB( const QString &sid, IVideoFrameCallback *cb )
    {
        if (cb)
        {
            if (m_mapCB.contains(sid))
            {
                if (m_mapCB[sid] && m_mapCB[sid] != cb)
                {
                    // error
					qWarning() << Q_FUNC_INFO << "sid with a different cb.";
                }
            }
            else
            {
                m_mapCB[sid] = cb;
                if (!m_frameSize.isNull())
                {
                    cb->setImageSize(m_frameSize);
                }
                connect(cb->instance(), SIGNAL(destroyed(QObject*)), this, SLOT(onCBDestoryed(QObject*)));
            }
        }
    }

    void VideoCapture::removeVideoFrameCB( const QString &sid )
    {
        if (m_mapCB.contains(sid))
        {
            m_mapCB.remove(sid);
        }
    }

    void VideoCapture::getframe(QImage &frame) const
    {
		{
			QReadLocker locker(&m_lock);
			frame = m_curFrame;
		}
    }

	VideoCapture::CameraError VideoCapture::errorCode() const
	{
		return m_CameraError;
	}

    void VideoCapture::onFrameReady( const QImage &f )
    {
		{ // update current frame
			QWriteLocker locker(&m_lock);
			m_curFrame = f;
		}

		if (f.isNull())
		{
			qDebug() << Q_FUNC_INFO << "--------------   get null image !!!";
		}
		
		// update self frame
        foreach (IVideoFrameCallback *cb, m_mapCB.values())
        {
            cb->frameChanged(f);
        }
    }

    void VideoCapture::onFrameSizeChanged( const QSize &s )
    {
        if (m_frameSize == s)
		{
            return;
		}

        m_frameSize = s;

        foreach (IVideoFrameCallback *pcb, m_mapCB.values())
        {
            if (pcb)
            {
                pcb->setImageSize(s);
            }
        }
    }

    void VideoCapture::onCBDestoryed( QObject *obj )
    {
        foreach (const QString sid, m_mapCB.keys())
        {
            IVideoFrameCallback *cb = m_mapCB.value(sid);
            if (cb && cb->instance() == obj)
            {
                m_mapCB.remove(sid);
            }
        }
    }

	void VideoCapture::onCameraStateChanged( QCamera::State state )
	{
		qDebug() << Q_FUNC_INFO << state;
		qPmApp->getLogger()->debug(QString("VideoCapture::onCameraStateChanged state: %1").arg((int)state));

		if (m_bStartCapture 
			&& QCamera::LoadedState == m_CameraState 
			&& QCamera::LoadedState == state)
        {
            // error
            qDebug() << Q_FUNC_INFO << "camera is locked.";

			qPmApp->getLogger()->debug(QString("VideoCapture::onCameraStateChanged: camera is opened"));

			m_CameraError = Error_CameraOpened;
        }
		m_CameraState = state;
    }

	void VideoCapture::onCameraStatusChanged( QCamera::Status status )
	{
		qDebug() << Q_FUNC_INFO << status;

		qPmApp->getLogger()->debug(QString("VideoCapture::onCameraStatusChanged status: %1").arg((int)status));
	}

	void VideoCapture::onCameraError( QCamera::Error errcode )
	{
        qDebug() << Q_FUNC_INFO << errcode;

		qPmApp->getLogger()->debug(QString("VideoCapture::onCameraError error: %1 %2").arg((int)errcode).arg(m_pCamera->errorString()));
    }

    void VideoCapture::initCamera()
    {
		qPmApp->getLogger()->debug("VideoCapture::initCamera begin");

		if (m_bInitCamera)
			return;

		QList<QByteArray> availableDevices = QCamera::availableDevices();
		qDebug() << Q_FUNC_INFO << availableDevices;

        if (m_CameraDevice.isEmpty() || !availableDevices.contains(m_CameraDevice))
        {
            if (availableDevices.isEmpty())
            {
                // error 
				qDebug() << Q_FUNC_INFO << "no camera";
				m_CameraError = Error_NoCamera;
				m_CameraDevice.clear();

				m_pCamera.reset(new QCamera());
			}
			else
			{
				m_CameraDevice = availableDevices[0];
				m_pCamera.reset(new QCamera(m_CameraDevice));
			}
        }
        else
        {
            m_pCamera.reset(new QCamera(m_CameraDevice));
        }

		disconnect(m_pCamera.data(), SIGNAL(stateChanged(QCamera::State)), this, SLOT(onCameraStateChanged(QCamera::State)));
		disconnect(m_pCamera.data(), SIGNAL(statusChanged(QCamera::Status)), this, SLOT(onCameraStatusChanged(QCamera::Status)));
		disconnect(m_pCamera.data(), SIGNAL(error(QCamera::Error)), this, SLOT(onCameraError(QCamera::Error)));
		connect(m_pCamera.data(), SIGNAL(stateChanged(QCamera::State)), this, SLOT(onCameraStateChanged(QCamera::State)));
		connect(m_pCamera.data(), SIGNAL(statusChanged(QCamera::Status)), this, SLOT(onCameraStatusChanged(QCamera::Status)));
		connect(m_pCamera.data(), SIGNAL(error(QCamera::Error)), this, SLOT(onCameraError(QCamera::Error)));

		m_CameraState = QCamera::UnloadedState;
        m_pCamera->setViewfinder(m_pCameraSurface.data());

        m_bInitCamera = true;

		qPmApp->getLogger()->debug("VideoCapture::initCamera end");
    }

    void VideoCapture::releaseCamera()
    {
		qPmApp->getLogger()->debug("VideoCapture::releaseCamera begin");

        m_pCamera.reset();
        m_bInitCamera = false;
		m_CameraError = Error_NoError;
		m_CameraState = QCamera::UnloadedState;

		qPmApp->getLogger()->debug("VideoCapture::releaseCamera end");
    }

}

#include "VideoCapture.moc"
