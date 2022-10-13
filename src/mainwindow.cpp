#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent)
          : QDialog(parent)
{
    //This is the main control window, where all configured mounts are listed.
    //One-time actions are in the settings window, not in the main window.

    //Main layout
    QVBoxLayout *vbox = new QVBoxLayout;
    #if true
    setLayout(vbox);
    #else
    QWidget *main = new QWidget;
    main->setLayout(vbox);
    setCentralWidget(main);
    #endif
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    //Title area
    QLabel *lbl_heading = new QLabel("RCLONE CONTROL");
    vbox->addWidget(lbl_heading);

    //Main frame (haha)
    m_frm_conns = new QFrame;
    QSizePolicy size_frm_conns = m_frm_conns->sizePolicy();
    m_frm_conns->setFrameStyle(QLabel::StyledPanel | QLabel::Plain);
    //horizontalPolicy = QSizePolicy::Preferred, verticalPolicy = QSizePolicy::MinimumExpanding
    size_frm_conns.setVerticalPolicy(QSizePolicy::MinimumExpanding);
    m_frm_conns->setSizePolicy(size_frm_conns);
    //m_frm_conns->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbox->addWidget(m_frm_conns);

    //Buttons at the bottom
    QHBoxLayout *hbox_btns = new QHBoxLayout;
    QPushButton *btn_settings = new QPushButton(tr("&Settings"));
    hbox_btns->addWidget(btn_settings);
    connect(btn_settings, SIGNAL(clicked()), SLOT(openSettings()));
    hbox_btns->addStretch();
    QPushButton *btn_quit = new QPushButton(tr("&Quit"));
    hbox_btns->addWidget(btn_quit);
    vbox->addLayout(hbox_btns);

    //System tray icon
    QIcon ico(":/res/folderblack_3.png");
    m_tray_icon = new QSystemTrayIcon(ico);
    m_tray_icon->setToolTip(qApp->applicationName());
    m_tray_icon->setVisible(true);
    m_mnu_tray = new QMenu;
    m_tray_icon->setContextMenu(m_mnu_tray);
    connect(this, SIGNAL(mountStateChanged(const QString&)), SLOT(updateTrayMenu(const QString&)));
    connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    //Load window dimensions from settings
    //Default settings group "main" set in main routine (main.cpp)
    MountSettings *settings = getSettings();
    //SettingsManager *settings = getSettings();
    resize(settings->variant("size", QSize(400, 400)).toSize());
    move(settings->variant("pos", QPoint(200, 200)).toPoint());

    //Load saved mounts, initialize list
    initConnections();

}

void
MainWindow::openSettings()
{
    SettingsWindow *settings_window = new SettingsWindow(getSettings(), this);
    //connect(settings_window, SIGNAL(saved()), SLOT(saveSettings()));
    settings_window->setModal(true);
    settings_window->exec();
    QList<QVariantMap> conns = settings_window->result();
    initConnections();
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
    //Default settings group "main" set in main routine (main.cpp)
    MountSettings *settings = getSettings();
    settings->setVariant("size", size());
    settings->setVariant("pos", pos());

    saveSettings();
    event->accept();
}

void
MainWindow::reject()
{
    QTimer::singleShot(0, this, SLOT(hide()));
}

void
MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        show();
    }
}

void
MainWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        if (this->windowState() & Qt::WindowMinimized)
        {
            QTimer::singleShot(0, this, SLOT(hide()));
        }
    }

    QDialog::changeEvent(e);
}

void
MainWindow::loadConnections(QList<QVariantMap> conn_list)
{
    foreach (QObject *child, m_frm_conns->children()) //also see findChildren()
        child->deleteLater();
    QVBoxLayout *frm_vbox = new QVBoxLayout;
    delete m_frm_conns->layout();
    m_frm_conns->setLayout(frm_vbox);
    m_btn_map.clear();

    //Import/load configured connections in frame
    int s_connections_count = conn_list.size();
    for (int i = 0; i < s_connections_count; ++i)
    {
        QVariantMap conn_config = conn_list[i];
        QString mountpoint = conn_config["mountpoint"].toString();
        QString title = conn_config["label"].toString();
        if (title.isEmpty())
            title = conn_config["connection"].toString();
        ItemButton *itm_btn = new ItemButton(title);
        itm_btn->setSignalName(mountpoint); //mountpoint used to identify config item
        itm_btn->setToolTip(mountpoint);
        connect(itm_btn, SIGNAL(clicked(const QString&)), SLOT(actButton(const QString&)));
        connect(itm_btn, SIGNAL(entered()), SLOT(updateButton()));
        connect(itm_btn, SIGNAL(left()), SLOT(updateButton()));
        frm_vbox->addWidget(itm_btn);
        frm_vbox->addSpacing(5);
        m_btn_map[mountpoint] = itm_btn;
    }

    if (!s_connections_count)
    {
        ItemButton *itm_btn = new ItemButton(tr("(No mounts configured yet...)"));
        itm_btn->setLineWidth(2);
        connect(itm_btn, SIGNAL(clicked()), SLOT(openSettings()));
        frm_vbox->addWidget(itm_btn);
        frm_vbox->addSpacing(5);
    }

    frm_vbox->addStretch();

    updateTrayMenu(); //TODO get rid of list argument
}

void
MainWindow::initConnections()
{
    MountSettings *settings = getSettings();

    QList<QVariantMap> conn_list = settings->mountConfigList();
    loadConnections(conn_list);
}

void
MainWindow::saveSettings()
{
    getSettings()->save();
}

