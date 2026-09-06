#include "AdSMSBlueprintLibrary.h"
#include "AdSMSSubsystem.h"
#include "AdManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UAdSMSSubsystem* UAdSMSBlueprintLibrary::GetAdSMSSubsystem(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = GEngine
        ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
        : nullptr;

    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    return GameInstance ? GameInstance->GetSubsystem<UAdSMSSubsystem>() : nullptr;
}

UAdManager* UAdSMSBlueprintLibrary::GetAdManager(const UObject* WorldContextObject)
{
    UAdSMSSubsystem* Subsystem = GetAdSMSSubsystem(WorldContextObject);
    return Subsystem ? Subsystem->GetAdManager() : nullptr;
}
