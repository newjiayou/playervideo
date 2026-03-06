#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include<QPainter>
// 必须包含 FFmpeg 头文件
// 必须用 extern "C" 包裹 FFmpeg 的头文件
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// 【新增】处理图像内存分配的头文件
#include <libavutil/imgutils.h>

// 【新增】处理图像格式转换 (YUV -> RGB) 的头文件
#include <libswscale/swscale.h>
#include <libswresample/swresample.h> // 【新增】音频
#include <libavutil/opt.h>            // 【新增】
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    iconPlay = QIcon(":/src/R.jpg");
    iconPause = QIcon(":/src/true.png");
        // 【新增】绑定进度条的拖动事件
    connect(ui->sliderProgress, &QSlider::sliderPressed, this, &MainWindow::slot_sliderPressed);
    connect(ui->sliderProgress, &QSlider::sliderMoved, this, &MainWindow::slot_sliderMoved);
    connect(ui->sliderProgress, &QSlider::sliderReleased, this, &MainWindow::slot_sliderReleased);




    connect(this, &MainWindow::sig_renderImage, this, &MainWindow::slot_receiveImage);
    // UI上记得放一个按钮，objectName 叫 btnStart

    m_timer = new QTimer (this);
    connect(m_timer,&QTimer::timeout,this,[this]()
            {
                // m_audioClock 是你原本代码里计算的当前播放时间（秒）
                // 我们把它乘 1000 换算成毫秒，设置给进度条
                if (m_audioClock.load() > 0.0 && !m_isSeek) {
                    ui->sliderProgress->setValue(m_audioClock.load() * 1000);
                }
            });

    // 【新增】为指针成员分配内存
    m_videoTimeBase = new AVRational();
    m_audioTimeBase = new AVRational();
}

MainWindow::~MainWindow()
{
    // 关闭窗口时，要优雅地结束线程
    m_isExit = true; // 1. 告诉线程该停了

    // 2. 等待线程真正结束
    if (m_readThread.joinable()) m_readThread.join();
    if (m_playThread.joinable()) m_playThread.join();
    if (m_audioThread.joinable()) m_audioThread.join();

    // 【新增】释放内存
    delete m_videoTimeBase;
    delete m_audioTimeBase;
    m_videoTimeBase = nullptr;
    m_audioTimeBase = nullptr;

    delete ui;
}

// ==========================================
// 1. 点击按钮，启动双线程
// =========================================
void MainWindow::slot_receiveImage(QImage img)
{
    ui->screenwidget->setImage(img);
}

