#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>

namespace logger = SKSE::log;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        return;
    }
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::info);
}

void ConvertToLowerCase(std::string& text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });
}

void GetFormByID() {
    auto* sweetroll = RE::TESForm::LookupByID(0x64B3D);
    logger::info("Form Name: {}", sweetroll->GetName());
}

void GetFormByEditorID_IncludesFormCasting() {
    auto* sweetroll1 = skyrim_cast<RE::AlchemyItem*, RE::TESForm>(RE::TESForm::LookupByEditorID("FoodSweetRoll"));
    auto* sweetroll2 = RE::TESForm::LookupByEditorID("FoodSweetRoll")->As<RE::AlchemyItem>();
    auto* sweetroll3 = RE::TESForm::LookupByEditorID<RE::AlchemyItem>("FoodSweetRoll");

    auto* sweetroll = sweetroll1;

    if (!sweetroll) {
        logger::error("Could not get the Sweet Roll!");
        return;
    }
    if (sweetroll) {
        logger::info("Is {} a food? {}", sweetroll->GetName(), sweetroll->IsFood());
        for (auto* effect : sweetroll->effects) {
            logger::info("Effect {} of magnitude {}", effect->baseEffect->GetName(), effect->GetMagnitude());
        }
    }
}

void LoopThruAllFormsOfType() {
    // Print out all Alchemy items, e.g. potions/food, which are food and contain "soup" in the name
    auto alchemyItems = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::AlchemyItem>();
    for (auto* item : alchemyItems)
        if (item->IsFood()) {
            auto name = std::string(item->GetName());
            ConvertToLowerCase(name);
            if (name.contains("soup")) logger::info("Yum, yum soup! {}", name);
        }
}

void LoopThruAllForms() {
    // Print out anything that costs over 5000 gold
    const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *literallyEveryFormInTheGame) {
        if (form->IsArmor())
            if (form->GetGoldValue() > 5000)
                logger::info("Whoa! {} is expensive! It costs {}", form->GetName(), form->GetGoldValue());
    }
}

void LoopThruHodReferences() {
    // For any *Object Reference* where the base name contains "Hod", print it out!
    const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *literallyEveryFormInTheGame) {
        auto* reference = form->AsReference();
        if (reference) {
            auto name = std::string(reference->GetBaseObject()->GetName());
            ConvertToLowerCase(name);
            if (name.contains("hod")) logger::info("Hod-related reference: {} {:x}", name, reference->GetFormID());
        }
    }
}

void OnFormsAvailable() {
    GetFormByID();
    GetFormByEditorID_IncludesFormCasting();
    LoopThruAllFormsOfType();
    LoopThruAllForms();
    LoopThruHodReferences();
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) OnFormsAvailable();
    });
    return true;
}
