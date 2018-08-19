#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>
#include <QObject>

#define SETTINGS_ORGANISATION "mummesoft"
#define SETTINGS_APPLICATION "classphoto"
#define LAST_OPENED_PROJECT_PATH "lastOpenedProjectPath"



QSettings *settingsFactory(const QString groupName, QObject *owner);



#endif // SETTINGS_H
