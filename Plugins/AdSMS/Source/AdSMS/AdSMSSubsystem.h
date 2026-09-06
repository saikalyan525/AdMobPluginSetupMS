#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AdSMSSubsystem.generated.h"

class UAdManager;

/**
 * AdSMS entry point. Automatically exists alongside the project's normal GameInstance.
 * No custom GameInstance inheritance is required.
 */
UCLASS(BlueprintType)
class ADSMS_API UAdSMSSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category="AdSMS")
    UAdManager* GetAdManager() const;

private:
    UPROPERTY()
    TObjectPtr<UAdManager> AdManager;
};
