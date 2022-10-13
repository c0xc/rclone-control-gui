#include "control.hpp"

QStringList
MountControl::activeMountpoints()
{
    QStringList mountpoints;
    foreach (QPointer<MountControl> m, activeConnections())
        if (m->isMounted()) mountpoints << m->mountpoint();
    return mountpoints;
}

QPointer<MountControl>
MountControl::fromMountpoint(const QDir &mountpoint, const QString &conn, bool return_new)
{
    QPointer<MountControl> mount;
    foreach (QPointer<MountControl> m, activeConnections())
    {
        if (m->mountpoint() == mountpoint.path())
        {
            mount = m;
            break;
        }
    }

    if (!mount && (!conn.isEmpty() || return_new))
    {
        mount = QPointer<MountControl>(new MountControl(mountpoint));
        mount->setConnection(conn);
    }

    return mount;
}

QPointer<MountControl>
MountControl::fromMountpoint(const QDir &mountpoint, bool return_new)
{
    return fromMountpoint(mountpoint, "", return_new);
}

MountControl::MountControl(const QDir &mountpoint)
            : QObject(),
              m_mounted(false)
{
    m_mountpoint = mountpoint.path();
    m_proc.setProgram(getRclonePath());
    connect(&m_proc, SIGNAL(destroyed()), SLOT(checkStateDestroyed()));
    connect(&m_proc, SIGNAL(started()), SLOT(checkStateStarted()));
    connect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(checkStateFinished(int, QProcess::ExitStatus)));
    connect(&m_proc, SIGNAL(error(QProcess::ProcessError)), SLOT(checkStateError(QProcess::ProcessError)));
}

QString
MountControl::mountpoint() const
{
    return m_mountpoint;
}

void
MountControl::setConnection(QString conn)
{
    m_r_conn = conn;
}

bool
MountControl::isExternallyMounted() const
{

    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes())
    {
        QDir storage_dir(storage.rootPath());
        if (storage_dir == m_mountpoint)
        {
            return true;
        }
    }

    return false;
}

bool
MountControl::isMounted() const
{
    return m_mounted;
}

bool
MountControl::mount()
{
    //Keep reference to this object in internal list (while active)
    //This reference can be retrieved using fromMountpoint().
    //It will be removed as soon as the process is terminated.
    //But we need to keep this reference to prevent this control object
    //to be destroyed while the process is running.
    bool is_watched = false;
    foreach (QPointer<MountControl> m, activeConnections())
        if (m == this) is_watched = true;
    if (!is_watched)
    {
        //We'll delete it later.
        //If we wanted to use a QSharedPointer, we'd have to store
        //this pointer in the constructor function to have it available here.
        activeConnections() << QPointer<MountControl>(this);
    }

    //Return if already mounted or nothing to mount
    if (m_mounted) return false;
    if (m_r_conn.isEmpty()) return false;

    //Mount arguments
    QString remote_path = "/"; //TODO settings (MountSettings)
    QStringList args; //rclone ...
    args << "mount";
    args << m_r_conn + ":" + remote_path;
    args << mountpoint();
    m_proc.setArguments(args);
    m_proc.start();

    m_mounted = true;
}

void
MountControl::umount()
{
    if (!umountSystem())
    {
        m_proc.terminate(); //soft force option
    }
    m_proc.waitForFinished();
}

void
MountControl::discard()
{
    //Remove this reference from internal list
    activeConnections().removeAll(QPointer<MountControl>(this));
    //Delete self
    //This is necessary because this object isn't tracked by anything
    //Or we could switch from QPointer to QSharedPointer (again)
    deleteLater();
}

bool
MountControl::umountSystem()
{
    QProcess proc;
    proc.setProgram("fusermount");
    QStringList args;
    args << "-u" << mountpoint();
    proc.setArguments(args);
    proc.start();

    proc.waitForFinished();
    return proc.exitCode() == 0;
}

void
MountControl::checkStateDestroyed()
{
}

void
MountControl::checkStateStarted()
{
    //Mount process has started
    emit startedSignal(mountpoint());
    //Wait a bit before emitting mounted signal
    //If the mount command fails immediately, started will be emitted
    //followed by finished.
    QTimer::singleShot(1000, this, SLOT(checkStateMounted()));
}

void
MountControl::checkStateMounted()
{
    if (!m_mounted) return;
    emit mountedSignal(mountpoint());
}

void
MountControl::checkStateError(QProcess::ProcessError error)
{
    m_mounted = false;
}

void
MountControl::checkStateFinished(int rc, QProcess::ExitStatus status)
{
    m_mounted = false;
    QByteArray err_output = m_proc.readAllStandardError();
    emit umountedSignal(mountpoint(), rc, err_output);

    if (status == QProcess::NormalExit)
    {
        discard();
    }
}

QString
MountControl::getRclonePath()
{
    //TODO Settings?
    QString bin_dir = "/usr/bin/rclone";
    return bin_dir;
}

QList<QPointer<MountControl>>&
MountControl::activeConnections()
{
    static QList<QPointer<MountControl>> mounts;
    return mounts;
}

