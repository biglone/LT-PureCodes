#include <QtCore>
#include "simplecryptdevice.h"
#include "cryptsettings.h"

const int blockSize = 256;
quint64 CryptSettings::m_key = 0x0c2ad4a4acb9f023;
QSettings::Format CryptSettings::m_format = QSettings::IniFormat;

void CryptSettings::InitCryptSettings()
{
    static bool init = false;

    if (!init)
    {
        // To-do ...
        m_format = QSettings::registerFormat("conf", IniReadFunc, IniWriteFunc);

        init = true;
    }
}

QSettings::Format CryptSettings::format()
{
#ifdef NDEBUG
	if (m_format == QSettings::IniFormat)
	{
		InitCryptSettings();
	}
#endif
	
    return m_format;
}

void CryptSettings::setCryptProps(SimpleCryptDevice* device)
{
    device->setBlockSize(blockSize);
    device->setKey(m_key);
    device->setCompressionMode(SimpleCrypt::CompressionAuto);
    device->setIntegrityProtectionMode(SimpleCrypt::ProtectionHash);
}

bool CryptSettings::IniReadFunc(QIODevice &device, QSettings::SettingsMap &settingsMap)
{
    bool ok = true;
    QIODevice::OpenMode mode = device.openMode();
    {
        SimpleCryptDevice deviceFilter(&device);
        setCryptProps(&deviceFilter);
        deviceFilter.open(QIODevice::ReadOnly);

        QString currentSection;
        QTextStream stream(&deviceFilter);
        stream.setCodec("UTF-8");
        QString data;
        while (!stream.atEnd()) {
            data = stream.readLine();
            if (data.trimmed().isEmpty()) {
                continue;
            }
            if (data[0] == QChar('[')) {
                QString iniSection;
                int inx = data.indexOf(QChar(']'));
                if (inx == -1){
                    ok = false;
                    iniSection = data.mid(1);
                } else {
                    iniSection = data.mid(1, inx - 1);
                }

                iniSection = iniSection.trimmed();
                if (iniSection.compare(QString("general"), Qt::CaseInsensitive) == 0) {
                    currentSection.clear();
                } else {
                    if (iniSection.compare(QString("%general"), Qt::CaseInsensitive) == 0) {
                        currentSection = QString("general");
                    } else {
                        currentSection = iniSection;
                    }
                    currentSection += QChar('/');
                }
            } else {
                bool inQuotes = false;
                int equalsPos = -1;
                QList<int> commaPos;
                int i = 0;
                while (i < data.size())
                {
                    QChar ch = data.at(i);
                    if (ch == QChar('=')) {
                        if (!inQuotes && equalsPos == -1) {
                            equalsPos = i;
                        }
                    } else if (ch == QChar('"')) {
                        inQuotes = !inQuotes;
                    } else if (ch == QChar(',')) {
                        if (!inQuotes && equalsPos != -1) {
                            commaPos.append(i);
                        }
                    } else if (ch == QChar(';') || ch == QChar('#')) {
                        if (!inQuotes) {
                            data.resize(i);
                            break;
                        }
                    } else if (ch == QChar('\\')) {
                        if (++i < data.size()) {
                        } else {
                            ok = false;
                            break;
                        }
                    }
                    i++;
                }
                if (equalsPos == -1) {
                    break;
                } else {
                    QString key = data.mid(0, equalsPos).trimmed();
                    if (key.isEmpty()) {
                        break;
                    } else {
                        key = currentSection + key;
                    }
                    if (commaPos.isEmpty()) { //value
                        QString v = data.mid(equalsPos+1).trimmed();
                        if (v.startsWith("\"") && v.endsWith("\"") && v.length()>1) {
                            v = v.mid(1, v.length()-2);                     }
                        //                    qWarning() << Q_FUNC_INFO << key;
                        settingsMap[key] = stringToVariant(unescapedString(v));
                    } else { //value list
                        commaPos.prepend(equalsPos);
                        commaPos.append(-1);
                        QVariantList vals;
                        for (int i=1; i<commaPos.size(); ++i) {
                            QString d = data.mid(commaPos.at(i-1)+1, commaPos.at(i)-commaPos.at(i-1)-1);
                            QString v = d.trimmed();
                            if (v.startsWith("\"") && v.endsWith("\"") && v.length()>1) {
                                v = v.mid(1, v.length()-2);                         }
                            vals.append(stringToVariant(unescapedString(v)));
                        }
                        //                    qWarning() << Q_FUNC_INFO << key;
                        settingsMap[key] = vals;
                    }
                }
            }
        }
    }
    
    if (mode.testFlag(QIODevice::WriteOnly))
    {
        device.open(mode);
    }

    return ok;
}

