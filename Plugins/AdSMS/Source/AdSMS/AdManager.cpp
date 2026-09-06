#include "AdManager.h"

#include "AdRewardHandler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#if !PLATFORM_ANDROID
#include "Engine/GameViewportClient.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJava.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogAdManager, Log, All);

#if PLATFORM_ANDROID
namespace
{
	static jmethodID FindAdMethod(JNIEnv* Env, const char* Name, const char* Signature)
	{
		if (!Env)
		{
			UE_LOG(LogAdManager, Error, TEXT("[AdSMS][JNI] JNIEnv is NULL. Method=%hs Signature=%hs"), Name, Signature);
			return nullptr;
		}

		if (!FJavaWrapper::GameActivityClassID)
		{
			UE_LOG(LogAdManager, Error, TEXT("[AdSMS][JNI] GameActivityClassID is NULL. Method=%hs Signature=%hs"), Name, Signature);
			return nullptr;
		}

		if (!FJavaWrapper::GameActivityThis)
		{
			UE_LOG(LogAdManager, Error, TEXT("[AdSMS][JNI] GameActivityThis is NULL. Method=%hs Signature=%hs"), Name, Signature);
			return nullptr;
		}

		jmethodID Method = FJavaWrapper::FindMethod(
			Env,
			FJavaWrapper::GameActivityClassID,
			Name,
			Signature,
			false
		);

		if (!Method)
		{
			UE_LOG(LogAdManager, Error, TEXT("[AdSMS][JNI] Method NOT found: %hs %hs"), Name, Signature);
		}

		return Method;
	}

