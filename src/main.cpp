#include <stddef.h>

#include "NFAC/NFAC.h"

using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    auto ptr = Loki::NoFollowerAttackCollision::GetSingleton();

    switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded: {
            ptr->InstallInputSink();
            break;
        }
        case SKSE::MessagingInterface::kNewGame: {
            break;
        }
        case SKSE::MessagingInterface::kPostLoadGame: {
            break;
        }
        case SKSE::MessagingInterface::kPostLoad: {
            ptr->InstallMeleeHook();
            ptr->InstallSweepHook();
            ptr->InstallArrowHook();
            break;
        }
        case SKSE::MessagingInterface::kPostPostLoad: {
            break;
        }
        default:
            break;
    }
}

SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);


    Init(skse);
    SKSE::AllocTrampoline(64);

    
    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", SKSEMessageHandler)) {  // add callbacks for TrueHUD
        return false;
    }

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
