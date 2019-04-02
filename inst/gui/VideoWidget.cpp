#include <QMutex>
#include <QEvent>
#include <QDebug>
#include <QTimer>


#include <windows.h>
#include <ddraw.h>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#include "VideoWidget.h"

#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      32    // bits per pixel

#define SAFE_DELETE(p)  { if (p) { delete (p);     (p) = NULL; } }
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }
#define GETWNDRECT(rect)                            \
    GetWindowRect(m_hWnd, rect);

class VideoWidgetPrivate
{
    Q_DECLARE_PUBLIC(VideoWidget)
public:
    explicit VideoWidgetPrivate(VideoWidget *p);
    virtual ~VideoWidgetPrivate();

    void recreateDeviceResource();
    void destroyDeviceResource();
    void createDeviceResource();

    void drawFrame(const QImage &img);

    void drawBackground();

    void update_screen();

    int initDirectDraw();
    int createPrimarySurface();
    int createYUVDispBuff();
    int createRGBDispBuff();
    int createWndClipper();
    bool restoreAllSurfaces();

public:
    typedef enum
    {
        FORMAT_MODE_NONE    = 1, 
        FORMAT_MODE_RGB555  = 2, 
        FORMAT_MODE_RGB565  = 3, 
        FORMAT_MODE_RGB24   = 4, 
        FORMAT_MODE_RGB32   = 5, 
        FORMAT_MODE_YUYV    = 6, 
        FORMAT_MODE_UYVY    = 7
    }DISP_FORMAT;

    LPDIRECTDRAW7           m_pDD;
    LPDIRECTDRAWSURFACE7    m_pddsFrontBuffer;
    LPDIRECTDRAWSURFACE7    m_pddsDispBuffer;

    DISP_FORMAT m_DispBuffFormat;
    QSize m_imageSize;

    QMutex      m_mutex;

    bool        m_bInit;

    VideoWidget *q_ptr;
};

VideoWidgetPrivate::VideoWidgetPrivate( VideoWidget *p )
: q_ptr(p)
, m_pDD(0)
, m_pddsDispBuffer(0)
, m_pddsFrontBuffer(0)
, m_DispBuffFormat(FORMAT_MODE_NONE)
, m_bInit(false)
{
    m_imageSize.setWidth(SCREEN_WIDTH);
    m_imageSize.setHeight(SCREEN_HEIGHT);
}

VideoWidgetPrivate::~VideoWidgetPrivate()
{
    destroyDeviceResource();
}

void VideoWidgetPrivate::recreateDeviceResource()
{
    destroyDeviceResource();
    createDeviceResource();
}

void VideoWidgetPrivate::destroyDeviceResource()
{
    SAFE_RELEASE(m_pddsDispBuffer);
    SAFE_RELEASE(m_pddsFrontBuffer);

    if (m_pDD)
        m_pDD->SetCooperativeLevel(0, DDSCL_NORMAL);
    SAFE_RELEASE(m_pDD);
    m_bInit = false;
    qDebug() << Q_FUNC_INFO;
}

void VideoWidgetPrivate::createDeviceResource()
{
    qDebug() << Q_FUNC_INFO << "begin";

    m_bInit = false;
    do 
    {
        if (initDirectDraw() != 0)
        {
            break;
        }

        if (createPrimarySurface() != 0)
        {
            break;
        }
        
        if (createRGBDispBuff() != 0)
        {
            break;
        }

        drawBackground();
        m_bInit = true;
    } while (0);

    if (!m_bInit)
    {
        destroyDeviceResource();
    }
    qDebug() << Q_FUNC_INFO << "end";
}


void VideoWidgetPrivate::update_screen()
{
    if (!m_bInit) 
        return;

    if ((m_pddsFrontBuffer == NULL) || (m_pddsDispBuffer == NULL))
    {
        return;
    }

    RECT        Rect;
    GetWindowRect((HWND)(q_func()->winId()), &Rect);

    HRESULT ddrval = m_pddsFrontBuffer->Blt(&Rect, m_pddsDispBuffer, NULL, DDBLT_DONOTWAIT, NULL);
    if (ddrval == DDERR_SURFACELOST)
    {
        restoreAllSurfaces();
    }
}

