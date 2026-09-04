#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdMobBlueprintLibrary.generated.h"

class UWidgetComponent;

UCLASS()
class ADMOBPLUGIN_API UAdMobBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="AdMob|Ads", meta=(WorldContext="WorldContextObject"))
    static void InitializeAds(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Ads", meta=(WorldContext="WorldContextObject"))
    static void ShutdownAds(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject"))
    static void LoadBanner(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject"))
    static void ShowBanner(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject"))
    static void ShowBannerAtScreenPosition(const UObject* WorldContextObject, int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject", DisplayName="Show Banner At Widget"))
    static void ShowBannerAtWidget(const UObject* WorldContextObject, UWidgetComponent* WidgetComponent);

    UFUNCTION(BlueprintCallable, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject"))
    static void HideBanner(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category="AdMob|Banner", meta=(WorldContext="WorldContextObject"))
    static bool IsBannerLoaded(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Interstitial", meta=(WorldContext="WorldContextObject"))
    static void LoadInterstitial(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Interstitial", meta=(WorldContext="WorldContextObject"))
    static void ShowInterstitial(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category="AdMob|Interstitial", meta=(WorldContext="WorldContextObject"))
    static bool IsInterstitialReady(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Rewarded", meta=(WorldContext="WorldContextObject"))
    static void LoadRewarded(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="AdMob|Rewarded", meta=(WorldContext="WorldContextObject"))
    static void ShowRewarded(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category="AdMob|Rewarded", meta=(WorldContext="WorldContextObject"))
    static bool IsRewardedReady(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category="AdMob|Ads", meta=(WorldContext="WorldContextObject"))
    static bool IsAdsInitialized(const UObject* WorldContextObject);
};
