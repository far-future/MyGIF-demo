//#pragma execution_character_set("utf-8") 指令指示编译器将源代码中
//的窄字符和窄字符串文本编码为可执行文件中的 UTF-8。 此输出编码与源文件的编码方式无关。
#pragma execution_character_set("utf-8")

#include "frmgifwidget.h"
#include <QApplication>
#include <QTextCodec>
#include <QIcon>
#include <QDebug>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

    QApplication a(argc, argv);
    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPixelSize(13);
    a.setFont(font);
    a.setWindowIcon(QIcon(":/image/gifwidget.ico"));

    /*要保证绝对的中文无乱码，最好设置以下

        2.1  设置开发cpp文件为UTF-8，（对于execution_character_set，它会根据设置改变cpp的编码）

        2.2 设置字符串编码（QT4）

        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
        QT5中已经默认编码为UTF-8了
    */

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    frmGifWidget w;
    w.resize(QSize(1000,612));
    w.setWindowTitle("GIF录屏工具");
    w.show();

    return a.exec();
}