int VideoWidgetPrivate::initDirectDraw()
{
    int iRet = -1;
    do 
    {
        HRESULT hr;
        if (FAILED(hr = DirectDrawCreateEx(NULL, (VOID **)&m_pDD, 
            IID_IDirectDraw7, NULL)))
            break;

        hr = m_pDD->SetCooperativeLevel(/*q_func()->winId()*/0, DDSCL_NORMAL);
        if (FAILED(hr))
            break;
        iRet = 0;
    } while (0);

    if (iRet)
    {
        SAFE_RELEASE(m_pDD);
    }

    return iRet;
}

/***************************** Code *****************************/
int VideoWidgetPrivate::createPrimarySurface()
{
    int ret = -1;
    do 
    {
        if (m_pDD == NULL)
            break;

        DDSURFACEDESC2  ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize  = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        if (FAILED(m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)))
        {
            qDebug("%s Create front buffer failed.", Q_FUNC_INFO);
            break;
        }
        ret = 0;
    } while (0);

    if (ret)
    {
        SAFE_RELEASE(m_pddsFrontBuffer);
    }
    return ret;
}

/***************************** Code *****************************/
int VideoWidgetPrivate::createYUVDispBuff()
{
    if (m_pDD == NULL || m_pddsFrontBuffer == NULL || m_imageSize.isNull())
    {
        return -1;
    }
    int ret = -1;
    do 
    {
        HRESULT         hr;
        DDPIXELFORMAT   g_ddpfDisplayFormats = {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0};
        DDSURFACEDESC2  ddsd;

        DDCAPS         ddcap;
        bool           bBltStretchX = false;

        m_DispBuffFormat = FORMAT_MODE_YUYV;

        ZeroMemory(&ddcap, sizeof(ddcap));
        ddcap.dwSize = sizeof(ddcap);
        hr = m_pDD->GetCaps(&ddcap, NULL);
        if (FAILED(hr))
        {
            qDebug("%s get caps failed.", Q_FUNC_INFO);
            break;
        }
        if ((ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHX) 
            && (ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHY))
            bBltStretchX = true;

        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.ddpfPixelFormat = g_ddpfDisplayFormats;
        ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

        if (bBltStretchX)
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;//| DDSCAPS_SYSTEMMEMORY;//DDSCAPS_SYSTEMMEMORY;//
        else
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;//DDSCAPS_SYSTEMMEMORY;//

        ddsd.dwWidth  = m_imageSize.width();
        ddsd.dwHeight = m_imageSize.height();
        if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsDispBuffer, NULL)))
        {
            qDebug("%s Create disp buffer failed.", Q_FUNC_INFO);
            break;
        }

        if (createWndClipper())
        {
            qDebug("%s Create wnd Clipper failed.", Q_FUNC_INFO);
            break;
        }

        ret = 0;
    } while (0);

    if (ret)
    {
        SAFE_RELEASE(m_pddsDispBuffer);
    }

    return ret;
}

/***************************** Code *****************************/
int VideoWidgetPrivate::createRGBDispBuff()
{
    if (!m_pDD || !m_pddsFrontBuffer || m_imageSize.isNull())
        return -1;

    int ret = -1;
    do 
    {
        HRESULT         hr = DD_OK;
        DDSURFACEDESC2  ddsd;
        DDCAPS          ddcap;
        bool            bBltStretchX = false; 

        ZeroMemory(&ddcap, sizeof(ddcap));
        ddcap.dwSize = sizeof(ddcap);
        hr=m_pDD->GetCaps(&ddcap, NULL);
        if (FAILED(hr)) 
        {
            qDebug("%s getCaps failed.", Q_FUNC_INFO);
            break;
        }

        if ((ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHX) 
            && (ddcap.dwFXCaps & DDFXCAPS_BLTSTRETCHY))
            bBltStretchX = true;

        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;//|DDSD_PIXELFORMAT;
        if (bBltStretchX)    
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;//| DDSCAPS_SYSTEMMEMORY;//DDSCAPS_SYSTEMMEMORY;//
        else
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;//DDSCAPS_SYSTEMMEMORY;//
        //ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;

        ddsd.dwWidth   = m_imageSize.width();
        ddsd.dwHeight  = m_imageSize.height();
        if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsDispBuffer, NULL)))
        {
            qDebug("%s create disp buffer surface failed.", Q_FUNC_INFO);
            break;
        }

        m_DispBuffFormat = FORMAT_MODE_NONE;
        ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
        ddsd.dwSize     = sizeof(DDSURFACEDESC2);
        hr = m_pddsDispBuffer->Lock(NULL, &ddsd, 
            DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 
            NULL);
        if (FAILED(hr))
        {
            qDebug("%s disp buffer lock failed.", Q_FUNC_INFO);
            break;
        }

        m_DispBuffFormat = FORMAT_MODE_NONE;
        if ((ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB))
        {
            if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
            {
                m_DispBuffFormat = FORMAT_MODE_RGB32;
            }
            else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 24)
            {
                m_DispBuffFormat = FORMAT_MODE_RGB24;
            }
            else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
            {
                if (ddsd.ddpfPixelFormat.dwRBitMask == 0xf800)
                {
                    m_DispBuffFormat = FORMAT_MODE_RGB565;
                }
                else if (ddsd.ddpfPixelFormat.dwRBitMask == 0x7C00) // RGB555
                {
                    m_DispBuffFormat = FORMAT_MODE_RGB555;
                }
            }
        }

        m_pddsDispBuffer->Unlock(NULL);

        if (m_DispBuffFormat == FORMAT_MODE_NONE)
        {
            qDebug("%s m_DispBuffFormat is none", Q_FUNC_INFO);
            break;
        }

        if (createWndClipper() != 0)
        {
            qDebug("%s Create wnd Clipper failed.", Q_FUNC_INFO);
            break;
        }
        ret = 0;
    } while (0);

    if (ret)
    {
        SAFE_RELEASE(m_pddsDispBuffer);
    }

    return ret;
}