// ==========================================
// 2. 生产者线程：读取视频 -> 放入队列
// ==========================================
void MainWindow::readThreadFunc()
{
    // ---------------- 前置准备 ----------------
    const char *filepath = "C:\\Users\\86139\\Documents\\video/test.mp4";
    AVFormatContext *fmtCtx = avformat_alloc_context();

    if (avformat_open_input(&fmtCtx, filepath, nullptr, nullptr) != 0) {
        qDebug() << "Could not open file";
        return;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        qDebug() << "Could not find stream info";
        return;
    }

    //获取总时间
    if (fmtCtx->duration != AV_NOPTS_VALUE) {
        // 算出总共有多少毫秒
        int total_duration_ms = (fmtCtx->duration* 1000 / AV_TIME_BASE) ;

        // 注意：UI 操作必须回到主线程！这里用 invokeMethod 投递到主线程执行
        QMetaObject::invokeMethod(this,[this, total_duration_ms]() {
            ui->sliderProgress->setRange(0, total_duration_ms);
            ui->sliderProgress->setEnabled(true);
        });
    }

    // 寻找最优流和对应的解码器
    int video_stream_index = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);
    int audio_stream_index = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &audio_codec, 0);

    // ---------------- 1. 独立处理视频线 ----------------
    if (video_stream_index >= 0 && video_codec) {
        // 【修改】使用指针解引用赋值
        *m_videoTimeBase = fmtCtx->streams[video_stream_index]->time_base;

        video_codec_params = fmtCtx->streams[video_stream_index]->codecpar;
        video_codec_ctx = avcodec_alloc_context3(video_codec); // 分配视频上下文
        avcodec_parameters_to_context(video_codec_ctx, video_codec_params); // 拷贝参数

        // 打开视频解码器
        if (avcodec_open2(video_codec_ctx, video_codec, nullptr) < 0) {
            std::cerr << "Could not open video_codec." << std::endl;
            return; // 视频打开失败，一般就直接退出了，因为播放器主要是看画面
        }
    }
    else {
        printf("错误：未找到视频流或视频解码器\n");
        return;
    }

    // ---------------- 2. 独立处理音频线 ----------------
    // 只有在真的找到了音频流和音频解码器时，才去执行分配和打开操作！
    if (audio_stream_index >= 0 && audio_codec) {
        // 【修改】使用指针解引用赋值
        *m_audioTimeBase = fmtCtx->streams[audio_stream_index]->time_base;

        audio_codec_params = fmtCtx->streams[audio_stream_index]->codecpar;
        audio_codec_ctx = avcodec_alloc_context3(audio_codec); // 分配音频上下文
        avcodec_parameters_to_context(audio_codec_ctx, audio_codec_params); // 拷贝参数

        // 打开音频解码器
        if (avcodec_open2(audio_codec_ctx, audio_codec, nullptr) < 0) {
            std::cerr << "警告：Could not open audio_codec." << std::endl;
            // 注意：音频打开失败，我们可以选择不 return，继续让程序无声播放视频
        }
    } else {
        printf("提示：该文件无音频流，将进行无声播放\n");
    }

    //------确保解码器运行之后 打开消费者线程------------
    if (video_codec_ctx != nullptr) {
        // 确保之前没有运行（防止异常重启）
        if (!m_playThread.joinable()) {
            m_playThread = std::thread(&MainWindow::video_playThreadFunc, this);
            qDebug() << "Play Thread started safely!";
        }
    }

    if (audio_codec_ctx != nullptr) {
        if (!m_audioThread.joinable()) {
            m_audioThread = std::thread(&MainWindow::audio_playThreadFunc, this);
            qDebug() << "Audio Thread started!";
        }
    }
    // -------- 到这里，两个解码器（或只有一个视频解码器）已经完美就绪 --------

    AVPacket *pkt = av_packet_alloc(); // 作为一个临时容器

    while (!m_isExit) {
        // 【新增 1】：处理视频跳转逻辑（必须放在暂停判断的最前面！）
        // 【终极修复 1：双重保险跳转算法】
        if (m_isSeek) {
            int64_t target_ms = m_seekTargetMs.load();
            double target_sec = target_ms / 1000.0;

            // 方案A：优先使用 FFmpeg 默认流(-1) 和全局微秒进行跳转（兼容90%的文件，尤其是MP4）
            int64_t seek_pos_micro = target_ms * 1000;
            int ret = av_seek_frame(fmtCtx, -1, seek_pos_micro, AVSEEK_FLAG_BACKWARD);

            // 方案B：如果奇葩文件导致方案A失败，立刻降级用“视频流时间基”进行跳转
            if (ret < 0) {
                int64_t seek_target_v = target_sec / av_q2d(*m_videoTimeBase);
                ret = av_seek_frame(fmtCtx, video_stream_index, seek_target_v, AVSEEK_FLAG_BACKWARD);
            }

            // 如果两次中有一次成功了
            if (ret >= 0) {
                m_videoQueue.clear();
                m_audioQueue.clear();

                AVPacket *flush_pkt_v = av_packet_alloc();
                flush_pkt_v->stream_index = -1;
                m_videoQueue.push(flush_pkt_v);

                AVPacket *flush_pkt_a = av_packet_alloc();
                flush_pkt_a->stream_index = -1;
                m_audioQueue.push(flush_pkt_a);

                m_audioClock = target_sec;
            }

            m_isSeek = false;
            m_seekTargetMs = -1;
            continue;
        }
        // 【新增 1】：如果按下暂停，就让线程睡大觉，不再读包
        if (m_isPause)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }


        // 【可选】队列控制机制：如果队列里堆积了太多包，休眠等待消费者消费
        // 防止解码过慢、读取过快导致内存爆满
        if ((m_videoQueue.size() > 200 && m_audioQueue.size() > 200) ||
            m_videoQueue.size() > 1000 || m_audioQueue.size() > 1000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0) {
            qDebug() << "End of file or read error";
            break; // 读完了
        }

        // 【修复点3】判断这个包是视频包还是音频包，分开塞入不同的队列
        if (pkt->stream_index == video_stream_index) {
            AVPacket *clonePkt = av_packet_alloc();
            av_packet_move_ref(clonePkt, pkt); // 优化：用 move_ref 直接转移数据所有权，省去拷贝再解引用的开销
            m_videoQueue.push(clonePkt);       // 放入视频队列
        }
        else if (pkt->stream_index == audio_stream_index) {
            AVPacket *clonePkt = av_packet_alloc();
            av_packet_move_ref(clonePkt, pkt);
            m_audioQueue.push(clonePkt);       // 放入音频队列
        }
        else {
            // 其他包（如字幕等），暂不处理，直接丢弃引用
            av_packet_unref(pkt);
        }
    }

    // 5. ======== 资源释放 ========
    av_packet_free(&pkt);
    avformat_close_input(&fmtCtx);
    qDebug() << "Read thread finished.";
}

