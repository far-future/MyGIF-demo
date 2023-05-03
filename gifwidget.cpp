//#pragma execution_character_set("utf-8") 指令指示编译器将源代码中
//的窄字符和窄字符串文本编码为可执行文件中的 UTF-8。 此输出编码与源文件的编码方式无关。
#pragma execution_character_set("utf-8")

#include "gifwidget.h"
#include "qmutex.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qevent.h"
#include "qstyle.h"
#include "qpixmap.h"
#include "qtimer.h"
#include "qdatetime.h"
#include "qapplication.h"
#include "qdesktopservices.h"
#include "qfiledialog.h"
#include "qurl.h"
#include "qtextcodec.h"
#include "qdebug.h"

//解决跨平台的版本问题
//早期的版本中，利用qApp->desktop()->availableGeometry()获取桌面的几何信息，
//QDesktopWidget废弃后，桌面的信息主要通过QGuiApplication::primaryScreen()->geometry()获取信息。
//QGuiApplication::primaryScreen()等价于前期的QDesktopWidget::primaryScreen()。
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include "qscreen.h"
//获取设备屏幕大小
#define deskGeometry qApp->primaryScreen()->geometry()
//获取可用桌面大小
#define deskGeometry2 qApp->primaryScreen()->availableGeometry()
#else
//旧版本
#include "qdesktopwidget.h"
#define deskGeometry qApp->desktop()->geometry()
#define deskGeometry2 qApp->desktop()->availableGeometry()
#endif

QScopedPointer<GifWidget> GifWidget::self;
GifWidget *GifWidget::Instance()
{
    //两个嵌套的 if 来判断单例对象是否为空的操作就叫做双重检查锁定。
    //解决单例-懒汉模式下的线程安全问题 使用互斥锁
    if (self.isNull()) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (self.isNull()) {
            self.reset(new GifWidget);
        }
    }

    return self.data();
}

GifWidget::GifWidget(QWidget *parent) : QDialog(parent)
{
    this->initControl();
    this->initForm();
}