	static bool JavaBool(const char* Name)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "()Z");
			if (!Method)
			{
				return false;
			}

			const jboolean Result = FJavaWrapper::CallBooleanMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method
			);

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] %hs -> %s"),
				Name,
				Result ? TEXT("true") : TEXT("false")
			);

			return Result != JNI_FALSE;
		}

		return false;
	}

	static void JavaVoid(const char* Name)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "()V");
			if (Method)
			{
				UE_LOG(LogAdManager, Log, TEXT("[AdSMS][JNI] Calling %hs"), Name);
				FJavaWrapper::CallVoidMethod(
					Env,
					FJavaWrapper::GameActivityThis,
					Method
				);
			}
		}
	}

	static void JavaStringInt4Void(const char* Name, const char* Value, int32 A, int32 B, int32 C, int32 D)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(
				Env,
				Name,
				"(Ljava/lang/String;IIII)V"
			);

			if (!Method)
			{
				return;
			}

			jstring JavaValue = Env->NewStringUTF(Value ? Value : "");

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] Calling %hs: X=%d Y=%d W=%d H=%d"),
				Name, A, B, C, D
			);

			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue,
				A, B, C, D
			);

			Env->DeleteLocalRef(JavaValue);
		}
	}

	static void JavaStringInt2Void(const char* Name, const char* Value, int32 A, int32 B)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(Ljava/lang/String;II)V");
			if (!Method)
			{
				return;
			}

			jstring JavaValue = Env->NewStringUTF(Value ? Value : "");

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] Calling %hs with NativeAdUnitID: %dx%d"),
				Name, A, B
			);

			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue,
				A,
				B
			);

			Env->DeleteLocalRef(JavaValue);
		}
	}

	static void JavaStringStringInt2Void(const char* Name, const char* ValueA, const char* ValueB, int32 A, int32 B)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(Ljava/lang/String;Ljava/lang/String;II)V");
			if (!Method)
			{
				return;
			}

			jstring JavaValueA = Env->NewStringUTF(ValueA ? ValueA : "");
			jstring JavaValueB = Env->NewStringUTF(ValueB ? ValueB : "");

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] Calling %hs RequestID=%hs Size=%dx%d"),
				Name,
				ValueB ? ValueB : "",
				A,
				B
			);

			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValueA,
				JavaValueB,
				A,
				B
			);

			Env->DeleteLocalRef(JavaValueA);
			Env->DeleteLocalRef(JavaValueB);
		}
	}

	static bool JavaStringBool(const char* Name, const FString& Value)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(Ljava/lang/String;)Z");
			if (!Method)
			{
				return false;
			}

			FTCHARToUTF8 Utf8(*Value);
			jstring JavaValue = Env->NewStringUTF(Utf8.Get());
			const jboolean Result = FJavaWrapper::CallBooleanMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue
			);
			Env->DeleteLocalRef(JavaValue);
			return Result != JNI_FALSE;
		}
		return false;
	}

	static FString JavaStringArg(const char* Name, const FString& Value)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(Ljava/lang/String;)Ljava/lang/String;");
			if (!Method)
			{
				return FString();
			}

			FTCHARToUTF8 Utf8(*Value);
			jstring JavaValue = Env->NewStringUTF(Utf8.Get());
			jstring Result = static_cast<jstring>(FJavaWrapper::CallObjectMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue
			));
			Env->DeleteLocalRef(JavaValue);

			if (!Result)
			{
				return FString();
			}

			const char* ResultChars = Env->GetStringUTFChars(Result, nullptr);
			FString Out = UTF8_TO_TCHAR(ResultChars ? ResultChars : "");
			if (ResultChars)
			{
				Env->ReleaseStringUTFChars(Result, ResultChars);
			}
			Env->DeleteLocalRef(Result);
			return Out;
		}
		return FString();
	}

	static void JavaStringVoidArg(const char* Name, const FString& Value)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(Ljava/lang/String;)V");
			if (!Method)
			{
				return;
			}

			FTCHARToUTF8 Utf8(*Value);
			jstring JavaValue = Env->NewStringUTF(Utf8.Get());
			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue
			);
			Env->DeleteLocalRef(JavaValue);
		}
	}

	static void JavaInt2Void(const char* Name, int32 A, int32 B)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "(II)V");
			if (!Method)
			{
				return;
			}

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] Calling %hs: A=%d B=%d"),
				Name, A, B
			);

			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				A,
				B
			);
		}
	}

	static void JavaStringVoid(const char* Name, const char* Value)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(
				Env,
				Name,
				"(Ljava/lang/String;)V"
			);

			if (!Method)
			{
				return;
			}

			jstring JavaValue = Env->NewStringUTF(Value ? Value : "");

			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][JNI] Calling %hs with AdUnitID"),
				Name
			);

			FJavaWrapper::CallVoidMethod(
				Env,
				FJavaWrapper::GameActivityThis,
				Method,
				JavaValue
			);

			Env->DeleteLocalRef(JavaValue);
		}
	}


	static int32 JavaInt(const char* Name)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(Env, Name, "()I");
			if (!Method)
			{
				return 0;
			}

			return static_cast<int32>(
				FJavaWrapper::CallIntMethod(
					Env,
					FJavaWrapper::GameActivityThis,
					Method
				)
			);
		}

		return 0;
	}

	static FString JavaString(const char* Name)
	{
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			jmethodID Method = FindAdMethod(
				Env,
				Name,
				"()Ljava/lang/String;"
			);

			if (!Method)
			{
				return FString();
			}

			jstring JavaResult = static_cast<jstring>(
				FJavaWrapper::CallObjectMethod(
					Env,
					FJavaWrapper::GameActivityThis,
					Method
				)
			);

			if (!JavaResult)
			{
				return FString();
			}

			const char* UTF8Result = Env->GetStringUTFChars(JavaResult, nullptr);
			FString Result = UTF8Result ? UTF8_TO_TCHAR(UTF8Result) : FString();

			if (UTF8Result)
			{
				Env->ReleaseStringUTFChars(JavaResult, UTF8Result);
			}

			Env->DeleteLocalRef(JavaResult);
			return Result;
		}

		return FString();
	}
}
#endif


void UAdManager::InitializeAds()
{
	if (bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("InitializeAds() called, but AdMob is already initialized."));
		return;
	}

	UE_LOG(LogAdManager, Log, TEXT("========================================"));
	UE_LOG(LogAdManager, Log, TEXT("Initializing AdMob system..."));

	RewardHandler = NewObject<UAdRewardHandler>(this, UAdRewardHandler::StaticClass());

	if (RewardHandler)
	{
		UE_LOG(LogAdManager, Log, TEXT("AdRewardHandler created successfully."));
	}
	else
	{
		UE_LOG(LogAdManager, Error, TEXT("Failed to create AdRewardHandler."));
	}

	bAdsInitialized = true;

	UE_LOG(LogAdManager, Log, TEXT("AdMob Unreal-side manager initialized."));

	// AdSMS owns Banner, Interstitial and Rewarded. No Unreal AndroidAdvertising dependency is used.
	LoadInterstitial();
	LoadRewarded();

	UE_LOG(LogAdManager, Log, TEXT("Initial Interstitial and Rewarded loading requests submitted."));
	UE_LOG(LogAdManager, Log, TEXT("Banner is available on demand from the test widget."));
	UE_LOG(LogAdManager, Log, TEXT("========================================"));
}

