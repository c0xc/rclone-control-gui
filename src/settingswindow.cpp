#include "settingswindow.hpp"

SettingsWindow::SettingsWindow(MountSettings *settings, QWidget *parent)
              : QDialog(parent),
                m_settings(settings)
{
    //List of mount config items
    //They are copied here and the copy is modified
    //until the user confirms.
    m_settings_new = *settings;

    //Main layout
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    m_tab_widget = new QTabWidget;
    vbox->addWidget(m_tab_widget);

    //Frames with connections, mountpoints
    QWidget *wid_conns = new QWidget;
    m_tab_widget->addTab(wid_conns, tr("Connections"));
    QVBoxLayout *vbox_conns = new QVBoxLayout;
    wid_conns->setLayout(vbox_conns);
    //Mountpoints frame
    QGroupBox *grp_mounts = new QGroupBox(tr("Configured Mountpoints"));
    vbox_conns->addWidget(grp_mounts);
    QVBoxLayout *vbox1 = new QVBoxLayout;
    grp_mounts->setLayout(vbox1);
    m_wid_mounts = new QWidget;
    vbox1->addWidget(m_wid_mounts);
    //Connections frame
    QGroupBox *grp_conns = new QGroupBox(tr("Available Connections"));
    vbox_conns->addWidget(grp_conns);
    QVBoxLayout *vbox2 = new QVBoxLayout;
    grp_conns->setLayout(vbox2);
    m_wid_conns = new QWidget;
    vbox2->addWidget(m_wid_conns);
    //Initialize both frames
    loadMountsFrame();
    loadConnectionsFrame();

    QWidget *wid_settings = new QWidget;
    m_tab_widget->addTab(wid_settings, tr("General"));
    QFormLayout *settings_box = new QFormLayout;
    wid_settings->setLayout(settings_box);
    QLineEdit *txt_rclone_path = new QLineEdit;
    txt_rclone_path->setText(MountControl::getRclonePath());
    txt_rclone_path->setDisabled(true);
    settings_box->addRow(tr("Path to rclone"), txt_rclone_path);

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btn_save = new QPushButton(tr("&Save"));
    connect(btn_save, SIGNAL(clicked()), SLOT(save()));
    hbox->addWidget(btn_save);
    QPushButton *btn_close = new QPushButton(tr("&Close"));
    connect(btn_close, SIGNAL(clicked()), SLOT(close()));
    hbox->addWidget(btn_close);
    vbox->addLayout(hbox);

}

QList<QVariantMap>
SettingsWindow::result()
{
    return m_settings_new.mountConfigList();
}

void
SettingsWindow::save()
{
    //Save new mountpoint configurations
    if (m_settings)
    {
        //m_settings->setMountConfigList(m_mounts);
        m_settings->replaceWith(m_settings_new);
        m_settings->save();
    }

    //Send signal for main window to show new configuration
    emit savedSignal();
    emit mountsSavedSignal();

    close();
}

void
SettingsWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void
SettingsWindow::loadMountsFrame()
{
    foreach (QObject *child, m_wid_mounts->children())
        child->deleteLater();
    QVBoxLayout *vbox = new QVBoxLayout;
    delete m_wid_mounts->layout();
    m_wid_mounts->setLayout(vbox);

    QStringList mountpoints = getMountpoints();
    foreach (QString mountpoint, mountpoints)
    {
        QVariantMap info = getMountpointInfo(mountpoint);
        QString conn_name = info.value("connection").toString();
        ItemButton *itm_mount = new ItemButton(mountpoint);
        itm_mount->setSubtitle(conn_name);
        itm_mount->setHoverBgColor("steelblue");
        vbox->addWidget(itm_mount);
        vbox->addSpacing(5);
        QAction *act_rename = itm_mount->addAction(tr("Rename"));
        act_rename->setData(mountpoint);
        connect(act_rename, SIGNAL(triggered()), SLOT(renameItem()));
        QAction *act_remove = itm_mount->addAction(tr("Remove"));
        act_remove->setData(mountpoint);
        connect(act_remove, SIGNAL(triggered()), SLOT(removeItem()));
        QAction *act_umount = itm_mount->addAction(tr("Unmount"));
        act_umount->setData(mountpoint);
        connect(act_umount, SIGNAL(triggered()), SLOT(umountItem()));
    }
    if (mountpoints.isEmpty())
    {
        ItemButton *itm_mount = new ItemButton(
            tr("(No mountpoints configured. Select a connection to mount it.)"),
            true);
        vbox->addWidget(itm_mount);
        vbox->addSpacing(5);
    }

}