// ==========================================
// 3. 消费者线程：从队列取 -> 打印 -> 释放
// ==========================================
void MainWindow::video_playThreadFunc()
{
    // 1. 获取转换前信息 从解码器上下文获取
    int width = video_codec_ctx->width;
    int height = video_codec_ctx->height;
    AVPixelFormat in_pix_fmt = video_codec_ctx->pix_fmt;

    // Qt 的 Format_RGB32 对应的 FFmpeg 格式通常是 AV_PIX_FMT_BGRA
    AVPixelFormat out_pix_fmt = AV_PIX_FMT_BGRA;

    // 2. 分配储存空间
    AVFrame *frame = av_frame_alloc();
    AVFrame *frameRGB = av_frame_alloc();
    int numB = av_image_get_buffer_size(out_pix_fmt, width, height, 1);
    uint8_t *out_buffer = (uint8_t *)av_malloc(numB * sizeof(uint8_t));

    // 3. 将地址与储存空间关联上
    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer, out_pix_fmt, width, height, 1);

    // 4. 初始化转化器
    SwsContext *sws_ctx = sws_getContext(
        width, height, in_pix_fmt,   // 输入
        width, height, out_pix_fmt,  // 输出
        SWS_BILINEAR, nullptr, nullptr, nullptr
        );

    while (!m_isExit) {
        if (m_isPause) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // 1. 从队列取包
        AVPacket *pkt = m_videoQueue.pop();
        if (!pkt) continue;
        if (pkt->stream_index == -1) {
            avcodec_flush_buffers(video_codec_ctx); // 清空解码器肚子里的历史残余帧
            av_packet_free(&pkt);                   // 销毁假包
            continue;                               // 立刻去拿下一个真正的新包
        }



        // 2. 将包丢到解码器
        int ret = avcodec_send_packet(video_codec_ctx, pkt);
        if (ret == 0) {
            while (avcodec_receive_frame(video_codec_ctx, frame) == 0) {

                // --- 【音视频同步与性能优化核心】 ---

                // A. 获取当前帧的时间戳
                int64_t pts = frame->best_effort_timestamp;
                if (pts == AV_NOPTS_VALUE) pts = frame->pts;

                double video_pts = 0.0;
                if (pts != AV_NOPTS_VALUE) {
                    // 【修改】使用指针解引用
                    video_pts = pts * av_q2d(*m_videoTimeBase);
                }

                bool drop_frame = false; // 默认不丢帧

                // B. 计算与音频时钟的差距
                if (m_audioClock.load() > 0.0) {
                    double diff = video_pts - m_audioClock.load();

                    // 情况1：视频太快了 (diff > 0.02) -> 睡觉等待
                    if (diff > 0.02) {
                        int wait_ms = (int)(diff * 1000);
                        // 【安全防护】防止 wait_ms 异常过大导致界面卡死
                        if (wait_ms > 1000) wait_ms = 1000;

                        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
                    }
                    // 情况2：视频太慢了 (diff < -0.2) -> 标记丢帧
                    // 【优化】：阈值设为 -0.2 (200ms)，给 UI 线程更多喘息机会
                    else if (diff < -0.2) {
                        drop_frame = true;
                    }
                }
                else {
                    // 音频没启动时的默认限速
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }

                // C. 【终极优化】：只有不丢帧的时候，才做耗时的 sws_scale 和 copy
                if (!drop_frame) {
                    // 执行格式转换 (耗时操作)
                    sws_scale(sws_ctx,
                              (const uint8_t* const*)frame->data, frame->linesize,
                              0, height,
                              frameRGB->data, frameRGB->linesize);

                    // 构造 QImage
                    QImage img(out_buffer, width, height, QImage::Format_RGB32);

                    // 发送信号 (耗时操作：因为涉及内存拷贝)
                    emit sig_renderImage(img.copy());
                }
            }
        }
        av_packet_free(&pkt);
    }

    // 资源释放
    av_free(out_buffer);
    av_frame_free(&frameRGB);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    qDebug() << "Play thread finished.";
}
void MainWindow::audio_playThreadFunc()
{
    // 1. 初始化 FFmpeg 音频重采样器 (SwrContext)
    SwrContext *swr_ctx = swr_alloc();

    // 配置输入参数（从解码器取）
    av_opt_set_chlayout(swr_ctx, "in_chlayout",  &audio_codec_ctx->ch_layout, 0);
    av_opt_set_int(swr_ctx,      "in_sample_rate",audio_codec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx,"in_sample_fmt", audio_codec_ctx->sample_fmt, 0);

    // 配置输出参数（固定为通用标准：双声道，44100Hz，16位整数）
    AVChannelLayout out_ch_layout;
    av_channel_layout_default(&out_ch_layout, 2); // 2 = 双声道
    av_opt_set_chlayout(swr_ctx, "out_chlayout",  &out_ch_layout, 0);
    av_opt_set_int(swr_ctx,      "out_sample_rate",44100, 0);
    av_opt_set_sample_fmt(swr_ctx,"out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    swr_init(swr_ctx);

    // 2. 初始化 Qt6 的音频播放器 (QAudioSink)
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioSink *audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);
    QIODevice *audioDevice = audioSink->start(); // 返回一个可写入数据的设备指针

    AVFrame *frame = av_frame_alloc();

    while (!m_isExit) {
        if (m_isPause) {
            // 如果声卡正在运行，告诉声卡暂停物理播放
            if (audioSink->state() == QAudio::ActiveState || audioSink->state() == QAudio::IdleState) {
                audioSink->suspend();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue; // 跳过后续的解码和写入
        } else {
            // 如果处于暂停状态，告诉声卡恢复物理播放
            if (audioSink->state() == QAudio::SuspendedState) {
                audioSink->resume();
            }
        }
        AVPacket *pkt = m_audioQueue.pop();
        if (!pkt) continue;

        if (pkt->stream_index == -1) {
            avcodec_flush_buffers(audio_codec_ctx); // 清空音频解码器残余

            // 【关键】：立刻停止并重启声卡，抛弃底层积压的“滋滋”旧声音
            audioSink->stop();
            audioDevice = audioSink->start();

            av_packet_free(&pkt);
            continue;
        }



        if (avcodec_send_packet(audio_codec_ctx, pkt) == 0) {
            while (avcodec_receive_frame(audio_codec_ctx, frame) == 0) {

                // 【核心 1：更新全局音频时钟】
                // PTS 乘以 时间基，得出当前音频播放到了多少秒！
                if (frame->pts != AV_NOPTS_VALUE) {
                    // 【修改】使用指针解引用
                    double current_pts_time = frame->pts * av_q2d(*m_audioTimeBase);

                    // 2. 计算声卡底层缓冲区里，还有多少数据没有真正播出去
                    int bytes_per_sec = 44100 * 2 * 2; // 采样率44100 * 双声道(2) * 16bit(2字节)
                    int queued_bytes = audioSink->bufferSize() - audioSink->bytesFree();

                    // 3. 换算成排队的延迟时间（秒）
                    double hardware_delay = (double)queued_bytes / bytes_per_sec;

                    // 4. 【核心精进】真正的物理世界声音时间 = 理论时间 - 正在排队的时间！
                    m_audioClock = current_pts_time - hardware_delay;
                }

                // 【核心 2：重采样音频数据】
                // 计算重采样后会有多少个采样点
                int out_samples = swr_get_out_samples(swr_ctx, frame->nb_samples);
                // 分配输出内存 (大小 = 采样点数 * 2声道 * 2字节(16bit))
                uint8_t *out_buffer = (uint8_t *)av_malloc(out_samples * 2 * 2);

                // 执行转换
                int real_out_samples = swr_convert(swr_ctx, &out_buffer, out_samples,
                                                   (const uint8_t **)frame->data, frame->nb_samples);

                int out_size = real_out_samples * 2 * 2;

                // 【核心 3：喂给声卡播放】
                // write 函数在声卡缓冲区满时会自动阻塞，这就起到了控制整个播放速度的作用！
                if (out_size > 0 && audioDevice) {
                    while (audioSink->bytesFree() < out_size && !m_isExit) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // 睡 2 毫秒再问
                    }

                    // 等声卡有空位了，再把数据塞给它
                    audioDevice->write((const char*)out_buffer, out_size);
                }

                av_free(out_buffer);
            }
        }
        av_packet_free(&pkt);
    }

    // 清理资源
    av_frame_free(&frame);
    swr_free(&swr_ctx);
    audioSink->stop();
    delete audioSink;
    qDebug() << "Audio thread finished.";
}