void UAdManager::ShutdownAds()
{
	if (!bAdsInitialized)
	{
		return;
	}

	UE_LOG(LogAdManager, Log, TEXT("Shutting down advertisement system."));

	StopInterstitialReadyCheck();
	StopRewardedReadyCheck();
	StopRewardedCompletionCheck();
	bWaitingToShowRewarded = false;

#if PLATFORM_ANDROID
	JavaVoid("AndroidThunkJava_CloseAdBanner");
#endif

#if !PLATFORM_ANDROID
	if (PCMockBannerOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(PCMockBannerOverlay.ToSharedRef());
	}
	PCMockBannerText.Reset();
	PCMockBannerWidget.Reset();
	PCMockBannerOverlay.Reset();
#endif

	bBannerLoaded = false;
	bBannerVisible = false;
	bInterstitialReady = false;
	bRewardedReady = false;
	bWaitingToShowInterstitial = false;

	RewardHandler = nullptr;
	bAdsInitialized = false;

	UE_LOG(LogAdManager, Log, TEXT("Advertisement system shut down."));
	PendingNativeAdTextureRequests.Empty();
	NativeAdTexturesByRequest.Empty();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NativeAdTextureRequestsReadyCheckTimerHandle);
	}

}

void UAdManager::LoadBanner()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("LoadBanner() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID
	UE_LOG(LogAdManager, Log, TEXT("Requesting Banner advertisement through AdSMS JNI."));

	JavaStringVoid(
		"AndroidThunkJava_ShowAdBanner",
		"ca-app-pub-3940256099942544/9214589741"
	);

	bBannerVisible = true;
	bBannerLoaded = true;

	UE_LOG(LogAdManager, Log, TEXT("Banner request submitted."));
#else
	// Windows/Editor: use the same screen-space coordinates as Android, but render a
	// visible mock banner so billboard positioning can be tested without packaging.
	ShowBannerAtScreenPosition(0, 0, 320, 50);
#endif
}

void UAdManager::ShowBanner()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("ShowBanner() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID
	UE_LOG(LogAdManager, Log, TEXT("Showing Banner advertisement through AdSMS JNI."));

	JavaStringVoid(
		"AndroidThunkJava_ShowAdBanner",
		"ca-app-pub-3940256099942544/9214589741"
	);

	bBannerVisible = true;
	bBannerLoaded = true;
#else
	ShowBannerAtScreenPosition(0, 0, 320, 50);
#endif
}

void UAdManager::HideBanner()
{
	if (!bAdsInitialized)
	{
		return;
	}

#if PLATFORM_ANDROID
	UE_LOG(LogAdManager, Log, TEXT("Hiding Banner advertisement through AdSMS JNI."));

	JavaVoid("AndroidThunkJava_HideAdBanner");

	bBannerVisible = false;
#else
	if (PCMockBannerOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(PCMockBannerOverlay.ToSharedRef());
	}
	PCMockBannerText.Reset();
	PCMockBannerWidget.Reset();
	PCMockBannerOverlay.Reset();
	bBannerVisible = false;
	bBannerLoaded = false;
#endif
}

void UAdManager::ShowBannerAtScreenPosition(int32 X, int32 Y, int32 Width, int32 Height)
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("ShowBannerAtScreenPosition() called before InitializeAds()."));
		return;
	}

	Width = FMath::Max(1, Width);
	Height = FMath::Max(1, Height);

#if PLATFORM_ANDROID
	// Android's AdMob AdSize.BANNER controls the actual ad size. X/Y are screen
	// coordinates supplied by Blueprint; width/height are retained for the native
	// popup container. Do not clamp X/Y here because an off-screen projected
	// billboard must not jump to (0,0).
	UE_LOG(
		LogAdManager,
		Log,
		TEXT("[AdSMS][ANDROID][BANNER REQUEST] X=%d Y=%d W=%d H=%d"),
		X, Y, Width, Height
	);

	JavaStringInt4Void(
		"AndroidThunkJava_ShowAdBannerAtPosition",
		"ca-app-pub-3940256099942544/9214589741",
		X, Y, Width, Height
	);

	bBannerVisible = true;