bool CryptSettings::IniWriteFunc(QIODevice &device, const QSettings::SettingsMap &settingsMap)
{
#ifdef Q_OS_WIN
    const char * const eol = "\r\n";
#else
    const char eol = '\n';
#endif
    bool writeError = false;

    // setEncypt
    SimpleCryptDevice deviceFilter(&device);
    setCryptProps(&deviceFilter);
    deviceFilter.open(QIODevice::WriteOnly);

    QString lastSection;
    QMapIterator<QString,QVariant> it(settingsMap);
    while(it.hasNext() && !writeError) {
        it.next();
        QString key = it.key();
        QString section;
        int idx = key.lastIndexOf(QChar('/'));
        if (idx == -1) {
            section = QString("[General]");
        } else {
            section = key.left(idx);
            key = key.mid(idx+1);
            if (section.compare(QString("General"), Qt::CaseInsensitive) == 0) {
                section = QString("[%General]");
            } else {
                section.prepend(QChar('['));
                section.append(QChar(']'));
            }
        }
        if (section.compare(lastSection, Qt::CaseInsensitive))
        {
            if (!lastSection.isEmpty()) {
                deviceFilter.write(eol);
            }
            lastSection = section;
            if (deviceFilter.write(section.toUtf8() + eol) == -1) {
                writeError = true;
            }
        }
        QByteArray block = key.toUtf8();
        block += " = ";
        if (it.value().type() == QVariant::StringList) {
            foreach (QString s, it.value().toStringList()) {
                block += escapedString(s);
                block += ", ";
            }
            if (block.endsWith(", ")) {
                block.chop(2);
            }
        } else if (it.value().type() == QVariant::List) {
            foreach (QVariant v, it.value().toList()) {
                block += escapedString(variantToString(v));
                block += ", ";
            }
            if (block.endsWith(", ")) {
                block.chop(2);
            }
        } else {
            block += escapedString(variantToString(it.value()));
        }
        block += eol;
        if (deviceFilter.write(block) == -1) {
            writeError = true;
        }
    }

    return writeError;
}

QString CryptSettings::variantToString(const QVariant &v)
{
    QString result;
    switch (v.type()) {
        case QVariant::String:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::Double:
        case QVariant::KeySequence: {
            result = v.toString();
            if (result.startsWith(QChar('@')))
                result.prepend(QChar('@'));
            break;
        }
        default: {
            QByteArray a;
            {
                QDataStream s(&a, QIODevice::WriteOnly);
                s.setVersion(QDataStream::Qt_4_0);
                s << v;
            }

            result = QString("@Variant(");
            result += QString::fromLatin1(a.constData(), a.size());
            result += QChar(')');
            break;
        }
    }

    return result;
}

QVariant CryptSettings::stringToVariant(const QString &s)
{
    if (s.startsWith(QChar('@'))) {
        if (s.endsWith(QChar(')'))) {
            if (s.startsWith(QString("@Variant("))) {
                QByteArray a(s.toUtf8().mid(9));
                QDataStream stream(&a, QIODevice::ReadOnly);
                stream.setVersion(QDataStream::Qt_4_0);
                QVariant result;
                stream >> result;
                return result;
            }
        }
        if (s.startsWith(QString("@@")))
            return QVariant(s.mid(1));
    }
    return QVariant(s);
}

QByteArray CryptSettings::escapedString(const QString &src)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    int i;
    QByteArray result;
    result.reserve(src.size() * 3 / 2);
    for (i = 0; i < src.size(); ++i) {
        uint ch = src.at(i).unicode();
        if (ch == ';' || ch == ',' || ch == '=' || ch == '#') {
            needsQuotes = true;
        }
        if (escapeNextIfDigit && ((ch >= '0' && ch <= '9')
                 || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
            result += "\\x";
            result += QByteArray::number(ch, 16);
            continue;
        }

        escapeNextIfDigit = false;

        switch (ch) {
        case '\0':
            result += "\\0";
            escapeNextIfDigit = true;
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '"':
        case '\\':
            result += '\\';
            result += (char)ch;
            break;
        default:
            if (ch <= 0x1F) {
                result += "\\x";
                result += QByteArray::number(ch, 16);
                escapeNextIfDigit = true;
            } else{
                result += QString(src[i]).toUtf8();
            }
        }
    }
    if (result.size()>0 && (result.at(0)==' ' || result.at(result.size() - 1) == ' ')) {
        needsQuotes = true;
    }
    if (needsQuotes) {
        result.prepend('"');
        result.append('"');
    }
    return result;
}


QString CryptSettings::unescapedString(const QString &src)
{
    static const char hexDigits[] = "0123456789ABCDEF";

    static const char escapeCodes[][2] =
    {
        { 'a', '\a' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
        { 'v', '\v' },
        { '"', '"' },
        { '?', '?' },
        { '\'', '\'' },
        { '\\', '\\' }
    };
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

    QString stringResult;
    int escapeVal = 0;
    QChar ch;
    int i = 0;
normal:
    while (i < src.size()) {
        ch = src.at(i);
        if (ch == QChar('\\')) {
            ++i;
            if (i >= src.size()) {
                break;
            }
            ch = src.at(i++);
            for (int j = 0; j < numEscapeCodes; ++j) {
                if (ch == escapeCodes[j][0]) {
                    stringResult += QChar(escapeCodes[j][1]);
                    goto normal;
                }
            }
            if (ch == 'x') {
                escapeVal = 0;
                if (i >= src.size())
                    break;
                ch = src.at(i);
                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
                    goto hexEscape;
            } else if (ch >= '0' && ch <= '7') {
                escapeVal = ch.unicode() - '0';
                goto octEscape;
            } else {
                //skip
            }
        } else {
            stringResult += ch;
        }
                i++;
    }
    goto end;

hexEscape:
    if (i >= src.size()) {
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = src.at(i);
    if (ch >= 'a')
        ch = ch.unicode() - ('a' - 'A');
    if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
        escapeVal <<= 4;
        escapeVal += strchr(hexDigits, ch.toLatin1()) - hexDigits;
        ++i;
        goto hexEscape;
    } else {
        stringResult += QChar(escapeVal);
        goto normal;
    }

octEscape:
    if (i >= src.size()) {
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = src.at(i);
    if (ch >= '0' && ch <= '7') {
        escapeVal <<= 3;
        escapeVal += ch.toLatin1() - '0';
        ++i;
        goto octEscape;
    } else {
        stringResult += QChar(escapeVal);
        goto normal;
    }

end:
    return stringResult;
}