void
MainWindow::updateButton(const QString &mountpoint, int mode)
{
    if (mountpoint.isEmpty()) return;
    QPointer<ItemButton> button = m_btn_map.value(mountpoint);

    if (mode == -1) mode = isConnected(mountpoint) ? 1 : 0;
    if (!mode)
    {
        //Paint connect button
        button->setHoverBgColor(QColor("limegreen"));
        button->setHoverFgColor(QColor("crimson"));
        //button->setBgColor(QColor("slateblue"));
        button->resetBgColor();
        button->setToolTip(tr("Mount: %1").arg(mountpoint));
    }
    else
    {
        //Paint disconnect button
        button->setHoverBgColor(QColor("crimson"));
        button->setHoverFgColor(QColor("limegreen"));
        button->setBgColor(QColor("limegreen"));
        button->setToolTip(tr("Unmount: %1").arg(mountpoint));
    }
}

void
MainWindow::updateButton()
{
    ItemButton *button = qobject_cast<ItemButton*>(QObject::sender());
    if (!button) return;
    updateButton(button->signalName());
}

void
MainWindow::actButton(const QString &mountpoint)
{
    //Connect if disconnected, disconnect if connected
    switchConnection(mountpoint);

    //Do NOT update button style before receiving signal confirmation
}

void
MainWindow::actButton(ItemButton *button)
{
    QString mountpoint = button->signalName();
    actButton(mountpoint);
}

void
MainWindow::mount(const QString &mountpoint)
{
    QPointer<MountControl> mount = MountControl::fromMountpoint(mountpoint);
    if (!mount)
    {
        //Create new mount object (reference will be saved on mount())
        QVariantMap cfg = getSettings()->mountConfig(mountpoint);
        QString conn = cfg["connection"].toString();
        mount = MountControl::fromMountpoint(mountpoint, conn);
        if (!mount) return; //error
        connect(mount.data(), SIGNAL(mountedSignal(const QString&)), SLOT(mounted(const QString&)));
        connect(mount.data(), SIGNAL(umountedSignal(const QString&, int, const QByteArray&)), SLOT(umounted(const QString&, int, const QByteArray&)));
    }
    mount->mount();
}

void
MainWindow::umount(const QString &mountpoint)
{
    QPointer<MountControl> mount = MountControl::fromMountpoint(mountpoint);
    if (!mount) return;
    mount->umount();
    mount->discard();
}

void
MainWindow::switchConnection(const QString &mountpoint)
{
    if (isConnected(mountpoint))
        umount(mountpoint);
    else
        mount(mountpoint);
}

void
MainWindow::switchConnection(ItemButton *button)
{
    QString mountpoint;
    if (!button)
    {
        QObject *obj = QObject::sender();
        if (ItemButton *button = qobject_cast<ItemButton*>(obj))
            mountpoint = button->signalName();
        else if (obj = qobject_cast<QAction*>(obj))
            mountpoint = obj->property("mountpoint").toString();
    }
    switchConnection(mountpoint);
}

void
MainWindow::mounted(const QString &mountpoint)
{
    //Trigger menu update
    emit mountStateChanged(mountpoint);

    //Paint active button
    updateButton(mountpoint, 1);

    //Show notification
    QString title = tr("Mounted: %1").arg(mountpoint);
    QString msg = tr("This mountpoint has been activated.");
    m_tray_icon->showMessage(title, msg);
}

void
MainWindow::umounted(const QString &mountpoint, int rc, const QByteArray &err_output)
{
    //Trigger menu update
    emit mountStateChanged(mountpoint);

    //Paint default button
    updateButton(mountpoint, 0);

    //Show error message, if any
    if (rc)
    {
        //Show error as balloon tip
        QString title = tr("Error: %1").arg(mountpoint);
        QString msg = tr("The mount control process has returned an error.");
        if (!err_output.isEmpty())
            msg = err_output;
        m_tray_icon->showMessage(title, msg, QSystemTrayIcon::Critical);
        //Show message box if main window is active
        if (isVisible())
        {
            QMessageBox::critical(this, title, msg);
        }
    }
    else
    {
        QString title = tr("Unmounted: %1").arg(mountpoint);
        QString msg = tr("This mountpoint was unmounted.");
        m_tray_icon->showMessage(title, msg);
    }
}

void
MainWindow::updateTrayMenu()
{
    //QMenu *menu = new QMenu;
    QMenu *menu = m_mnu_tray;
    foreach (QAction *act, menu->actions())
        act->deleteLater();
    QAction *act = menu->addAction(tr("Open"));
    connect(act, SIGNAL(triggered()), SLOT(show()));
    connect(act, SIGNAL(triggered()), SLOT(raise()));
    menu->addSeparator();
    foreach (const QVariantMap &cfg, getSettings()->mountConfigList())
    {
        QString mountpoint = cfg["mountpoint"].toString();
        QString title = cfg["label"].toString();
        if (title.isEmpty())
            title = cfg["connection"].toString();
        act = menu->addAction(title);
        act->setCheckable(true);
        act->setChecked(isConnected(mountpoint));
        act->setProperty("mountpoint", mountpoint);
        connect(act, SIGNAL(triggered()), SLOT(switchConnection()));
    }
    menu->addSeparator();
    //act = menu->addAction(tr("Quit"));

}

void
MainWindow::updateTrayMenu(const QString &mountpoint)
{
    foreach (QAction *act, m_mnu_tray->actions())
    {
        if (act->property("mountpoint").toString() != mountpoint) continue;

        act->setChecked(isConnected(mountpoint));
        break;
    }
}

MountSettings*
MainWindow::getSettings()
{
    //MountSettings *settings_ptr = static_cast<MountSettings*>(MountSettings::globalInstance());
    MountSettings *settings_ptr = MountSettings::globalInstance();
    return settings_ptr;
}

bool
MainWindow::isConnected(const QString &mountpoint)
{
    return MountControl::activeMountpoints().contains(mountpoint);
}