#else
	// Windows/Editor mock:
	// Use a full-screen transparent overlay as the viewport host and keep the
	// actual mock banner inside a fixed-size SBox. This prevents the viewport from
	// forcing the banner itself to fill the whole game window.
	if (!PCMockBannerOverlay.IsValid())
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			UE_LOG(LogAdManager, Error, TEXT("[AdSMS][PC MOCK] GameViewport is not available; cannot create mock banner."));
			return;
		}

		// Create the mock as a real Slate viewport widget. It is deliberately
		// independent of AdTestWidgetClass/UMG so the billboard test works even
		// when the optional test widget is not assigned.
		SAssignNew(PCMockBannerText, STextBlock)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor(FLinearColor::White));

		SAssignNew(PCMockBannerWidget, SBox)
			.WidthOverride((float)Width)
			.HeightOverride((float)Height)
			.Visibility(EVisibility::Visible)
			.RenderTransformPivot(FVector2D::ZeroVector)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.9f, 0.05f, 0.05f, 1.0f))
				.Padding(FMargin(4.0f))
				[
					PCMockBannerText.ToSharedRef()
				]
			];

		PCMockBannerOverlay = SNew(SOverlay)
			.Visibility(EVisibility::Visible)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				PCMockBannerWidget.ToSharedRef()
			];

		GEngine->GameViewport->AddViewportWidgetContent(
			PCMockBannerOverlay.ToSharedRef(),
			100000
		);

		UE_LOG(LogAdManager, Log, TEXT("[AdSMS][PC MOCK] Slate banner widget CREATED and added to GameViewport."));
	}

	if (PCMockBannerWidget.IsValid())
	{
		PCMockBannerWidget->SetVisibility(EVisibility::Visible);
		PCMockBannerWidget->SetWidthOverride((float)Width);
		PCMockBannerWidget->SetHeightOverride((float)Height);
		PCMockBannerWidget->SetRenderTransformPivot(FVector2D::ZeroVector);
		PCMockBannerWidget->SetRenderTransform(
			FSlateRenderTransform(FVector2D((float)X, (float)Y))
		);
	}

	if (PCMockBannerText.IsValid())
	{
		PCMockBannerText->SetText(
			FText::FromString(
				FString::Printf(TEXT("PC MOCK AD\\n%d x %d\\nAdMob Banner"), Width, Height)
			)
		);
	}

	bBannerLoaded = true;
	bBannerVisible = true;

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("[AdSMS][PC MOCK][SHOW] X=%d Y=%d W=%d H=%d"),
		X, Y, Width, Height
	);
#endif
}

void UAdManager::SetBannerScreenPosition(int32 X, int32 Y, int32 Width, int32 Height)
{
	if (!bAdsInitialized)
	{
		return;
	}

	Width = FMath::Max(1, Width);
	Height = FMath::Max(1, Height);

#if PLATFORM_ANDROID
	// Keep negative/off-screen X/Y values intact. Clamping them to zero would
	// teleport a billboard ad to the top-left corner when the anchor leaves the
	// camera viewport.
	UE_LOG(
		LogAdManager,
		Verbose,
		TEXT("[AdSMS][ANDROID][BANNER POSITION] X=%d Y=%d W=%d H=%d"),
		X, Y, Width, Height
	);

	JavaStringInt4Void(
		"AndroidThunkJava_SetAdBannerPosition",
		"ca-app-pub-3940256099942544/9214589741",
		X, Y, Width, Height
	);
#else
	UE_LOG(
		LogAdManager,
		Verbose,
		TEXT("[AdSMS][PC MOCK][POSITION REQUEST] X=%d Y=%d W=%d H=%d"),
		X, Y, Width, Height
	);

	if (PCMockBannerWidget.IsValid())
	{
		PCMockBannerWidget->SetVisibility(EVisibility::Visible);
		PCMockBannerWidget->SetWidthOverride((float)Width);
		PCMockBannerWidget->SetHeightOverride((float)Height);
		PCMockBannerWidget->SetRenderTransformPivot(FVector2D::ZeroVector);
		PCMockBannerWidget->SetRenderTransform(
			FSlateRenderTransform(FVector2D((float)X, (float)Y))
		);

		if (PCMockBannerText.IsValid())
		{
			PCMockBannerText->SetText(
				FText::FromString(
					FString::Printf(TEXT("PC MOCK AD\\n%d x %d\\nAdMob Banner"), Width, Height)
				)
			);
		}

		bBannerLoaded = true;
		bBannerVisible = true;

		UE_LOG(
			LogAdManager,
			Verbose,
			TEXT("[AdSMS][PC MOCK][POSITION APPLIED] X=%d Y=%d W=%d H=%d"),
			X, Y, Width, Height
		);
	}
	else
	{
		UE_LOG(
			LogAdManager,
			Warning,
			TEXT("[AdSMS][PC MOCK] Position requested but mock widget does not exist yet.")
		);
	}
#endif
}


bool UAdManager::IsBannerLoaded() const
{
#if PLATFORM_ANDROID
	return JavaBool("AndroidThunkJava_IsAdBannerLoaded");
#else
	return bBannerLoaded;
#endif
}




