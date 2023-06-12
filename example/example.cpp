//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog usage example

#include <cstdio>
#include <chrono>

void load_levels_example();
void stdout_logger_example();
void basic_example();
void rotating_example();
void daily_example();
void callback_example();
void async_example();
void binary_example();
void vector_example();
void stopwatch_example();
void trace_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void syslog_example();
void udp_example();
void custom_flags_example();
void file_events_example();
void replace_default_logger_example();

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types

int main(int, char *[])
{
    // Log levels can be loaded from argv/env using "SPDLOG_LEVEL"
    load_levels_example();

    // spd 里面用了 fmt 这个库， 可以很好的格式化输出
    // 下面是最常用的几个用法， 用不同level的log
    // log 的 level 从低到高
    // trace, debug, info, warn, error, critical
    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    spdlog::error("error~");
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    spdlog::info("Support for floats {:.2f}", 1.23456); // .2表示小数点后两位
    spdlog::info("Positional args are {1} {0}..", "too", "supported"); // 给参数定位
    spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left"); // 长度以及对其

    // Runtime log levels
    // log level 设置为 info， 所以低于info的debug不会显示， 大于等于info的才会显示
    spdlog::set_level(spdlog::level::info); // Set global log level to info
    spdlog::debug("This message should not be displayed!");
    spdlog::set_level(spdlog::level::trace); // Set specific logger's log level
    spdlog::debug("This message should be displayed..");

    // Customize msg format for all loggers
    // 自定义格式， 这个set pattern会对所有的logger 生效
    spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
    spdlog::info("This an info message with custom format");
    spdlog::set_pattern("%+"); // back to default format
    spdlog::set_level(spdlog::level::info);

    // Backtrace support
    // Loggers can store in a ring buffer all messages (including debug/trace) for later inspection.
    // When needed, call dump_backtrace() to see what happened:
    // backtrace 中文翻译成回溯. 这里是把log的内容都存起来, 最后再一起输出
    // 这里设置只能存10条log
    spdlog::enable_backtrace(10); // create ring buffer with capacity of 10  messages
    // 这里 log 100 次, 因为buffer只有10个, 所以只剩下最后十个
    for (int i = 0; i < 100; i++)
    {
        spdlog::debug("Backtrace message {}", i); // not logged..
    }
    // e.g. if some error happened:
    // 把所有log吐出来
    spdlog::dump_backtrace(); // log them now!

    try
    {
        stdout_logger_example();
        basic_example();
        rotating_example();
        daily_example();
        callback_example();
        async_example();
        binary_example();
        vector_example();
        multi_sink_example();
        user_defined_example();
        err_handler_example();
        trace_example();
        stopwatch_example();
        udp_example();
        custom_flags_example();
        file_events_example();
        replace_default_logger_example();

        // Flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        spdlog::flush_every(std::chrono::seconds(3));

        // Apply some function on all registered loggers
        spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->info("End of example."); });

        // Release all spdlog resources, and drop all loggers in the registry.
        // This is optional (only mandatory if using windows + async log).
        spdlog::shutdown();
    }

    // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const spdlog::spdlog_ex &ex)
    {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }
}

#include "spdlog/sinks/stdout_color_sinks.h"
// or #include "spdlog/sinks/stdout_sinks.h" if no colors needed.
void stdout_logger_example()
{
    // Create color multi threaded logger.
    // 后缀 mt 是 multi thread, 还有另一个后缀为 st 的版本, 意思是 single thread
    // spdlog对每个logger都提供了多线程和单线程版本, 但是并不需要多写很多代码, 因为无论是哪种版本, 
    // 只要传入一个类型参数 Mutex, 而对于这个 Mutex, 如果传入std::mutex, 那就是线程安全的版本, 适用于多线程的环境.
    // 如果不需要线程安全, 就传入spdlog自定义的一个类 null_mutex, 里面的 lock 和 unlock 方法是空实现.

    // 再说说spdlog创建一个logger的方法. 用的是工厂模式. 在Factory中定义一个静态的create函数, 
    // 传入logger的名字和其他参数, 并返回一个logger的shared_ptr. 
    // 对于不同的logger, 创建的时候需要不同的参数. 这里用的是c++的一些模板参数推断和参数包转发的功能
    // 这些应该都是c++11的特性, 读c++primer的时候有看到过, 一直没机会用上(其实一般也不会用到), 这些特性
    // 都是给写库的人用的. 
    auto console = spdlog::stdout_color_mt("console"); // 传入的参数是logger的名字

    // 这里贴一下这个stdout_color_mt函数的具体实现, 因为里面有个不认识的语法
    // return Factory::template create<sinks::stdout_color_sink_mt>(logger_name, mode);
    // Factory后面为什么要跟着template呢?  (这里贴一篇 milo yip 的知乎, https://zhuanlan.zhihu.com/p/20029820)
    // 因为对于一个模板, 你不知道Factory::hehe, 这个hehe是变量还是函数还是类型, 所以在这里需要指定create是一个模板函数,
    // 因此必须写 Factory::template create
    // 在 :: . -> 之后一定要写template
    // 写zmesh的时候也遇到过类似的问题, 不知道模板里的东西是函数还是变量还是类型名, 需要显示地写 typename, 表明它是一个类型名

    // or for stderr:
    // auto console = spdlog::stderr_color_mt("error-logger");
}

