#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

// 【关键】用“前向声明”代替包含 FFmpeg 头文件
struct AVPacket;

class PacketQueue {
public:
    PacketQueue() {}

    void push(AVPacket *pkt) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(pkt);
        cond_.notify_one();
    }

    AVPacket* pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            cond_.wait(lock);
        }
        AVPacket *pkt = queue_.front();
        queue_.pop();
        return pkt;
    }

    int size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    void clear();

private:
    std::queue<AVPacket*> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif // PACKETQUEUE_H