void UAdManager::LoadNativeAdTextureForRequest(const FString& RequestID, const FString& AdUnitID, int32 Width, int32 Height)
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("LoadNativeAdTextureForRequest() called before InitializeAds()."));
		return;
	}

	const FString TrimmedRequestID = RequestID.TrimStartAndEnd();
	const FString TrimmedAdUnitID = AdUnitID.TrimStartAndEnd();
	if (TrimmedRequestID.IsEmpty())
	{
		UE_LOG(LogAdManager, Warning, TEXT("[AdSMS][NativeTexture] RequestID cannot be empty."));
		return;
	}

	if (TrimmedAdUnitID.IsEmpty())
	{
		UE_LOG(LogAdManager, Warning, TEXT("[AdSMS][NativeTexture] Ad Unit ID cannot be empty. RequestID=%s"), *TrimmedRequestID);
		return;
	}

	Width = FMath::Clamp(Width, 128, 2048);
	Height = FMath::Clamp(Height, 128, 2048);

	PendingNativeAdTextureRequests.Add(TrimmedRequestID);

#if PLATFORM_ANDROID
	FTCHARToUTF8 NativeAdUnitIDUtf8(*TrimmedAdUnitID);
	FTCHARToUTF8 RequestIDUtf8(*TrimmedRequestID);

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("[AdSMS][NativeTexture] Requesting independent Native Ad. RequestID=%s Size=%dx%d"),
		*TrimmedRequestID,
		Width,
		Height
	);

	JavaStringStringInt2Void(
		"AndroidThunkJava_LoadNativeAdTextureForRequest",
		NativeAdUnitIDUtf8.Get(),
		RequestIDUtf8.Get(),
		Width,
		Height
	);

	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(NativeAdTextureRequestsReadyCheckTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				NativeAdTextureRequestsReadyCheckTimerHandle,
				this,
				&UAdManager::CheckNativeAdTextureRequestsReady,
				0.25f,
				true
			);
		}
	}
#else
	const int32 PreviewWidth = Width;
	const int32 PreviewHeight = Height;
	UTexture2D* PreviewTexture = UTexture2D::CreateTransient(PreviewWidth, PreviewHeight, PF_B8G8R8A8);
	if (PreviewTexture && PreviewTexture->GetPlatformData() && PreviewTexture->GetPlatformData()->Mips.Num() > 0)
	{
		FTexture2DMipMap& Mip = PreviewTexture->GetPlatformData()->Mips[0];
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		uint8* Pixels = static_cast<uint8*>(Data);

		const uint32 Hash = GetTypeHash(TrimmedRequestID);
		const uint8 R = static_cast<uint8>(40 + (Hash & 0x7F));
		const uint8 G = static_cast<uint8>(70 + ((Hash >> 8) & 0x7F));
		const uint8 B = static_cast<uint8>(90 + ((Hash >> 16) & 0x7F));

		for (int32 Y = 0; Y < PreviewHeight; ++Y)
		{
			for (int32 X = 0; X < PreviewWidth; ++X)
			{
				const int32 Index = (Y * PreviewWidth + X) * 4;
				const bool Stripe = ((X / 64) + (Y / 64)) % 2 == 0;
				Pixels[Index + 0] = Stripe ? B : static_cast<uint8>(B / 2);
				Pixels[Index + 1] = Stripe ? G : static_cast<uint8>(G / 2);
				Pixels[Index + 2] = Stripe ? R : static_cast<uint8>(R / 2);
				Pixels[Index + 3] = 255;
			}
		}

		Mip.BulkData.Unlock();
		PreviewTexture->SRGB = true;
		PreviewTexture->NeverStream = true;
		PreviewTexture->UpdateResource();
		NativeAdTexturesByRequest.Add(TrimmedRequestID, PreviewTexture);
		PendingNativeAdTextureRequests.Remove(TrimmedRequestID);
		OnNativeAdTextureReadyForRequest.Broadcast(TrimmedRequestID, PreviewTexture);
	}
#endif
}

