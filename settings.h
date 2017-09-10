#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>
#include <QObject>

#define SETTINGS_ORGANISATION "mummesoft"
#define SETTINGS_APPLICATION "classphoto"



QSettings *settingsFactory(const QString groupName, QObject *owner)
{
    QSettings *newSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, SETTINGS_ORGANISATION, SETTINGS_APPLICATION, owner);
    newSettings->beginGroup(groupName);
    return newSettings;
}



#endif // SETTINGS_H