void
SettingsWindow::loadConnectionsFrame()
{
    foreach (QObject *child, m_wid_conns->children())
        child->deleteLater();
    QVBoxLayout *vbox = new QVBoxLayout;
    delete m_wid_conns->layout();
    m_wid_conns->setLayout(vbox);

    QStringList conn_names = getConnectionNames();
    foreach (QString name, conn_names)
    {
        ItemButton *itm_conn = new ItemButton(name);
        itm_conn->setSignalName(name);
        connect(itm_conn, SIGNAL(clicked(const QString&)), SLOT(addMount(const QString&)));
        vbox->addWidget(itm_conn);
        vbox->addSpacing(5);
    }
    if (conn_names.isEmpty())
    {
        ItemButton *itm_conn = new ItemButton(
            tr("(No connections found.)"),
            true);
        itm_conn->setDisabled(true);
        vbox->addWidget(itm_conn);
        vbox->addSpacing(5);
    }

}

void
SettingsWindow::renameItem()
{
    QAction *action = qobject_cast<QAction*>(QObject::sender());
    QString mountpoint = action->data().toString();
    if (mountpoint.isEmpty()) return;
    if (MountControl::fromMountpoint(mountpoint, true)->isMounted())
    {
        QMessageBox::critical(this, tr("Mountpoint is active"),
            tr("This mountpoint cannot be modified because it is active."));
        return;
    }

    QVariantMap cfg = m_settings_new.mountConfig(mountpoint);

    QString cur_label = cfg["label"].toString();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit mountpoint"),
        tr("Label for mountpoint at %1:").arg(mountpoint),
        QLineEdit::Normal, cur_label, &ok);
    if (!ok) return;

    cfg["label"] = text;
    m_settings_new.setMountConfig(cfg);

    loadMountsFrame();
}

void
SettingsWindow::removeItem()
{
    QAction *action = qobject_cast<QAction*>(QObject::sender());
    QString mountpoint = action->data().toString();
    if (mountpoint.isEmpty()) return;
    if (MountControl::fromMountpoint(mountpoint, true)->isMounted())
    {
        QMessageBox::critical(this, tr("Mountpoint is active"),
            tr("This mountpoint cannot be modified because it is active."));
        return;
    }

    QVariantMap cfg = m_settings_new.mountConfig(mountpoint);

    QMessageBox::StandardButton btn = QMessageBox::question(this,
        tr("Edit mountpoint"),
        tr("Are you sure you want to remove this mountpoint?\n%1").arg(mountpoint));
    if (btn != QMessageBox::Yes) return;

    m_settings_new.removeMountConfig(mountpoint);

    loadMountsFrame();
}

