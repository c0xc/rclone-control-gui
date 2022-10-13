#include "mountsettings.hpp"

MountSettings*
MountSettings::globalInstance()
{
    static MountSettings global_instance;
    return &global_instance;
}

MountSettings::MountSettings()
             : SettingsManager()
{
    if (!m_state["default_group_exception"].isValid())
        m_state["default_group_exception"] = QStringList() << "mount_list";
}

MountSettings::MountSettings(const MountSettings &other)
             : SettingsManager(other)
{
    if (!m_state["default_group_exception"].isValid())
        m_state["default_group_exception"] = QStringList() << "mount_list";
}

QList<QVariantMap>
MountSettings::mountConfigList() const
{
    QList<QVariantMap> list;
    foreach (QVariant v, variant("mount_list").toList())
    {
        QVariantMap cfg = v.toMap();
        list.append(cfg);
    }
    return list;
}

QVariantMap
MountSettings::mountConfig(const QString &mountpoint) const
{
    foreach (QVariantMap cfg, mountConfigList())
    {
        QString cur_mountpoint = cfg.value("mountpoint").toString();
        if (cur_mountpoint == mountpoint)
            return cfg;
    }

    return QVariantMap(); //not found, return empty map
}

void
MountSettings::setMountConfig(const QString &mountpoint, const QVariantMap &config)
{
    //Find and replace config item
    QList<QVariantMap> list = mountConfigList();
    bool found = false;
    for (int i = 0, ii = list.size(); i < ii; i++)
    {
        QString cur_mountpoint = list[i].value("mountpoint").toString();
        if (cur_mountpoint == mountpoint)
        {
            if (!config.isEmpty())
                list[i] = config; //replace mount config
            else
                list.removeAt(i);
            found = true;
            break;
        }
    }
    if (!found && !config.isEmpty())
        list.append(config); //add new mount config

    QVariantList var_list;
    foreach (QVariantMap item, list)
    {
        var_list << QVariant(item);
    }
    setVariant("mount_list", QVariant(var_list));
}

void
MountSettings::setMountConfig(const QVariantMap &config)
{
    QString mountpoint = config.value("mountpoint").toString();
    if (mountpoint.isEmpty())
        return; //bad call, empty config
    setMountConfig(mountpoint, config);
}

void
MountSettings::removeMountConfig(const QString &mountpoint)
{
    setMountConfig(mountpoint, QVariantMap());
}

void
MountSettings::setMountConfigList(const QList<QVariantMap> &config)
{
    //List of old mountpoints
    QStringList old_mountpoints;
    foreach (QVariantMap cfg, mountConfigList())
    {
        old_mountpoints << cfg["mountpoint"].toString();
    }

    //Add, update mountpoints
    foreach (QVariantMap cfg, config)
    {
        old_mountpoints.removeAll(cfg["mountpoint"].toString());
        setMountConfig(cfg);
    }

    //Delete removed mountpoints
    foreach (QString mp, old_mountpoints)
    {
        removeMountConfig(mp);
    }

}