#include "spdlog/sinks/basic_file_sink.h"
void basic_example()
{
    // Create basic file logger (not rotated).
    // 这个例子是log到文件里, 第一个参数是logger的名字, 第二个参数是log文件路径, 第三个参数是是否truncate
    auto my_logger = spdlog::basic_logger_mt("file_logger", "logs/basic-log.txt", true);
}

#include "spdlog/sinks/rotating_file_sink.h"
void rotating_example()
{
    // Create a file rotating logger with 5mb size max and 3 rotated files.
    // log rotate中文译为日志轮换, 下面的例子是3个5m的文件
    // 填满一个文件之后, 就去填下一个空的文件, 直到三个都满了, 就把最先填满的那个清空, 写到那里面
    auto rotating_logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
}

#include "spdlog/sinks/daily_file_sink.h"
void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am.
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
}

#include "spdlog/sinks/callback_sink.h"
void callback_example()
{
    // Create the logger
    auto logger = spdlog::callback_logger_mt("custom_callback_logger", [](const spdlog::details::log_msg & /*msg*/) {
        // do what you need to do with msg
    });
}

#include "spdlog/cfg/env.h"
void load_levels_example()
{
    // Set the log level to "info" and mylogger to "trace":
    // SPDLOG_LEVEL=info,mylogger=trace && ./example
    spdlog::cfg::load_env_levels();
    // or from command line:
    // ./example SPDLOG_LEVEL=info,mylogger=trace
    // #include "spdlog/cfg/argv.h" // for loading levels from argv
    // spdlog::cfg::load_argv_levels(args, argv);
}

#include "spdlog/async.h"
void async_example()
{
    // Default thread pool settings can be modified *before* creating the async logger:
    // spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");

    for (int i = 1; i < 101; ++i)
    {
        async_file->info("Async message #{}", i);
    }
}

// Log binary data as hex.
// Many types of std::container<char> types can be used.
// Iterator ranges are supported too.
// Format flags:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output to lines.

#include "spdlog/fmt/bin_to_hex.h"
void binary_example()
{
    std::vector<char> buf(80);
    for (int i = 0; i < 80; i++)
    {
        buf.push_back(static_cast<char>(i & 0xff));
    }
    spdlog::info("Binary example: {}", spdlog::to_hex(buf));
    spdlog::info("Another binary example:{:n}", spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
    // more examples:
    // logger->info("uppercase: {:X}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters: {:Xs}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters, no position info: {:Xsp}", spdlog::to_hex(buf));
    // logger->info("hexdump style: {:a}", spdlog::to_hex(buf));
    // logger->info("hexdump style, 20 chars per line {:a}", spdlog::to_hex(buf, 20));
}

// Log a vector of numbers
#ifndef SPDLOG_USE_STD_FORMAT
#    include "spdlog/fmt/ranges.h"
void vector_example()
{
    std::vector<int> vec = {1, 2, 3};
    spdlog::info("Vector example: {}", vec);
}

#else
void vector_example() {}
#endif

// ! DSPDLOG_USE_STD_FORMAT

// Compile time log levels.
// define SPDLOG_ACTIVE_LEVEL to required level (e.g. SPDLOG_LEVEL_TRACE)
void trace_example()
{
    // trace from default logger
    SPDLOG_TRACE("Some trace message.. {} ,{}", 1, 3.23);
    // debug from default logger
    SPDLOG_DEBUG("Some debug message.. {} ,{}", 1, 3.23);

    // trace from logger object
    auto logger = spdlog::get("file_logger");
    SPDLOG_LOGGER_TRACE(logger, "another trace message");
}

// stopwatch example
#include "spdlog/stopwatch.h"
#include <thread>
void stopwatch_example()
{
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
    spdlog::info("Stopwatch: {} seconds", sw);
}

#include "spdlog/sinks/udp_sink.h"
void udp_example()
{
    spdlog::sinks::udp_sink_config cfg("127.0.0.1", 11091);
    auto my_logger = spdlog::udp_logger_mt("udplog", cfg);
    my_logger->set_level(spdlog::level::debug);
    my_logger->info("hello world");
}

// A logger with multiple sinks (stdout and file) - each with a different format and log level.
void multi_sink_example()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}

