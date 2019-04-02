#ifndef CRYPTSETTINGS_H
#define CRYPTSETTINGS_H
#include <QString>
#include <QIODevice>
#include <QSettings>

class SimpleCryptDevice;
class CryptSettings {

public:
    static void InitCryptSettings();
    static QSettings::Format format();

private:
    static void setCryptProps(SimpleCryptDevice* device);
    static bool IniReadFunc(QIODevice &device, QSettings::SettingsMap &settingsMap);
    static bool IniWriteFunc(QIODevice &device, const QSettings::SettingsMap &settingsMap);
    static QString variantToString(const QVariant &v);
    static QVariant stringToVariant(const QString &s);
    static QByteArray escapedString(const QString &src);
    static QString unescapedString(const QString &src);

private:
    static quint64 m_key;
    static QSettings::Format     m_format;

private:
    CryptSettings() {}
    ~CryptSettings() {}
};

#endif // CRYPTSETTINGS_H