void MainWindow::on_btnPause_clicked()
{
    // 1. 如果读取线程还没有启动过，说明是【第一次点击】，需要启动视频
    if (!m_readThread.joinable()) {
        m_isExit = false;
        m_isPause = false; // 确保初始状态不是暂停

        // 启动读取线程（生产者）
        m_readThread = std::thread(&MainWindow::readThreadFunc, this);
        m_timer->start(100);

        // 变成“暂停”图标，因为现在正在播放
        ui->btnPause->setIcon(iconPlay);
        qDebug() << "Threads started!";
    }
    // 2. 如果线程已经启动了，说明是在【播放中】，需要切换 暂停/恢复 状态
    else {
        m_isPause = !m_isPause; // 翻转暂停状态

        if (m_isPause) {
            ui->btnPause->setIcon(iconPause);  // 暂停了，显示“播放”图标提示用户点击播放
        } else {
            ui->btnPause->setIcon(iconPlay);// 恢复了，显示“暂停”图标
        }
    }
}

void MainWindow::slot_sliderPressed()
{
    m_isSliderDragging = true; // 按下时暂停 timer 自动推走滑块
}

void MainWindow::slot_sliderMoved(int position)
{
    // 拖拽过程中（可选）：你可以在这里把 position (毫秒) 转换成字符串显示在UI上的Label里
}

void MainWindow::slot_sliderReleased()
{
    m_isSliderDragging = false;
    m_seekTargetMs = ui->sliderProgress->value(); // 获取释放时的目标位置
    m_isSeek = true; // 开启跳转标志，等待读取线程去执行
}