bool GifWidget::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint mousePoint;
    static bool mousePressed = false;

    //static_cast：带安全检查的类型转换，用于替代强制类型转换，
    //此转换会在编译时进行类型检查，而强制转换不会。
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->type() == QEvent::MouseButtonPress) {
        if (mouseEvent->button() == Qt::LeftButton) {
            mousePressed = true;
            /*globalPos()，给出的坐标信息是相对于桌面的，即以桌面左上角为原点。
             * pos()，是相对于窗口的，以窗口左上角为原点（去除边框）。
             * 即pos()给出的是一个相对位置坐标。而globalPos(),给出的是一个绝对坐标。
             * */
            //捕捉到鼠标点击事件的全局精准坐标
            mousePoint = mouseEvent->globalPos() - this->pos();
            return true;
        }
    } else if (mouseEvent->type() == QEvent::MouseButtonRelease) {
        mousePressed = false;
        return true;
    } else if (mouseEvent->type() == QEvent::MouseMove) {
        if (mousePressed) {
            //捕捉到鼠标事件在点击不放的情况下的移动
            this->move(mouseEvent->globalPos() - mousePoint);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

//重写resizeEvent(QResizeEvent *event)函数，使得Qt界面的控件随窗口的变化而变化
void GifWidget::resizeEvent(QResizeEvent *e)
{
    //拉动右下角改变大小自动赋值
    txtWidth->setText(QString::number(widgetMain->width()));
    txtHeight->setText(QString::number(widgetMain->height()));
    QDialog::resizeEvent(e);
}

void GifWidget::paintEvent(QPaintEvent *)
{
    int width = txtWidth->text().toInt();
    int height = txtHeight->text().toInt();
    rectGif = QRect(borderWidth, widgetTop->height(), width - (borderWidth * 2), height);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(this->rect(), 5, 5);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(rectGif, Qt::SolidPattern);
}

int GifWidget::getBorderWidth() const
{
    return this->borderWidth;
}

QColor GifWidget::getBgColor() const
{
    return this->bgColor;
}

void GifWidget::initControl()
{
    this->setObjectName("GifWidget");
    this->resize(800, 600);
    //右下角的窗口尺寸调整符号，右下角的小黑三角。
    //提供两个方法isSizeGripEnabled()、setSizeGripEnabled(bool)设置其是否显示。
    this->setSizeGripEnabled(true);
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    //setContentsMargins作用：设置左侧、顶部、右侧和底部边距，以便在布局周围使用。
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    //ObjectName主要是用于外界来访问内部的控件成员的，
    //如果外界不需要访问这个成员，则理论上无需设置它的ObjectName。
    verticalLayout->setObjectName("verticalLayout");
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    widgetTop = new QWidget(this);
    widgetTop->setObjectName("widgetTop");
    widgetTop->setMinimumSize(QSize(0, 35));
    widgetTop->setMaximumSize(QSize(16777215, 35));

    QHBoxLayout *layoutTop = new QHBoxLayout(widgetTop);
    layoutTop->setSpacing(0);
    layoutTop->setContentsMargins(11, 11, 11, 11);
    layoutTop->setObjectName("layoutTop");
    layoutTop->setContentsMargins(0, 0, 0, 0);

    QPushButton *btnIcon = new QPushButton(widgetTop);
    btnIcon->setObjectName("btnIcon");

    /*
     * QSizePolicy类是一个描述布局水平和垂直方向调整策略的属性。
     * 大小策略会影响布局引擎处理部件的方式，部件加入布局以后，会返回一个QSizePolicy，
     * 描述了其水平和垂直方向的大小策略。可以通过QWidget::sizePolicy属性为特定部件设置大小策略。
     * QSizePolicy包含了两个独立的QSizePolicy::Policy值和两个缩放因子，一个描述了部件水平方向
     * 上的大小策略，另一个描述了垂直方向上的大小策略。它还包含一个标志表明高度和宽度是否与首选大小有关。
     * 水平和垂直方向的大小策略可以在构造函数中设置，也可以通过setHorizontalPolicy()和
     * setVerticalPolicy()函数改变。缩放因子可以使用setHorizontalStretch()和
     * setVerticalStretch()函数设置。setHeightForWidth()函数的标志表示部件的
     * 缺省大小（sizeHint()）是否是width-dependent（例如：菜单栏或自动换行标签） 。
     * 可以使用horizontalPolicy()、verticalPolicy()、horizontalStretch()和
     * verticalStretch()函数来返回当前的大小策略和缩放因子。另外，使用
     * transpose()函数可以互换水平和垂直的大小策略和缩放因子。
     * hasHeightForWidth()函数返回了当前状态表示的大小依赖性。
     * */

    //QSizePolicy::Minimum缺省大小是最小值，并且是充分的。
    //部件允许扩展，但是并不倾向扩展（例如：水平方向上的按钮），不能比缺省大小提供的大小更小。
    //QSizePolicy::Expanding缺省大小是合理的大小，但部件允许缩小并且可用。
    //部件可以利用额外的空间，因此它将会得到尽可能多的空间（例如：水平方向上的滑块）。
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    //void QSizePolicy::setHorizontalStretch(int stretchFactor)
    //设置水平大小策略的缩放因子，取值范围[0,255]。
    sizePolicy.setHorizontalStretch(0);

    //void QSizePolicy::setVerticalStretch(int stretchFactor)
    //设置垂直大小策略的缩放因子，取值范围[0,255]。
    sizePolicy.setVerticalStretch(0);

    //void QSizePolicy::setHeightForWidth(bool dependent)
    //设置标志判断窗口部件的首选高度是否依赖于它的宽度。
    sizePolicy.setHeightForWidth(btnIcon->sizePolicy().hasHeightForWidth());

    btnIcon->setSizePolicy(sizePolicy);
    btnIcon->setMinimumSize(QSize(35, 0));
    //去掉按钮的边框，让pushbutton按钮跟背景色融为一体，可以用函数QPushbutton::setFlat(true)来实现
    btnIcon->setFlat(true);
    layoutTop->addWidget(btnIcon);

    QLabel *labTitle = new QLabel(widgetTop);
    labTitle->setObjectName("labTitle");
    layoutTop->addWidget(labTitle);

    QSpacerItem *horizontalSpacer = new QSpacerItem(87, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layoutTop->addItem(horizontalSpacer);

    QPushButton *btnClose = new QPushButton(widgetTop);
    btnClose->setObjectName("btnClose");
    sizePolicy.setHeightForWidth(btnClose->sizePolicy().hasHeightForWidth());
    btnClose->setSizePolicy(sizePolicy);
    btnClose->setMinimumSize(QSize(35, 0));
    btnClose->setFocusPolicy(Qt::NoFocus);
    btnClose->setFlat(true);
    layoutTop->addWidget(btnClose);
    verticalLayout->addWidget(widgetTop);

    widgetMain = new QWidget(this);
    widgetMain->setObjectName("widgetMain");
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    //setHeightForWidth设置保持相对的宽高比
    sizePolicy1.setHeightForWidth(widgetMain->sizePolicy().hasHeightForWidth());
    widgetMain->setSizePolicy(sizePolicy1);
    verticalLayout->addWidget(widgetMain);

    widgetBottom = new QWidget(this);
    widgetBottom->setObjectName("widgetBottom");
    widgetBottom->setMinimumSize(QSize(0, 45));
    widgetBottom->setMaximumSize(QSize(16777215, 45));

    QHBoxLayout *layoutBottom = new QHBoxLayout(widgetBottom);
    layoutBottom->setSpacing(6);
    layoutBottom->setContentsMargins(11, 11, 11, 11);
    layoutBottom->setObjectName("layoutBottom");
    layoutBottom->setContentsMargins(9, 9, -1, -1);

    QLabel *labFps = new QLabel(widgetBottom);
    labFps->setObjectName("labFps");
    layoutBottom->addWidget(labFps);

    txtFps = new QLineEdit(widgetBottom);
    txtFps->setObjectName("txtFps");
    txtFps->setMaximumSize(QSize(50, 16777215));
    //setAlignment设置对齐方式为:AlignCenter水平方向居中
    txtFps->setAlignment(Qt::AlignCenter);
    layoutBottom->addWidget(txtFps);

    QLabel *labWidth = new QLabel(widgetBottom);
    labWidth->setObjectName("labWidth");
    layoutBottom->addWidget(labWidth);

    txtWidth = new QLineEdit(widgetBottom);
    txtWidth->setObjectName("txtWidth");
    txtWidth->setEnabled(true);
    txtWidth->setMaximumSize(QSize(50, 16777215));
    txtWidth->setAlignment(Qt::AlignCenter);
    layoutBottom->addWidget(txtWidth);

    QLabel *labHeight = new QLabel(widgetBottom);
    labHeight->setObjectName("labHeight");
    layoutBottom->addWidget(labHeight);

    txtHeight = new QLineEdit(widgetBottom);
    txtHeight->setObjectName("txtHeight");
    txtHeight->setEnabled(true);
    txtHeight->setMaximumSize(QSize(50, 16777215));
    txtHeight->setAlignment(Qt::AlignCenter);
    layoutBottom->addWidget(txtHeight);

    labStatus = new QLabel(widgetBottom);
    labStatus->setObjectName("labStatus");
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(labStatus->sizePolicy().hasHeightForWidth());
    labStatus->setSizePolicy(sizePolicy2);
    labStatus->setAlignment(Qt::AlignCenter);
    layoutBottom->addWidget(labStatus);

    btnStart = new QPushButton(widgetBottom);
    btnStart->setObjectName("btnStart");
    sizePolicy.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
    btnStart->setSizePolicy(sizePolicy);
    layoutBottom->addWidget(btnStart);
    verticalLayout->addWidget(widgetBottom);

    labTitle->setText("GIF录屏工具");
    labFps->setText("帧率");
    labWidth->setText("宽度");
    labHeight->setText("高度");
    btnStart->setText("开始录屏");
    btnStart->setIcon(QIcon(":/image/wg.jpg"));
    this->setWindowTitle(labTitle->text());

    //设置录屏界面的样式图标以及关闭图标
    //btnIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    btnIcon->setIcon(QIcon(":/image/7182d5628535e5ddc927968b61c6a7efce1b621e.png"));
    btnClose->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));

    connect(btnClose, SIGNAL(clicked(bool)), this, SLOT(closeAll()));
    connect(btnStart, SIGNAL(clicked(bool)), this, SLOT(record()));
    connect(txtWidth, SIGNAL(editingFinished()), this, SLOT(resizeForm()));
    connect(txtHeight, SIGNAL(editingFinished()), this, SLOT(resizeForm()));
}

void GifWidget::initForm()
{
    borderWidth = 3;
    //玉米花蓝色 十六进制代码:＃6495ED  十进制代码:(100,149,237)
    bgColor = QColor(100,149,237);

    fps = 10;
    txtFps->setText(QString::number(fps));
    gifWriter = 0;

    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(saveImage()));

    // setAttribute(Qt::WA_TranslucentBackground,true); 表示窗口小部件应具有半透明背景，
    //即窗口小部件的任何非不透明区域将是半透明的（因为窗口小部件将具有Alpha通道。）
    this->setAttribute(Qt::WA_TranslucentBackground);
    //Qt::FramelessWindowHint无边框化，移动，大小调整
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->installEventFilter(this);

    QStringList qss;
    qss.append("QLabel{color:#ffffff;}");
    qss.append("#btnClose,#btnIcon{border:none;border-radius:0px;}");
    qss.append("#btnClose:hover{background-color:#ff0000;}");
    qss.append("#btnClose{border-top-right-radius:5px;}");
    qss.append("#labTitle{font:bold 16px;}");
    qss.append("#labStatus{font:15px;}");
    this->setStyleSheet(qss.join(""));
}