/***************************** Code *****************************/
int VideoWidgetPrivate::createWndClipper()
{
    if ((m_pddsFrontBuffer == NULL) || (m_pDD == NULL))
        return -1;

    HRESULT             hr = DD_OK;
    LPDIRECTDRAWCLIPPER pcClipper = NULL;


    hr = m_pddsFrontBuffer->GetClipper(&pcClipper);
    if (hr == DD_OK && pcClipper)
    {
        hr = pcClipper->SetHWnd(0, (HWND)(q_func()->winId()));
    }
    else
    {
        hr = m_pDD->CreateClipper(0, &pcClipper, NULL);
        if (hr != DD_OK) 
            return -1;

        hr = pcClipper->SetHWnd(0, (HWND)(q_func()->winId()));
        if (hr != DD_OK) 
        {
            pcClipper->Release(); 
            return -1;
        }

        hr = m_pddsFrontBuffer->SetClipper(pcClipper);
        if (hr != DD_OK) 
        {
            pcClipper->Release(); 
            return -1;
        }
        pcClipper->Release();
    }
    return hr == DD_OK ? 0 : -1;
}

/***************************** Code *****************************/
bool VideoWidgetPrivate::restoreAllSurfaces()
{
    qDebug("%s begin", Q_FUNC_INFO);
    bool bRet = false;
    do 
    {
        if (m_pddsFrontBuffer == NULL)
        {
            if (createPrimarySurface() != 0)
            {
                m_pddsFrontBuffer = NULL;
                break;
            }
        }

        if (m_pddsDispBuffer == NULL)
        {
            if (createRGBDispBuff() != 0)
            {
                m_pddsDispBuffer = NULL;
                break;
            }
        }

        HRESULT ddrval;
        ddrval = m_pddsFrontBuffer->Restore();
        if (FAILED(ddrval))
        {
            SAFE_RELEASE(m_pddsFrontBuffer);
            break;
        }
        ddrval = m_pddsDispBuffer->Restore();
        if (FAILED(ddrval)) 
        {
            SAFE_RELEASE(m_pddsDispBuffer);
            break;
        }

        bRet = true;
    } while (0);
    qDebug("%s end bRet[%s]", Q_FUNC_INFO, (bRet?"true":"false"));

    return bRet;
}