void UAdManager::CheckNativeAdTextureRequestsReady()
{
#if PLATFORM_ANDROID
	TArray<FString> RequestsToRemove;

	for (const FString& RequestID : PendingNativeAdTextureRequests)
	{
		if (!JavaStringBool("AndroidThunkJava_IsNativeAdTextureReadyForRequest", RequestID))
		{
			if (!JavaStringBool("AndroidThunkJava_IsNativeAdTextureRequestActive", RequestID))
			{
				RequestsToRemove.Add(RequestID);
				UE_LOG(LogAdManager, Warning, TEXT("[AdSMS][ANDROID] Independent Native Ad request finished without an ad. RequestID=%s"), *RequestID);
			}
			continue;
		}

		const FString Path = JavaStringArg("AndroidThunkJava_GetNativeAdTexturePathForRequest", RequestID);
		if (Path.IsEmpty())
		{
			continue;
		}

		if (LoadNativeAdTextureFromFileForRequest(RequestID, Path))
		{
			RequestsToRemove.Add(RequestID);
			UE_LOG(
				LogAdManager,
				Log,
				TEXT("[AdSMS][ANDROID] Independent Native Ad texture imported. RequestID=%s"),
				*RequestID
			);
			OnNativeAdTextureReadyForRequest.Broadcast(RequestID, NativeAdTexturesByRequest[RequestID]);
		}
	}

	for (const FString& RequestID : RequestsToRemove)
	{
		PendingNativeAdTextureRequests.Remove(RequestID);
	}

	if (PendingNativeAdTextureRequests.Num() == 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(NativeAdTextureRequestsReadyCheckTimerHandle);
		}
	}
#endif
}

bool UAdManager::LoadNativeAdTextureFromFileForRequest(const FString& RequestID, const FString& FilePath)
{
	TArray<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *FilePath) || CompressedData.Num() == 0)
	{
		UE_LOG(LogAdManager, Warning, TEXT("[AdSMS] Failed to read native ad texture file for RequestID=%s: %s"), *RequestID, *FilePath);
		return false;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
	{
		return false;
	}

	TArray<uint8> RawBGRA;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA))
	{
		return false;
	}

	const int32 Width = Wrapper->GetWidth();
	const int32 Height = Wrapper->GetHeight();
	if (Width <= 0 || Height <= 0 || RawBGRA.Num() != Width * Height * 4)
	{
		return false;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, RawBGRA.GetData(), RawBGRA.Num());
	Mip.BulkData.Unlock();

	Texture->SRGB = true;
	Texture->NeverStream = true;
	Texture->UpdateResource();
	NativeAdTexturesByRequest.Add(RequestID, Texture);
	return true;
}

void UAdManager::ReleaseNativeAdTextureForRequest(const FString& RequestID)
{
	const FString TrimmedRequestID = RequestID.TrimStartAndEnd();
	if (TrimmedRequestID.IsEmpty())
	{
		return;
	}

	PendingNativeAdTextureRequests.Remove(TrimmedRequestID);
	NativeAdTexturesByRequest.Remove(TrimmedRequestID);

#if PLATFORM_ANDROID
	JavaStringVoidArg("AndroidThunkJava_ReleaseNativeAdTextureForRequest", TrimmedRequestID);
#endif

	if (PendingNativeAdTextureRequests.Num() == 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(NativeAdTextureRequestsReadyCheckTimerHandle);
		}
	}
}















void UAdManager::LoadInterstitial()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("LoadInterstitial() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID
	// If Android already has an ad, do not request another one.
	if (JavaBool("AndroidThunkJava_IsInterstitialAdAvailable"))
	{
		bInterstitialReady = true;

		UE_LOG(
			LogAdManager,
			Log,
			TEXT("Interstitial is already READY. No new load request needed.")
		);

		return;
	}

	if (JavaBool("AndroidThunkJava_IsInterstitialAdRequested"))
	{
		UE_LOG(
			LogAdManager,
			Log,
			TEXT("Interstitial load is already in progress.")
		);

		return;
	}

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("Loading Interstitial Ad through AdSMS JNI.")
	);

	JavaStringVoid(
		"AndroidThunkJava_LoadInterstitialAd",
		"ca-app-pub-3940256099942544/1033173712"
	);

	bInterstitialReady = false;
#else
	UE_LOG(LogAdManager, Log, TEXT("Interstitial ads are Android-only. Windows load skipped."));
#endif
}

void UAdManager::ShowInterstitial()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("ShowInterstitial() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID

	// Case 1: the next ad is already loaded.
	if (JavaBool("AndroidThunkJava_IsInterstitialAdAvailable"))
	{
		StopInterstitialReadyCheck();

		bInterstitialReady = true;
		bWaitingToShowInterstitial = false;

		UE_LOG(
			LogAdManager,
			Log,
			TEXT("Interstitial READY -> SHOWING NOW.")
		);

		JavaVoid("AndroidThunkJava_ShowInterstitialAd");

		bInterstitialReady = false;

		UE_LOG(
			LogAdManager,
			Log,
			TEXT("Interstitial shown. Android will preload the next Interstitial after dismissal.")
		);

		return;
	}

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("Interstitial is NOT ready. Requesting it and waiting to show...")
	);

	bWaitingToShowInterstitial = true;

	LoadInterstitial();
	StartInterstitialReadyCheck();
