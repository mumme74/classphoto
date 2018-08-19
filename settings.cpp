
#include "settings.h"


QSettings *settingsFactory(const QString groupName, QObject *owner)
{
    QSettings *newSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, SETTINGS_ORGANISATION, SETTINGS_APPLICATION, owner);
    newSettings->beginGroup(groupName);
    return newSettings;
}
