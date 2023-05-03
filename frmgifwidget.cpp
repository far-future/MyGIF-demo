#include "frmgifwidget.h"
#include "ui_frmgifwidget.h"
#include "gifwidget.h"

frmGifWidget::frmGifWidget(QWidget *parent) : QWidget(parent), ui(new Ui::frmGifWidget)
{
    ui->setupUi(this);

    /* 设置背景图片 */
    QPalette pal =this->palette();
    pal.setBrush(QPalette::Background,QBrush(QPixmap(":/image/0b6ac65c103853434102d3589913b07ecb8088be.png")));
    setPalette(pal);

    ui->pushButton->setFont(QFont("宋体",13));
    ui->pushButton->resize(QSize(162,60));
    ui->pushButton->setIcon(QIcon(":/image/wg.jpg"));
    ui->pushButton->setIconSize(QSize(25,25));
}

frmGifWidget::~frmGifWidget()
{
    delete ui;
}

void frmGifWidget::on_pushButton_clicked()
{
    //设置截图窗口置顶显示
    GifWidget::Instance()->setWindowFlags(GifWidget::Instance()->windowFlags() | Qt::WindowStaysOnTopHint);
    GifWidget::Instance()->show();
}
