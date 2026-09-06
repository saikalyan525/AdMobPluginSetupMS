#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NativeAdBillboardActor.generated.h"

class UAdManager;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class USceneComponent;
class UTexture2D;

/**
 * Simple 3D billboard that receives the Native Ad texture from AdManager.
 * No Tick, screen projection, PopupWindow, or screen-space positioning is used.
 *
 * Development/prototype use: the NativeAd texture is a bitmap snapshot of a
 * NativeAdView. See UAdManager::LoadNativeAdTextureForRequest() for the production caveat.
 */
UCLASS(Blueprintable)
class ADSMS_API ANativeAdBillboardActor : public AActor
{
    GENERATED_BODY()

public:
    ANativeAdBillboardActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void HandleNativeAdTextureReadyForRequest(const FString& InRequestID, UTexture2D* Texture);

    void ApplyTexture(UTexture2D* Texture);

public:
    /** Root scene component. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ad Billboard")
    TObjectPtr<USceneComponent> Root;

    /** 3D plane that displays the ad material. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ad Billboard")
    TObjectPtr<UStaticMeshComponent> Plane;

    /** Material containing a Texture Sample parameter for the ad. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard")
    TObjectPtr<UMaterialInterface> AdMaterial;

    /** Texture parameter name inside AdMaterial. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard")
    FName TextureParameterName = TEXT("AdTexture");

    /** AdMob Native Ad Unit ID. Google test ID is used by default.
     *  Replace this in the child Blueprint with your own Native Ad Unit ID for live ads. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard|AdMob")
    FString NativeAdUnitID = TEXT("ca-app-pub-3940256099942544/2247696110");

    /** Texture resolution requested from the Android native-ad renderer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard", meta = (ClampMin = "128", ClampMax = "2048"))
    int32 TextureWidth = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard", meta = (ClampMin = "128", ClampMax = "2048"))
    int32 TextureHeight = 512;

    /** Automatically request a Native Ad when the actor begins play. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ad Billboard")
    bool bLoadAdOnBeginPlay = true;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicAdMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UAdManager> CachedAdManager;

    FString RequestID;
};
