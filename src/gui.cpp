#include "gui.hpp"

ItemButton::ItemButton(QString title, bool disabled, QWidget *parent)
          : QFrame(parent)
{
    m_disabled = disabled;
    m_mouse_hover = false;
    m_mouse_pressed = false;

    //Main layout
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);

    //Button-y frame style
    //setFrameStyle(QLabel::StyledPanel | QLabel::Plain);
    setFrameStyle(QLabel::Panel | QLabel::Raised);
    setLineWidth(5);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    //Title label
    QVBoxLayout *vbox_title = new QVBoxLayout;
    hbox->addLayout(vbox_title);
    m_lbl_title = new QLabel(title);
    vbox_title->addWidget(m_lbl_title);

    //Title font (big text)
    QFont fnt_title = m_lbl_title->font();
    fnt_title.setFamily("Monospace");
    fnt_title.setStyleHint(QFont::TypeWriter);
    fnt_title.setPointSize(14);
    fnt_title.setWeight(QFont::Bold);
    m_lbl_title->setFont(fnt_title);

    //Subtitle label (regular text, under title)
    m_lbl_subtitle = new QLabel();
    m_lbl_subtitle->setVisible(false);
    vbox_title->addWidget(m_lbl_subtitle);

    //Functions button "..."
    hbox->addStretch();
    m_btn_functions = new QPushButton("...");
    m_btn_functions->setFlat(true);
    m_btn_functions->setDisabled(true);
    hbox->addWidget(m_btn_functions);

    //Misc, mouse cursor style
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));
    setAutoFillBackground(true);
    setAttribute(Qt::WA_Hover);
    if (!disabled)
        setCursor(Qt::PointingHandCursor);

    //Default mode palette
    QPalette pal1 = QPalette(palette());
    pal1.setColor(QPalette::Window, QColor("lightGray"));
    pal1.setColor(QPalette::WindowText, QColor("black"));
    m_pal_default = pal1;
    m_pal_default_old = m_pal_default;

    //Active mode palette
    QPalette pal2 = QPalette(palette());
    pal2.setColor(QPalette::Window, QColor("darkGray"));
    pal2.setColor(QPalette::WindowText, QColor("white"));
    m_pal_hover = pal2;
    m_pal_active = pal2;

    //Paint
    setPalette(m_pal_default);

    //TODO setFocusPolicy(Qt::TabFocus) + focus event

}

void
ItemButton::resetBgColor()
{
    m_pal_default.setColor(QPalette::Window, m_pal_default_old.color(QPalette::Window));
    updateColor();
}

void
ItemButton::setBgColor(QColor color)
{
    m_pal_default.setColor(QPalette::Window, color);
    updateColor();
}

void
ItemButton::setFgColor(QColor color)
{
    m_pal_default.setColor(QPalette::WindowText, color);
    updateColor();
}

void
ItemButton::setHoverBgColor(QColor color)
{
    m_pal_hover.setColor(QPalette::Window, color);
    updateColor();
}

void
ItemButton::setHoverFgColor(QColor color)
{
    m_pal_hover.setColor(QPalette::WindowText, color);
    updateColor();
}

void
ItemButton::setSubtitle(QString text)
{
    m_lbl_subtitle->setVisible(true);
    m_lbl_subtitle->setText(text);
}

QString
ItemButton::signalName() const
{
    return m_sig_name;
}

void
ItemButton::setSignalName(QString name)
{
    m_sig_name = name;
}

QMenu*
ItemButton::menu()
{
    QMenu *menu = m_btn_functions->menu();
    if (!menu) menu = m_ctx_menu;
    return menu;
}

QMenu*
ItemButton::setButtonMenu(QMenu *menu)
{
    if (this->menu()) return this->menu();

    m_btn_functions->setMenu(menu);
    m_btn_functions->setDisabled(false);
    connect(menu, SIGNAL(triggered(QAction*)), SLOT(trigger(QAction*)));
    return menu;
}

QMenu*
ItemButton::setContextMenu(QMenu *menu)
{
    if (this->menu()) return this->menu();

    m_ctx_menu = menu;
    return menu;
}

QAction*
ItemButton::addAction(const QString &text)
{
    QMenu *cur_menu = menu();
    if (!cur_menu)
        cur_menu = setButtonMenu(new QMenu);

    return cur_menu->addAction(text);
}

void
ItemButton::updateColor()
{
    if (m_mouse_hover)
    {
        setPalette(m_pal_hover);
    }
    else
    {
        setPalette(m_pal_default);
    }

    update();
}

void
ItemButton::enterEvent(QEvent *event)
{
    if (m_disabled) return;
    m_mouse_hover = true;

    setPalette(m_pal_hover);

    emit entered();
}

void
ItemButton::leaveEvent(QEvent *event)
{
    if (m_disabled) return;
    m_mouse_hover = false;

    setPalette(m_pal_default);
    update();

    emit left();
}

void
ItemButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouse_pressed = true;
        //setFrameStyle(frameStyle() ^ QLabel::Raised);
        //setFrameStyle(QLabel::Sunken);
    }

    QFrame::mousePressEvent(event);
}

void
ItemButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouse_pressed = false;
        //setFrameStyle(QLabel::Raised);

        emit clicked();
        if (!m_sig_name.isEmpty()) emit clicked(m_sig_name);
    }

    QFrame::mouseReleaseEvent(event);
}

void
ItemButton::showContextMenu(const QPoint &pos)
{
    QMenu *ctx_menu = menu();
    if (!ctx_menu) return;

    ctx_menu->exec(mapToGlobal(pos));
}

void
ItemButton::trigger(QAction *action)
{
    emit triggered(action);
}

