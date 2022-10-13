#ifndef SETTINGSWINDOW_HPP
#define SETTINGSWINDOW_HPP

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
#include <QSettings> //TODO external config file
#include <QCloseEvent>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>

#include "gui.hpp"
#include "control.hpp"
#include "mountsettings.hpp"

class SettingsWindow : public QDialog
{
    Q_OBJECT

signals:

    void
    savedSignal();

    void
    mountsSavedSignal();

public:

    SettingsWindow(MountSettings *settings, QWidget *parent = 0);

    QList<QMap<QString, QVariant>>
    result();

private slots:

    void
    save();

    void
    closeEvent(QCloseEvent *event);

    void
    loadMountsFrame();

    void
    loadConnectionsFrame();

    //void
    //saveSettings();

    void
    addMount(const QString &conn);

    void
    renameItem();

    void
    removeItem();

    void
    umountItem();

private:

    MountSettings*
    m_settings;

    MountSettings
    m_settings_new;

    QTabWidget
    *m_tab_widget;

    QWidget
    *m_wid_mounts;

    QWidget
    *m_wid_conns;

    QString
    getRcloneConfigPath();

    QStringList
    getConnectionNames();

    QStringList
    getMountpoints();

    QVariantMap
    getMountpointInfo(const QDir &mountpoint);

    bool
    isDirAccessible(const QDir &dir);

    bool
    isExternallyMounted(const QDir &dir);

    bool
    isDirectoryEmpty(const QDir &dir);



};

#endif
