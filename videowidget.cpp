#include "videowidget.h"
#include <QPainter>
#include <algorithm> // 用于 std::min

VideoWidget::VideoWidget(QWidget *parent): QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: black;");
}

void VideoWidget::setImage(const QImage &image)
{
    m_image = image;
    this->update();
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (m_image.isNull()) {
        return;
    }

    // 【关键性能优化】：让画笔去平滑缩放，绝对不要用 m_image.scaled() 产生新图片！
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 获取视频原尺寸和窗口尺寸
    int imgW = m_image.width();
    int imgH = m_image.height();
    int winW = this->width();
    int winH = this->height();

    // 2. 计算保持比例的缩放系数
    double ratioW = (double)winW / imgW;
    double ratioH = (double)winH / imgH;
    double ratio = std::min(ratioW, ratioH);

    // 3. 计算最终画在屏幕上的宽高
    int newW = (int)(imgW * ratio);
    int newH = (int)(imgH * ratio);

    // 4. 计算居中的起始坐标 (x, y)
    int x = (winW - newW) / 2;
    int y = (winH - newH) / 2;

    // 5. 指定目标区域，直接将原图画上去！底层会自动将原图映射到这个区域，性能极高！
    QRect targetRect(x, y, newW, newH);
    painter.drawImage(targetRect, m_image);
}
