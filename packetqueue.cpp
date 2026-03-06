#include "packetqueue.h"

// 【关键新增】在这里包含 FFmpeg 的头文件
// 因为 .cpp 文件不会被 moc 处理，所以在这里是安全的
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// packetqueue.cpp (由于PacketQueue的实现都在头文件里了，这个cpp文件可以只是为了包含头文件而存在)

void PacketQueue::clear()
{
    {
        std::lock_guard<std::mutex>lock(mutex_);
        while(!queue_.empty())
        {
            AVPacket *pkt = queue_.front();
            queue_.pop();
            av_packet_free(&pkt); // 正确释放FFmpeg包内存
        }
    }
}
