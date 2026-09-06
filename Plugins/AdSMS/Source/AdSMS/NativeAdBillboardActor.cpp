#include "NativeAdBillboardActor.h"

#include "AdManager.h"
#include "AdSMSSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Guid.h"
#include "UObject/ConstructorHelpers.h"

ANativeAdBillboardActor::ANativeAdBillboardActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
    Plane->SetupAttachment(Root);
    Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Plane->SetGenerateOverlapEvents(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
        TEXT("/Engine/BasicShapes/Plane.Plane")
    );

    if (PlaneMesh.Succeeded())
    {
        Plane->SetStaticMesh(PlaneMesh.Object);
    }
}

void ANativeAdBillboardActor::BeginPlay()
{
    Super::BeginPlay();

    RequestID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

    if (Plane)
    {
        UMaterialInterface* BaseMaterial = AdMaterial.Get();
        if (!BaseMaterial)
        {
            BaseMaterial = Plane->GetMaterial(0);
        }

        if (BaseMaterial)
        {
            DynamicAdMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicAdMaterial)
            {
                Plane->SetMaterial(0, DynamicAdMaterial);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[AdSMS][Billboard] No AdMaterial assigned and Plane has no Element 0 material."));
        }
    }

    UAdSMSSubsystem* AdSubsystem = GetWorld() && GetWorld()->GetGameInstance()
        ? GetWorld()->GetGameInstance()->GetSubsystem<UAdSMSSubsystem>()
        : nullptr;

    if (!AdSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AdSMS][Billboard] AdSMS GameInstanceSubsystem not found."));
        return;
    }

    CachedAdManager = AdSubsystem->GetAdManager();
    if (!CachedAdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AdSMS][Billboard] AdManager is null."));
        return;
    }

    CachedAdManager->OnNativeAdTextureReadyForRequest.AddDynamic(
        this,
        &ANativeAdBillboardActor::HandleNativeAdTextureReadyForRequest
    );

    if (bLoadAdOnBeginPlay)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[AdSMS][Billboard] Requesting independent Native Ad. RequestID=%s"),
            *RequestID
        );

        CachedAdManager->LoadNativeAdTextureForRequest(
            RequestID,
            NativeAdUnitID,
            TextureWidth,
            TextureHeight
        );
    }
}

void ANativeAdBillboardActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CachedAdManager)
    {
        CachedAdManager->OnNativeAdTextureReadyForRequest.RemoveDynamic(
            this,
            &ANativeAdBillboardActor::HandleNativeAdTextureReadyForRequest
        );

        if (!RequestID.IsEmpty())
        {
            CachedAdManager->ReleaseNativeAdTextureForRequest(RequestID);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void ANativeAdBillboardActor::HandleNativeAdTextureReadyForRequest(
    const FString& InRequestID,
    UTexture2D* Texture
)
{
    if (InRequestID == RequestID)
    {
        ApplyTexture(Texture);
    }
}

void ANativeAdBillboardActor::ApplyTexture(UTexture2D* Texture)
{
    if (!Texture || !DynamicAdMaterial)
    {
        return;
    }

    DynamicAdMaterial->SetTextureParameterValue(TextureParameterName, Texture);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[AdSMS][Billboard] Native Ad texture attached to 3D plane. Parameter=%s"),
        *TextureParameterName.ToString()
    );
}
