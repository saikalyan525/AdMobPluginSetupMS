#include "AdSMS.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdSMSModule, Log, All);

class FAdSMSModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogAdSMSModule, Log, TEXT("[AdSMS] Runtime module loaded."));
        FDefaultGameModuleImpl::StartupModule();
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogAdSMSModule, Log, TEXT("[AdSMS] Runtime module unloaded."));
        FDefaultGameModuleImpl::ShutdownModule();
    }
};

IMPLEMENT_MODULE(FAdSMSModule, AdSMS)