#else

	UE_LOG(LogAdManager, Log, TEXT("Interstitial ads are Android-only. Windows show skipped."));

#endif
}

bool UAdManager::IsInterstitialReady() const
{
#if PLATFORM_ANDROID
	return JavaBool("AndroidThunkJava_IsInterstitialAdAvailable");
#else
	return false;
#endif
}

bool UAdManager::IsInterstitialRequested() const
{
#if PLATFORM_ANDROID
	return JavaBool("AndroidThunkJava_IsInterstitialAdRequested");
#else
	return false;
#endif
}

void UAdManager::StartInterstitialReadyCheck()
{
	if (!bAdsInitialized || !bWaitingToShowInterstitial)
	{
		return;
	}

	if (!GEngine)
	{
		UE_LOG(LogAdManager, Warning, TEXT("Cannot start Interstitial readiness check: GEngine is null."));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		this,
		EGetWorldErrorMode::ReturnNull
	);

	if (!World)
	{
		UE_LOG(LogAdManager, Warning, TEXT("Cannot start Interstitial readiness check: World is not available."));
		return;
	}

	if (World->GetTimerManager().IsTimerActive(InterstitialReadyCheckTimerHandle))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		InterstitialReadyCheckTimerHandle,
		this,
		&UAdManager::CheckPendingInterstitialShow,
		0.25f,
		true
	);

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("Interstitial readiness polling started (250 ms).")
	);
}

void UAdManager::StopInterstitialReadyCheck()
{
	if (!GEngine)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		this,
		EGetWorldErrorMode::ReturnNull
	);

	if (World)
	{
		World->GetTimerManager().ClearTimer(InterstitialReadyCheckTimerHandle);
	}
}

void UAdManager::CheckPendingInterstitialShow()
{
#if PLATFORM_ANDROID

	if (!bAdsInitialized || !bWaitingToShowInterstitial)
	{
		StopInterstitialReadyCheck();
		return;
	}

 	if (!JavaBool("AndroidThunkJava_IsInterstitialAdAvailable"))
	{
		// Keep waiting. This timer deliberately does NOT stop after one failed check.
		return;
	}

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("Interstitial became READY after button request.")
	);

	StopInterstitialReadyCheck();

	bWaitingToShowInterstitial = false;
	bInterstitialReady = true;

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("SHOWING INTERSTITIAL after successful load.")
	);

	JavaVoid("AndroidThunkJava_ShowInterstitialAd");

	bInterstitialReady = false;

	UE_LOG(
		LogAdManager,
		Log,
		TEXT("Interstitial consumed. Android will preload the next one after dismissal.")
	);

#endif
}

// ============================================================
// REWARDED
// ============================================================

void UAdManager::LoadRewarded()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("LoadRewarded() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID
	if (JavaBool("AndroidThunkJava_IsRewardedAdAvailable"))
	{
		bRewardedReady = true;
		UE_LOG(LogAdManager, Log, TEXT("Rewarded is already READY."));
		return;
	}

	if (JavaBool("AndroidThunkJava_IsRewardedAdRequested"))
	{
		UE_LOG(LogAdManager, Log, TEXT("Rewarded load is already in progress."));
		return;
	}

	UE_LOG(LogAdManager, Log, TEXT("Loading Rewarded Ad. Using Google test rewarded ID."));
	JavaStringVoid(
        "AndroidThunkJava_LoadRewardedAd",
        "ca-app-pub-3940256099942544/5224354917"
    );
	bRewardedReady = false;
#else
	UE_LOG(LogAdManager, Log, TEXT("Rewarded ads are Android-only. Windows load skipped."));
#endif
}

void UAdManager::ShowRewarded()
{
	if (!bAdsInitialized)
	{
		UE_LOG(LogAdManager, Warning, TEXT("ShowRewarded() called before InitializeAds()."));
		return;
	}

#if PLATFORM_ANDROID
	if (!JavaBool("AndroidThunkJava_IsRewardedAdAvailable"))
	{
		UE_LOG(LogAdManager, Log, TEXT("Rewarded is not ready. Requesting it and waiting for READY."));
		bWaitingToShowRewarded = true;
		LoadRewarded();
		StartRewardedReadyCheck();
		return;
	}

	StopRewardedReadyCheck();
	bWaitingToShowRewarded = false;

	UE_LOG(LogAdManager, Log, TEXT("Showing Rewarded Ad."));
	JavaVoid("AndroidThunkJava_ClearRewardedEarned");
	JavaVoid("AndroidThunkJava_ShowRewardedAd");
	bRewardedReady = false;
	StartRewardedCompletionCheck();
#else
	UE_LOG(LogAdManager, Log, TEXT("Rewarded ads are Android-only. Windows show skipped."));
#endif
}