void
SettingsWindow::umountItem()
{
    QAction *action = qobject_cast<QAction*>(QObject::sender());
    QString mountpoint = action->data().toString();
    if (mountpoint.isEmpty()) return;

    if (MountControl::fromMountpoint(mountpoint, true)->isMounted())
    {
        if (QMessageBox::information(this, tr("Mountpoint is active"),
            tr("This mountpoint is active. It will be stopped and unmounted now."),
            QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        MountControl::fromMountpoint(mountpoint)->umount();
        QMessageBox::information(this, tr("Mountpoint unmounted"),
            tr("This mountpoint has been unmounted."));
    }
    else if (MountControl::fromMountpoint(mountpoint, true)->isExternallyMounted())
    {
        if (QMessageBox::information(this, tr("Mountpoint is in use"),
            tr("This mountpoint is in use but not managed by this program. An attempt will now be made to unmount it."),
            QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        if (!MountControl::fromMountpoint(mountpoint, true)->umountSystem())
            QMessageBox::critical(this, tr("Mountpoint is in use"),
                tr("Failed to unmount: %1").arg(mountpoint));
        else
            QMessageBox::information(this, tr("Mountpoint unmounted"),
                tr("This mountpoint has been unmounted."));
    }
    else
    {
        QMessageBox::information(this, tr("Mountpoint not active"),
            tr("This mountpoint is not mounted."));
    }
}

QString
SettingsWindow::getRcloneConfigPath()
{
    //~/.config/rclone/rclone.conf
    //TODO Settings, SettingsManager...
    QString file_path;
    file_path = QDir::home().absoluteFilePath(".config/rclone/rclone.conf");
    return file_path;
}

QStringList
SettingsWindow::getConnectionNames()
{
    QString file_path = getRcloneConfigPath();

    QSettings rclone_config(file_path, QSettings::IniFormat); //TODO rclone config

    return rclone_config.childGroups();
}

QStringList
SettingsWindow::getMountpoints()
{
    //Get configured mountpoints
    QStringList list;
    foreach (const QVariantMap &cfg, m_settings_new.mountConfigList())
        list.append(cfg.value("mountpoint").toString());
    return list;
}

QVariantMap
SettingsWindow::getMountpointInfo(const QDir &mountpoint)
{
    return m_settings_new.mountConfig(mountpoint.path());
}

void
SettingsWindow::addMount(const QString &conn)
{
    if (!getMountpointInfo(conn).isEmpty()) return;

    QString dir_path;
    do
    {

        //Show file dialog and let user select a directory
        dir_path = QFileDialog::getExistingDirectory(this,
            tr("Select mountpoint for: %1").arg(conn),
            QDir::homePath());
        if (dir_path.isEmpty()) return;
        QDir dir(dir_path);

        //Check if selected dir is already configured
        if (!getMountpointInfo(dir).isEmpty())
        {
            if (QMessageBox::critical(this, tr("Cannot add mountpoint"),
                tr("This mountpoint is already defined."),
                QMessageBox::Cancel | QMessageBox::Retry
            ) == QMessageBox::Cancel)
                return;
            dir_path = "";
        }

        //Check if dir is accessible to $USER
        //TODO if configured, allow only dirs within $HOME, matching schema X ...
        if (!isDirAccessible(dir))
        {
            if (QMessageBox::critical(this, tr("Cannot add mountpoint"),
                tr("This directory is not accessible."),
                QMessageBox::Cancel | QMessageBox::Retry
            ) == QMessageBox::Cancel)
                return;
            dir_path = "";
        }

        //Check if dir is already mounted (not by this program)
        if (isExternallyMounted(dir))
        {
            if (QMessageBox::critical(this, tr("Cannot add mountpoint"),
                tr("This directory is already mounted."),
                QMessageBox::Cancel | QMessageBox::Retry
            ) == QMessageBox::Cancel)
                return;
            dir_path = "";
        }

        //Check if dir contains files
        if (!isDirectoryEmpty(dir))
        {
            if (QMessageBox::critical(this, tr("Cannot add mountpoint"),
                tr("This directory is not empty. You need to select an empty directory."),
                QMessageBox::Cancel | QMessageBox::Retry,
                QMessageBox::Cancel
            ) == QMessageBox::Cancel)
                return;
            dir_path = "";
        }

    } while (dir_path.isEmpty());

    QMessageBox::information(this, tr("New mountpoint"),
        tr("You have created a new mountpoint at %1 for the connection %2.").
        arg(dir_path, conn));

    QVariantMap mount_conf;
    mount_conf["connection"] = conn;
    mount_conf["mountpoint"] = dir_path;
    m_settings_new.setMountConfig(mount_conf);

    loadMountsFrame();

}

bool
SettingsWindow::isDirAccessible(const QDir &dir)
{
    return dir.isReadable();
}

bool
SettingsWindow::isExternallyMounted(const QDir &dir)
{
    MountControl mount(dir);
    return mount.isExternallyMounted();
}

bool
SettingsWindow::isDirectoryEmpty(const QDir &dir)
{
    return dir.isEmpty();
}