// User defined types logging
struct my_type
{
    int i = 0;
    explicit my_type(int i)
        : i(i){};
};

#ifndef SPDLOG_USE_STD_FORMAT // when using fmtlib
template<>
struct fmt::formatter<my_type> : fmt::formatter<std::string>
{
    auto format(my_type my, format_context &ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};

#else // when using std::format
template<>
struct std::formatter<my_type> : std::formatter<std::string>
{
    auto format(my_type my, format_context &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};
#endif

void user_defined_example()
{
    spdlog::info("user defined type: {}", my_type(14));
}

// Custom error handler. Will be triggered on log failure.
void err_handler_example()
{
    // can be set globally or per logger(logger->set_error_handler(..))
    spdlog::set_error_handler([](const std::string &msg) { printf("*** Custom log error handler: %s ***\n", msg.c_str()); });
}

// syslog example (linux/osx/freebsd)
#ifndef _WIN32
#    include "spdlog/sinks/syslog_sink.h"
void syslog_example()
{
    std::string ident = "spdlog-example";
    auto syslog_logger = spdlog::syslog_logger_mt("syslog", ident, LOG_PID);
    syslog_logger->warn("This is warning that will end up in syslog.");
}
#endif

// Android example.
#if defined(__ANDROID__)
#    include "spdlog/sinks/android_sink.h"
void android_example()
{
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("android", tag);
    android_logger->critical("Use \"adb shell logcat\" to view this message.");
}
#endif

// Log patterns can contain custom flags.
// this will add custom flag '%*' which will be bound to a <my_formatter_flag> instance
#include "spdlog/pattern_formatter.h"
class my_formatter_flag : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        std::string some_txt = "custom-flag";
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<my_formatter_flag>();
    }
};

void custom_flags_example()
{

    using spdlog::details::make_unique; // for pre c++14
    auto formatter = make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v");
    // set the new formatter using spdlog::set_formatter(formatter) or logger->set_formatter(formatter)
    // spdlog::set_formatter(std::move(formatter));
}

void file_events_example()
{
    // pass the spdlog::file_event_handlers to file sinks for open/close log file notifications
    spdlog::file_event_handlers handlers;
    handlers.before_open = [](spdlog::filename_t filename) { spdlog::info("Before opening {}", filename); };
    handlers.after_open = [](spdlog::filename_t filename, std::FILE *fstream) {
        spdlog::info("After opening {}", filename);
        fputs("After opening\n", fstream);
    };
    handlers.before_close = [](spdlog::filename_t filename, std::FILE *fstream) {
        spdlog::info("Before closing {}", filename);
        fputs("Before closing\n", fstream);
    };
    handlers.after_close = [](spdlog::filename_t filename) { spdlog::info("After closing {}", filename); };
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/events-sample.txt", true, handlers);
    spdlog::logger my_logger("some_logger", file_sink);
    my_logger.info("Some log line");
}

void replace_default_logger_example()
{
    // store the old logger so we don't break other examples.
    auto old_logger = spdlog::default_logger();

    auto new_logger = spdlog::basic_logger_mt("new_default_logger", "logs/new-default-log.txt", true);
    spdlog::set_default_logger(new_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::debug("This message should not be displayed!");
    spdlog::set_level(spdlog::level::trace);
    spdlog::debug("This message should be displayed..");

    spdlog::set_default_logger(old_logger);
}