void VideoWidgetPrivate::drawFrame(const QImage &img)
{
    DDSURFACEDESC2  ddsd;
    HRESULT         ddrval;
    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    ddrval = m_pddsDispBuffer->Lock(NULL, &ddsd, 
        DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 
        NULL);
    if (FAILED(ddrval))
    {
        if (ddrval == DDERR_SURFACELOST)
        {
            restoreAllSurfaces();
        }
        return;
    }

    QImage image;
    switch (m_DispBuffFormat)
    {
    case VideoWidgetPrivate::FORMAT_MODE_YUYV:

        break;
    case VideoWidgetPrivate::FORMAT_MODE_RGB32:
        {
            image = img.convertToFormat(QImage::Format_ARGB32);
        }
        break;
    case VideoWidgetPrivate::FORMAT_MODE_RGB24:
        {
            image = image.convertToFormat(QImage::Format_RGB888);
        }
        break;
    case VideoWidgetPrivate::FORMAT_MODE_RGB555:
        {
            image = image.convertToFormat(QImage::Format_RGB555);
        }
        break;
    case VideoWidgetPrivate::FORMAT_MODE_RGB565:

        break;
    default:
        break;
    }

    if (!image.isNull())
    {
        // qDebug() << Q_FUNC_INFO << q_func()->objectName() << ddsd.dwWidth << ddsd.dwHeight << ddsd.lPitch;
        if (ddsd.lPitch == image.bytesPerLine())
        {
            memcpy(ddsd.lpSurface, image.constBits(), image.byteCount());
        }
        else
        {
            uchar *pSurface = (uchar*)ddsd.lpSurface;
            const uchar *pImg = image.constBits();
            int bytesPerLine = image.bytesPerLine();

            for (int i = 0; i < image.height(); ++i)
            {
                memcpy((pSurface+i*ddsd.lPitch), (pImg+i*bytesPerLine), bytesPerLine);
            }
        }
    }
    else
    {
        qDebug() << Q_FUNC_INFO << q_func()->objectName() << " image is null";
    }
    m_pddsDispBuffer->Unlock(NULL);
}

void VideoWidgetPrivate::drawBackground()
{
    DDSURFACEDESC2  ddsd;
    HRESULT         ddrval;
    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    ddrval = m_pddsDispBuffer->Lock(NULL, &ddsd, 
        DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 
        NULL);
    if (FAILED(ddrval))
    {
        if (ddrval == DDERR_SURFACELOST)
        {
            restoreAllSurfaces();
        }
        return;
    }

    switch (m_DispBuffFormat)
    {
    case VideoWidgetPrivate::FORMAT_MODE_YUYV:

        break;
    case VideoWidgetPrivate::FORMAT_MODE_RGB32:
    case VideoWidgetPrivate::FORMAT_MODE_RGB24:
    case VideoWidgetPrivate::FORMAT_MODE_RGB555:
    case VideoWidgetPrivate::FORMAT_MODE_RGB565:
        memset(ddsd.lpSurface, 0, ddsd.dwHeight*ddsd.lPitch);
        break;
    default:
        break;
    }

    m_pddsDispBuffer->Unlock(NULL);
}

//////////////////////////////////////////////////////////////////////////
VideoWidget::VideoWidget( QWidget *parent /*= 0*/ )
: QWidget(parent)
, d_ptr(new VideoWidgetPrivate(this))
{
    setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_PaintOnScreen);
}

VideoWidget::~VideoWidget()
{
}

QPaintEngine* VideoWidget::paintEngine() const
{
    return 0;
}

void VideoWidget::onUpdated( const QImage &frame )
{
    Q_D(VideoWidget);

    // qDebug() << Q_FUNC_INFO << this->objectName() << frame.size();

    if (!d->m_bInit)
        return;

    if ((d->m_pddsFrontBuffer == NULL) || (d->m_pddsDispBuffer == NULL))
    {
        return;
    }

    d->drawBackground();

    QImage img = frame;
    if (img.isNull())
    {
        return;
    }

    if (img.size() != d->m_imageSize)
    {
        img = img.scaled(d->m_imageSize, Qt::KeepAspectRatio);
    }

    d->drawFrame(img);
}

void VideoWidget::onSizeChanged( const QSize &s )
{
    Q_D(VideoWidget);
    qDebug() << Q_FUNC_INFO << this->objectName() << s;
    if (s.isValid() && d->m_imageSize != s)
    {
        d->m_imageSize = s;
        d->recreateDeviceResource();
    }
}

void VideoWidget::paintEvent( QPaintEvent *e )
{
    Q_UNUSED(e);
    Q_D(VideoWidget);

    if (!d->m_bInit) {
        d->recreateDeviceResource();
        if (!d->m_pDD)
        {
            qWarning("No render target!!!");
            return;
        }
    }

    d->update_screen();
}

void VideoWidget::showEvent( QShowEvent *e )
{
    Q_UNUSED(e);
    qDebug() << Q_FUNC_INFO << this->objectName();
    Q_D(VideoWidget);
    d->recreateDeviceResource();
}

void VideoWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
	QWidget::mouseDoubleClickEvent(e);

	emit videoDoubleClicked();
}

bool VideoWidget::event( QEvent *e )
{
    if (e->type() == QEvent::WinIdChange)
    {
        qDebug() << Q_FUNC_INFO << this->objectName() << "WinIdChange";
    }

    return QWidget::event(e);
}

QSize VideoWidget::imageSize() const
{
    return d_func()->m_imageSize;
}




