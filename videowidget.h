#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

    // 【新增】供外部（主窗口）调用，传入解码后的图片
    void setImage(const QImage &image);

protected:
    // 【核心】重写绘图事件
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_image; // 保存当前要画的一帧
};

#endif // VIDEOWIDGET_H
