// Include spdlog support for a basic file logger
// See below for more details
#include "spdlog/sinks/basic_file_sink.h"

// spdlog documentation
// https://github.com/gabime/spdlog

void SetupLog() {
    // Get the path to the SKSE logs folder
    // This will generally be your Documents\My Games\Skyrim Special Edition\SKSE
    //                          or Documents\My Games\Skyrim Special Edition GOG\SKSE
    auto logsFolder = SKSE::log::log_directory();

    // I really don't understand why the log_directory() might not be provided sometimes,
    // but... just incase... ?
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        return;
    }

    // Get the name of this SKSE plugin. We will use it to name the log file.
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    // Generate a path to our log file
    // e.g. Documents\My Games\Skyrim Special Edition\SKSE\OurPlugin.log
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);

    // Now, use whatever you want, but spdlog comes with CommonLibSSE
    // and is the SKSE logger of choice. So you might as well use it!

    // Let's use a spdlog "basic file sink"
    // So like... just a file logger...
    // But the spdlog interface really wants a Shared Pointer to the "basic file sink"
    // So we'll make one of those!
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);

    // Ok, but set_default_logger() specifically wants a Shared Pointer to a spdlog::logger
    // So we'll make one of those!
    // We'll give it the logger we made above. Yeah, I know, kinda redundant right? Welcome to C++
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));

    // Yay, let's setup spdlog now!
    // By default, let's print out *everything* including trace messages
    // You might want to do something like #ifdef NDEBUG then use trace, else use info or higher severity.
    spdlog::set_level(spdlog::level::trace);

    // This bit is important. When does spdlog WRITE to the file?
    // Make sure it does it everytime you log a message, otherwise it won't write to the file until the game exits.
    spdlog::flush_on(spdlog::level::info);
}

// Read about this more below...
namespace logger = SKSE::log;

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    // Setup logging (e.g. using spdlog)
    SetupLog();

    // Now log something!
    spdlog::info("Wassup, hi from info!");
    spdlog::error("Oh noes! Something bad with error code {} happened!", 69);

    // Here is what the lines will look like in the log:
    // [2022-12-11 12:52:35.797] [plugin-log] [info] Wassup, hi from info!
    // [2022-12-11 12:52:35.798] [plugin-log] [error] Oh noes! Something bad with error code 69 happened!

    // Cool, cool.
    // Now if you want to be more SKSE-ish, you should use SKSE::log (which just calls spdlog)
    SKSE::log::info("SKSE log info");
    SKSE::log::error("SKSE log error {}", 123);

    // SKSE::log uses spdlog but it adds a super useful prefix
    // which shows the actual line of code which the log was sent from!
    // Here is what the lines will look like in the log:
    // [2022-12-11 12:52:35.798] [plugin-log] [info] [plugin.cpp:52] SKSE log info
    // [2022-12-11 12:52:35.798] [plugin-log] [error] [plugin.cpp:53] SKSE log error 123
    // [2022-12-11 12:52:35.798] [plugin-log] [trace] [plugin.cpp:58] Logged using a nice shorthand for SKSE::log
    // The [plugin.cpp:<line>] lines are pretty rad, amiright?

    // Maybe, even better, use your own namespace (e.g. by aliasing SKSE::log)
    // And this way you can easily swap our your logger to be whatever you want in the future
    // so long as you provide this same interface.
    logger::trace("Logged using a nice shorthand for SKSE::log");

    // That's all, folks! Happy Modding!

    return true;
}
