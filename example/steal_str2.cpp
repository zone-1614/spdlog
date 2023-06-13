// 想要获取带有前缀的log, 应该只能自己写一个sink了
// https://github.com/gabime/spdlog/issues/1300

#include "spdlog/spdlog.h"
#include "spdlog/sinks/callback_sink.h"
#include <mutex>
#include <iostream>
#include <vector>

// 
using zmesh_sink = spdlog::sinks::base_sink<std::mutex>;

class steal_str_sink : public zmesh_sink {
public: 
    // 不熟悉多线程, 所以不考虑性能, 直接拷贝一份
    // 由于spdlog的mutex_没有加mutable, 这里有两种选择
    // 第一种是 logs 不声明为 const
    // 第二种是使用const_cast
    std::vector<std::string> logs() { 
        std::lock_guard<std::mutex> lck(zmesh_sink::mutex_); 
        std::vector<std::string> v(logs_.begin(), logs_.end());
        return v;
    }

    void clear() {
        std::lock_guard<std::mutex> lck(zmesh_sink::mutex_);
        logs_.clear();
    }
protected:
    virtual void sink_it_(const spdlog::details::log_msg& msg) override {
        // 从basic_file_sink-inl.h里面抄的
        spdlog::memory_buf_t formatted;
        zmesh_sink::formatter_->format(msg, formatted);
        logs_.push_back(fmt::to_string(formatted));
    }

    virtual void flush_() override {}
private:
    std::vector<std::string> logs_;
};

int main() {
    auto logger = spdlog::default_logger();
    auto sinks = logger->sinks();
    sinks.push_back(std::make_shared<steal_str_sink>());
    spdlog::info("asdhehe");
    spdlog::info("second log");
    auto sink = sinks.back();
    auto streal_str_sink = std::dynamic_pointer_cast<steal_str_sink>(sink);
    auto logs = streal_str_sink->logs();
    for (auto log : logs) {
        std::cout << log << std::endl;
    }
}