bool UAdManager::IsRewardedReady() const
{
#if PLATFORM_ANDROID
	return JavaBool("AndroidThunkJava_IsRewardedAdAvailable");
#else
	return false;
#endif
}

bool UAdManager::IsRewardedRequested() const
{
#if PLATFORM_ANDROID
	return JavaBool("AndroidThunkJava_IsRewardedAdRequested");
#else
	return false;
#endif
}

void UAdManager::CheckPendingRewardedShow()
{
#if PLATFORM_ANDROID
	if (!bAdsInitialized || !bWaitingToShowRewarded)
	{
		StopRewardedReadyCheck();
		return;
	}

	if (!JavaBool("AndroidThunkJava_IsRewardedAdAvailable"))
	{
		return;
	}

	StopRewardedReadyCheck();
	bWaitingToShowRewarded = false;
	bRewardedReady = true;

	UE_LOG(LogAdManager, Log, TEXT("Rewarded became READY after button request. Showing now."));
	JavaVoid("AndroidThunkJava_ClearRewardedEarned");
	JavaVoid("AndroidThunkJava_ShowRewardedAd");
	bRewardedReady = false;
	StartRewardedCompletionCheck();
#endif
}

void UAdManager::StartRewardedReadyCheck()
{
#if PLATFORM_ANDROID
	if (!GEngine) return;

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::ReturnNull);
	if (!World) return;

	if (World->GetTimerManager().IsTimerActive(RewardedReadyCheckTimerHandle)) return;

	World->GetTimerManager().SetTimer(
		RewardedReadyCheckTimerHandle,
		this,
		&UAdManager::CheckPendingRewardedShow,
		0.25f,
		true
	);

	UE_LOG(LogAdManager, Log, TEXT("Rewarded readiness polling started (250 ms)."));
#endif
}

void UAdManager::StopRewardedReadyCheck()
{
#if PLATFORM_ANDROID
	if (!GEngine) return;

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::ReturnNull);
	if (World) World->GetTimerManager().ClearTimer(RewardedReadyCheckTimerHandle);
#endif
}

void UAdManager::CheckRewardedCompletion()
{
#if PLATFORM_ANDROID
	if (!bAdsInitialized)
	{
		StopRewardedCompletionCheck();
		return;
	}

	if (!JavaBool("AndroidThunkJava_IsRewardedAdEarned"))
	{
		if (!JavaBool("AndroidThunkJava_IsRewardedAdShowing"))
		{
			StopRewardedCompletionCheck();
		}

		return;
	}

	const int32 RewardAmount = JavaInt("AndroidThunkJava_GetRewardedAmount");
	const FString RewardType = JavaString("AndroidThunkJava_GetRewardedType");

	StopRewardedCompletionCheck();

	if (RewardHandler && RewardAmount > 0)
	{
		UE_LOG(
			LogAdManager,
			Log,
			TEXT("Rewarded completion received. Granting reward: %d %s"),
			RewardAmount,
			*RewardType
		);

		RewardHandler->GiveReward(RewardAmount, RewardType);
	}
	else
	{
		UE_LOG(LogAdManager, Warning, TEXT("Rewarded completion received, but reward data is invalid."));
	}

	JavaVoid("AndroidThunkJava_ConsumeRewardedEarned");
#endif
}

void UAdManager::StartRewardedCompletionCheck()
{
#if PLATFORM_ANDROID
	if (!GEngine) return;

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::ReturnNull);
	if (!World) return;

	if (World->GetTimerManager().IsTimerActive(RewardedCompletionCheckTimerHandle)) return;

	World->GetTimerManager().SetTimer(
		RewardedCompletionCheckTimerHandle,
		this,
		&UAdManager::CheckRewardedCompletion,
		0.25f,
		true
	);

	UE_LOG(LogAdManager, Log, TEXT("Rewarded completion polling started (250 ms)."));
#endif
}

void UAdManager::StopRewardedCompletionCheck()
{
#if PLATFORM_ANDROID
	if (!GEngine) return;

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::ReturnNull);
	if (World) World->GetTimerManager().ClearTimer(RewardedCompletionCheckTimerHandle);
#endif
}

bool UAdManager::IsAdsInitialized() const
{
	return bAdsInitialized;
}







UAdRewardHandler* UAdManager::GetRewardHandler() const
{
	return RewardHandler;
}