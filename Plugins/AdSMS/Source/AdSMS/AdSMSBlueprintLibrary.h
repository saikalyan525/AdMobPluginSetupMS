#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdSMSBlueprintLibrary.generated.h"

class UAdSMSSubsystem;
class UAdManager;

/**
 * Project-agnostic AdSMS accessors.
 * These functions do not require the project's GameInstance to inherit from an AdSMS class.
 */
UCLASS()
class ADSMS_API UAdSMSBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Returns the AdSMS subsystem owned by the current world's GameInstance. */
    UFUNCTION(BlueprintPure, Category="AdSMS", meta=(WorldContext="WorldContextObject", CompactNodeTitle="AdSMS"))
    static UAdSMSSubsystem* GetAdSMSSubsystem(const UObject* WorldContextObject);

    /** Returns the AdManager owned by the current world's AdSMS subsystem. */
    UFUNCTION(BlueprintPure, Category="AdSMS", meta=(WorldContext="WorldContextObject", CompactNodeTitle="AdManager"))
    static UAdManager* GetAdManager(const UObject* WorldContextObject);
};
