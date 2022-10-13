#ifndef GUI_HPP
#define GUI_HPP

#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>
#include <QPushButton>

class ItemButton : public QFrame
{
    Q_OBJECT

signals:

    void
    entered();

    void
    left();

    void
    clicked();

    void
    clicked(const QString &name);

    void
    triggered(QAction *action);

public:

    ItemButton(QString title, bool disabled = false, QWidget *parent = 0);

    void
    resetBgColor();

    void
    setBgColor(QColor color);

    void
    setFgColor(QColor color);

    void
    setHoverBgColor(QColor color);

    void
    setHoverFgColor(QColor color);

    void
    setSubtitle(QString text);

    QString
    signalName() const;

    void
    setSignalName(QString name);

    QMenu*
    menu();

    QMenu*
    setButtonMenu(QMenu *menu);

    QMenu*
    setContextMenu(QMenu *menu);

    QAction*
    addAction(const QString &text);

private slots:

    void
    updateColor();

    void
    enterEvent(QEvent *event);

    void
    leaveEvent(QEvent *event);

    void
    mousePressEvent(QMouseEvent *event);

    void
    mouseReleaseEvent(QMouseEvent *event);

    void
    showContextMenu(const QPoint &pos);

    void
    trigger(QAction *action);

private:

    bool
    m_disabled;

    bool
    m_mouse_hover;

    bool
    m_mouse_pressed;

    QString
    m_sig_name;

    QPalette
    m_pal_default;

    QPalette
    m_pal_default_old;

    QPalette
    m_pal_hover;

    QPalette
    m_pal_active;

    QLabel
    *m_lbl_title;

    QLabel
    *m_lbl_subtitle;

    QPushButton
    *m_btn_functions;

    QPointer<QMenu>
    m_ctx_menu;



};

#endif
