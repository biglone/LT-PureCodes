#ifndef CHATINPUTBOX_H
#define CHATINPUTBOX_H

#include <QTextEdit>
#include <QFont>
#include <QColor>

class QMovie;
class QStandardItemModel;
class QCompleter;
class QModelIndex;

class CChatInputBox : public QTextEdit
{
	Q_OBJECT

public:
	explicit CChatInputBox(QWidget *parent = 0);
	~CChatInputBox();

	void setFaceSource(const QStringList& strlstName, const QStringList& strlstSrc);

	void setCompleteStrings(const QStringList &completeStrings);

	void insertFiles(const QStringList& rsFiles);
	void insertFile(const QString& rsFile);
	void insertDir(const QString &rsDir);

	void insertImage(const QString &rsFile, bool addImageElement = true);
	/*rsFile为[in,out]，rsFile可以为空，如果为空，则返回新的rsFile*/
	void insertImage(QString &rsFile, const QImage &image, bool addImageElement = true);

	void setTextForeground(const QBrush& brush);
	QBrush textForeground() const;

	void setTextBackgroud(const QBrush& brush);
	QBrush textBackground() const;

	QString msgText() const;
	QStringList msgPieces() const;
	QList<QString> atIds() const;
	QList<QString> msgFiles() const;
	QMap<QString, QPair<QString, QSize>> msgImages() const;

	void clearResource();

	static QMimeData *msgToCopyMimeData(const QString &title, const QString &msgText, 
		const QString &imagePath, const QString &attachPath, const QString &dirPath);
	static bool copyMimeDataToMsg(const QMimeData *mimeData, QString &title, QString &msgText, 
		QString &imagePath, QString &attachPath, QString &dirPath);

public slots:
	void insertFace(int idx, bool addImgElement = true);
	void updateFaceFrame(int frame);
	void updateGifFrame(int frame);
	void clear();
	void textCharFormatChanged(const QString &fontFamily, int fontSize, bool bBold, bool bItalic, bool bUnderline, const QColor &color);
	void onTextChanged();
	void addAtText(const QString &atText);
	void insertMimeData(const QMimeData *source);
	
signals:
	void sendMessage();
	void showInfoTip(const QString &tip);

protected:
	bool canInsertFromMimeData( const QMimeData *source ) const;
	void insertFromMimeData( const QMimeData *source );
	void keyPressEvent(QKeyEvent *e);
	void focusInEvent(QFocusEvent *e);
	void contextMenuEvent(QContextMenuEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *e);

private:
	bool canInsertFile(const QString& rsFile);
	void initCompleter();
	void performCompletion(const QString &completionPrefix);
	void performCompletion();
	static bool caseInsensitiveLessThan(const QString &a, const QString &b);
	QImage generateImageResource(const QImage &image);
	void insertBasicMimeData(const QMimeData *source);

private slots:
	void copyImage();
	void saveImage();
	void insertCompletion(const QModelIndex &completionIndex);

private:
	QStringList m_faceNames;
	QStringList m_faceFileNames;
	QMap<int, QMovie*> m_facesMap;
	
	QMap<QString, QPair<QString, QSize>> m_imageInfos; // image url <==> (image uuid, image size)

	QAction *m_actionCopyImage;
	QAction *m_actionSaveImage;

	QStandardItemModel *m_completerModel;
	QCompleter         *m_completer;
	QString             m_completionPrefix;

	QMap<QString, QMovie *> m_gifMap;
};

#endif // CHATINPUTBOX_H
