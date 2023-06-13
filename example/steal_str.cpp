#include "spdlog/spdlog.h"
#include "spdlog/sinks/callback_sink.h"
#include <vector>
#include <mutex>
#include <iostream>

// 这里实现怎么偷偷拿到 spdlog 的log信息, 不过拿不到前面的前缀, 因为format相关的函数在sink里面, 而这个sink里面相关的函数没暴露出来
class LogSystem {
private:
    static std::mutex mtx;
    static std::vector<std::string> logs_;
public:
    static void init() {
        auto default_logger = spdlog::default_logger();
        default_logger->sinks().clear();
        auto steal_str_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([&](const spdlog::details::log_msg & msg) {
            // auto logger = spdlog::get(std::string(msg.logger_name.data()));
            
            std::string str(msg.payload.data());
            std::lock_guard<std::mutex> lck(mtx);
            LogSystem::logs_.push_back(str);
        });
        default_logger->sinks().push_back(steal_str_sink);
    }
    static std::vector<std::string> logs() {
        return logs_;
    }
};

std::mutex LogSystem::mtx{};
std::vector<std::string> LogSystem::logs_{};

int main(){
    LogSystem::init();
    spdlog::info("hehe");
    spdlog::info("wuhu22");
    auto logs = LogSystem::logs();
    for (auto log : logs) {
        std::cout << log << std::endl;
    }
}