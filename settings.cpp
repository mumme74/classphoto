
#include "settings.h"


const uint majorVersion = 0,
           minorVersion = 2,
           bugFixVersion = 1;

const QString copyRight("2010, 2017, 2018, 2019 Fredrik Johansson");

QSettings *settingsFactory(const QString groupName, QObject *owner)
{
    QSettings *newSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, SETTINGS_ORGANISATION, SETTINGS_APPLICATION, owner);
    newSettings->beginGroup(groupName);
    return newSettings;
}