void GifWidget::saveImage()
{
    if (!gifWriter) {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QScreen *screen = QApplication::primaryScreen();
    QPixmap pix = screen->grabWindow(0, x() + rectGif.x(), y() + rectGif.y(), rectGif.width(), rectGif.height());
    QImage image = pix.toImage().convertToFormat(QImage::Format_RGBA8888);
#else
    //由于qt4没有RGBA8888,采用最接近RGBA8888的是ARGB32,颜色会有点偏差
    QPixmap pix = QPixmap::grabWindow(0, x() + rectGif.x(), y() + rectGif.y(), rectGif.width(), rectGif.height());
    QImage image = pix.toImage().convertToFormat(QImage::Format_ARGB32);
#endif

    gif.GifWriteFrame(gifWriter, image.bits(), rectGif.width(), rectGif.height(), fps);
    count++;
    labStatus->setText(QString("正在录制 第 %1 帧").arg(count));
}

void GifWidget::record()
{
    if (btnStart->text() == "开始录屏") {
        if (0 != gifWriter) {
            delete gifWriter;
            gifWriter = 0;
        }

        //先弹出文件保存对话框
        //fileName = qApp->applicationDirPath() + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss.gif");
        fileName = QFileDialog::getSaveFileName(this, "选择保存位置", qApp->applicationDirPath() + "/", "gif图片(*.gif)");
        if (fileName.isEmpty()) {
            return;
        }

        int width = txtWidth->text().toInt();
        int height = txtHeight->text().toInt();
        fps = txtFps->text().toInt();

#ifdef Q_OS_WIN
        //windows上需要先转码
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
        QTextCodec *code = QTextCodec::codecForName("utf-8");
#else
        QTextCodec *code = QTextCodec::codecForName("gbk");
#endif
        const char *name = code->fromUnicode(fileName).constData();
#else
        const char *name = fileName.toUtf8().constData();
#endif

        gifWriter = new Gif::GifWriter;
        bool ok = gif.GifBegin(gifWriter, name, width, height, fps);
        if (!ok) {
            delete gifWriter;
            gifWriter = 0;
            return;
        }

        count = 0;
        labStatus->setText("开始录制...");
        btnStart->setText("停止录屏");
        btnStart->setIcon(QIcon(":/image/wg.jpg"));
        //延时启动
        timer->setInterval(1000 / fps);
        QTimer::singleShot(1000, timer, SLOT(start()));
        //saveImage();
    } else {
        timer->stop();
        gif.GifEnd(gifWriter);

        delete gifWriter;
        gifWriter = 0;

        labStatus->setText(QString("录制完成 共 %1 帧").arg(count));
        btnStart->setText("开始录屏");
        //利用QDesktopServices打开本地文件或文件夹
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

void GifWidget::closeAll()
{
    if (0 != gifWriter) {
        delete gifWriter;
        gifWriter = 0;
    }

    this->close();
}

void GifWidget::resizeForm()
{
    int width = txtWidth->text().toInt();
    int height = txtHeight->text().toInt();
    this->resize(width, height + widgetTop->height() + widgetBottom->height());
}

void GifWidget::setBorderWidth(int borderWidth)
{
    if (this->borderWidth != borderWidth) {
        this->borderWidth = borderWidth;
        this->update();
    }
}

void GifWidget::setBgColor(const QColor &bgColor)
{
    if (this->bgColor != bgColor) {
        this->bgColor = bgColor;
        this->update();
    }
}
