#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <iostream>
#include "packetqueue.h"
#include "videowidget.h"
#include <atomic>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QTimer>

// 【============== 关键修改：在这里添加前向声明 ==============】
// 告诉编译器这些都是结构体类型，我们只使用它们的指针或实例，
// 完整的定义将在 .cpp 文件中提供。Moc 工具看到这个就能通过了。
struct AVCodecParameters;
struct AVCodec;
struct AVCodecContext;
struct AVRational;
// 【=======================================================】


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slot_receiveImage(QImage img);
    void on_btnPause_clicked();

     // 【新增】处理进度条拖动的槽函数
    void slot_sliderPressed(); //进度条被按下
    void slot_sliderMoved(int position);//进度条被滑动
    void slot_sliderReleased();//进度条被释放


signals:
    void sig_renderImage(QImage image);
private:
    Ui::MainWindow *ui;
    std::atomic<bool> m_isPause{false};// 【新增】全局暂停标志（原子操作，线程安全）
    QTimer *m_timer; // 【新增】用于定时刷新进度条的定时器


    AVCodecParameters* video_codec_params ;//解码器参数
    AVCodecParameters* audio_codec_params ;
    const AVCodec*  video_codec;//解码器
    const AVCodec*  audio_codec;
    AVCodecContext* video_codec_ctx ;// 分配解码器上下文
    AVCodecContext* audio_codec_ctx ;// 分配解码器上下文

    // 线程安全队列
    PacketQueue m_pktQueue;
    PacketQueue m_videoQueue;
    PacketQueue m_audioQueue;

    // 两个线程
    std::thread m_readThread;  // 生产者
    std::thread m_playThread;  // 消费者
    std::thread m_audioThread; // 【新增】音频消费者

    // 线程运行标志位（控制退出）
    bool m_isExit = false;

    // 具体的线程函数
    void readThreadFunc(); // 读取视频文件
    void video_playThreadFunc(); // 模拟播放
    void audio_playThreadFunc(); // 【新增】音频播放逻辑


    std::atomic<double> m_audioClock{0.0}; // 全局音频时钟 (秒)

    // 【============== 关键修改：从对象改为指针 ==============】
    AVRational *m_videoTimeBase;            // 视频时间基准 (指针)
    AVRational *m_audioTimeBase;            // 音频时间基准 (指针)
    // 【=======================================================】
    QIcon iconPlay;
    QIcon iconPause;

    std::atomic<bool> m_isSliderDragging{false}; // 用户是否正在拖拽
    std::atomic<bool> m_isSeek{false};           // 是否触发了跳转
    std::atomic<int64_t> m_seekTargetMs{-1};     // 跳转的目标时间(毫秒)

    // 【新增】通知消费者线程去刷新解码器内部缓存
    std::atomic<bool> m_videoNeedFlush{false};
    std::atomic<bool> m_audioNeedFlush{false};



};

#endif // MAINWINDOW_H
