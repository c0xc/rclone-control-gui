#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <cassert>

#include <QDebug>
#include <QApplication>
#include <QPixmap>
#include <QDialog>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QCloseEvent>
#include <QPushButton>
#include <QPointer>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>

#include "mountsettings.hpp"
#include "settingswindow.hpp"

class MainWindow : public QDialog
{
    Q_OBJECT

signals:

    void
    mountStateChanged(const QString &mountpoint);

public:

    MainWindow(QWidget *parent = 0);

public slots:

    void
    openSettings();

private slots:

    void
    closeEvent(QCloseEvent *event);

    void
    reject();

    void
    iconActivated(QSystemTrayIcon::ActivationReason reason);

    void
    changeEvent(QEvent* e);

    void
    loadConnections(QList<QVariantMap> conn_list);

    void
    initConnections();

    void
    updateTrayMenu(const QString &mountpoint);

    void
    updateTrayMenu();

    void
    saveSettings();

    void
    updateButton(const QString &mountpoint, int mode = -1);

    void
    updateButton();

    void
    actButton(const QString &mountpoint);

    void
    actButton(ItemButton *button);

    void
    mount(const QString &mountpoint);

    void
    umount(const QString &mountpoint);

    void
    switchConnection(const QString &mountpoint);

    void
    switchConnection(ItemButton *button = 0);

    void
    mounted(const QString &mountpoint);

    /**
     * This is called when the mount process ends.
     */
    void
    umounted(const QString &mountpoint, int rc, const QByteArray &err_output);

private:

    QSystemTrayIcon
    *m_tray_icon;

    QFrame
    *m_frm_conns;

    QPointer<QMenu>
    m_mnu_tray;

    QMap<QString, QPointer<ItemButton>>
    m_btn_map;

    MountSettings*
    getSettings();

    bool
    isConnected(const QString &mountpoint);


};

#endif
