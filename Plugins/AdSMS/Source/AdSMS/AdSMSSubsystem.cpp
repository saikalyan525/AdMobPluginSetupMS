#include "AdSMSSubsystem.h"
#include "AdManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdSMSSubsystem, Log, All);

void UAdSMSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    AdManager = NewObject<UAdManager>(this);
    if (!AdManager)
    {
        UE_LOG(LogAdSMSSubsystem, Error, TEXT("[AdSMS] Failed to create UAdManager."));
        return;
    }

    UE_LOG(LogAdSMSSubsystem, Log, TEXT("[AdSMS] GameInstanceSubsystem initialized. Creating AdManager."));
    AdManager->InitializeAds();
}

void UAdSMSSubsystem::Deinitialize()
{
    if (AdManager)
    {
        AdManager->ShutdownAds();
        AdManager = nullptr;
    }

    UE_LOG(LogAdSMSSubsystem, Log, TEXT("[AdSMS] GameInstanceSubsystem deinitialized."));
    Super::Deinitialize();
}

UAdManager* UAdSMSSubsystem::GetAdManager() const
{
    return AdManager;
}
