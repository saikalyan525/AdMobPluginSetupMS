#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "AdManager.generated.h"

class UAdRewardHandler;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnNativeAdTextureReadyForRequest,
    const FString&,
    RequestID,
    UTexture2D*,
    Texture
);

#if !PLATFORM_ANDROID
class SBox;
class SOverlay;
class STextBlock;
#endif

/** Central AdMob manager owned by UAdSMSSubsystem. */
UCLASS(BlueprintType)
class ADSMS_API UAdManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="AdSMS|Ads")
    void InitializeAds();

    UFUNCTION(BlueprintCallable, Category="AdSMS|Ads")
    void ShutdownAds();

    // ---------------- Banner: screen ----------------
    UFUNCTION(BlueprintCallable, Category="AdSMS|Banner")
    void LoadBanner();

    UFUNCTION(BlueprintCallable, Category="AdSMS|Banner")
    void ShowBanner();

    UFUNCTION(BlueprintCallable, Category="AdSMS|Banner")
    void HideBanner();

    UFUNCTION(BlueprintCallable, Category="AdSMS|Banner")
    void ShowBannerAtScreenPosition(int32 X, int32 Y, int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable, Category="AdSMS|Banner")
    void SetBannerScreenPosition(int32 X, int32 Y, int32 Width, int32 Height);

    UFUNCTION(BlueprintPure, Category="AdSMS|Banner")
    bool IsBannerLoaded() const;

    // ---------------- Native 3D billboard ----------------
    /** Requests an independent Native Ad texture for one billboard. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Native Billboard")
    void LoadNativeAdTextureForRequest(const FString& RequestID, const FString& AdUnitID, int32 Width=1024, int32 Height=512);

    /** Releases the Native Ad/texture belonging to one billboard. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Native Billboard")
    void ReleaseNativeAdTextureForRequest(const FString& RequestID);

    UPROPERTY(BlueprintAssignable, Category="AdSMS|Native Billboard")
    FOnNativeAdTextureReadyForRequest OnNativeAdTextureReadyForRequest;

    // ---------------- Interstitial ----------------
    /** Preloads the next Interstitial if one is not ready/requested. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Interstitial")
    void LoadInterstitial();

    /** Shows immediately if ready; otherwise requests and waits. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Interstitial")
    void ShowInterstitial();

    UFUNCTION(BlueprintPure, Category="AdSMS|Interstitial")
    bool IsInterstitialReady() const;

    UFUNCTION(BlueprintPure, Category="AdSMS|Interstitial")
    bool IsInterstitialRequested() const;

    // ---------------- Rewarded ----------------
    /** Preloads the next Rewarded if one is not ready/requested. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Rewarded")
    void LoadRewarded();

    /** Shows immediately if ready; otherwise requests and waits. */
    UFUNCTION(BlueprintCallable, Category="AdSMS|Rewarded")
    void ShowRewarded();

    UFUNCTION(BlueprintPure, Category="AdSMS|Rewarded")
    bool IsRewardedReady() const;

    UFUNCTION(BlueprintPure, Category="AdSMS|Rewarded")
    bool IsRewardedRequested() const;

    UFUNCTION(BlueprintPure, Category="AdSMS|Rewarded")
    UAdRewardHandler* GetRewardHandler() const;

    UFUNCTION(BlueprintPure, Category="AdSMS|Ads")
    bool IsAdsInitialized() const;

private:
    void CheckPendingInterstitialShow();
    void StartInterstitialReadyCheck();
    void StopInterstitialReadyCheck();

    void CheckRewardedCompletion();
    void StartRewardedCompletionCheck();
    void StopRewardedCompletionCheck();
    void CheckPendingRewardedShow();
    void StartRewardedReadyCheck();
    void StopRewardedReadyCheck();

    void CheckNativeAdTextureRequestsReady();
    bool LoadNativeAdTextureFromFileForRequest(const FString& RequestID, const FString& FilePath);

protected:
    UPROPERTY() bool bAdsInitialized=false;
    UPROPERTY() bool bBannerLoaded=false;
    UPROPERTY() bool bBannerVisible=false;
    UPROPERTY() bool bInterstitialReady=false;
    UPROPERTY() bool bRewardedReady=false;
    UPROPERTY() bool bWaitingToShowInterstitial=false;
    UPROPERTY() bool bWaitingToShowRewarded=false;

    FTimerHandle InterstitialReadyCheckTimerHandle;
    FTimerHandle RewardedReadyCheckTimerHandle;
    FTimerHandle RewardedCompletionCheckTimerHandle;
    FTimerHandle NativeAdTextureRequestsReadyCheckTimerHandle;

    UPROPERTY() TObjectPtr<UAdRewardHandler> RewardHandler;

    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<UTexture2D>> NativeAdTexturesByRequest;

    TSet<FString> PendingNativeAdTextureRequests;

#if !PLATFORM_ANDROID
    TSharedPtr<SOverlay> PCMockBannerOverlay;
    TSharedPtr<SBox> PCMockBannerWidget;
    TSharedPtr<STextBlock> PCMockBannerText;
#endif
};
