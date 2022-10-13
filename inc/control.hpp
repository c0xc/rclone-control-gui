#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <cassert>

#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QStorageInfo>
#include <QProcess>
#include <QPointer>
#include <QTimer>

//typedef MountControlPointer QSharedPointer<MountControl>;

class MountControl : public QObject
{
    Q_OBJECT

signals:

    void
    startedSignal(const QString &mountpoint);

    void
    mountedSignal(const QString &mountpoint);

    void
    umountedSignal(const QString &mountpoint, int rc, const QByteArray &err_output);

    //void
    //umountedSignal(const QSharedPointer<MountControl> &mount);

public:

    static QStringList
    activeMountpoints();

    static QPointer<MountControl>
    fromMountpoint(const QDir &mountpoint, const QString &conn = QString(), bool return_new = false);

    static QPointer<MountControl>
    fromMountpoint(const QDir &mountpoint, bool return_new);

    MountControl(const QDir &mountpoint);

    QString
    mountpoint() const;

    void
    setConnection(QString conn);

    bool
    isExternallyMounted() const;

    bool
    isMounted() const;

    static QString
    getRclonePath();

public slots:

    bool
    mount();

    void
    umount();

    void
    discard();

    bool
    umountSystem();

private slots:

    void
    checkStateDestroyed();

    void
    checkStateStarted();

    void
    checkStateMounted();

    void
    checkStateError(QProcess::ProcessError error);

    void
    checkStateFinished(int rc, QProcess::ExitStatus status);

private:

    QString
    m_mountpoint;

    QString
    m_r_conn;

    bool
    m_mounted;

    QProcess
    m_proc;

    static QList<QPointer<MountControl>>&
    activeConnections();

};

#endif
