#ifndef CHATINPUTDEF_H
#define CHATINPUTDEF_H
#include <QTextFormat>

namespace PM {
	enum TextFormat
	{
		FileTextFormat = QTextFormat::UserObject + 1,
		FileName,
		FileIcon,
		FilePath,
		ImageUuid,

		AtTextFormat,
		AtText,
		AtId
	};
};

#endif