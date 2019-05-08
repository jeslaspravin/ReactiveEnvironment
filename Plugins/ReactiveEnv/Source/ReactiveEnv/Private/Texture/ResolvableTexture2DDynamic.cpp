// Fill out your copyright notice in the Description page of Project Settings.

#include "ResolvableTexture2DDynamic.h"
#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif

UResolvableTexture2DDynamic::UResolvableTexture2DDynamic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsResolveTarget = 1;
	NumMips = 1;
}

#if WITH_EDITOR
void UResolvableTexture2DDynamic::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	EPixelFormat pixelFormat = Format = getPixelFormatFromResolveTextureFormat(textureFormat);

	const int32 WarnSize = 2048;

	SizeX = sizeX;
	SizeY = sizeY;

	if (SizeX > WarnSize || SizeY > WarnSize)
	{
		const float MemoryMb = SizeX * SizeY * GPixelFormats[Format].BlockBytes / 1024.0f / 1024.0f;
		FNumberFormattingOptions FloatFormat;
		FloatFormat.SetMaximumFractionalDigits(1);
		FText Message = FText::Format(NSLOCTEXT("ResolvingTexture", "LargeResolvingTextureWarning", "A Texture of size {0}x{1} will use {2}Mb ({3}Mb), which may result in extremely poor performance or an Out Of Video Memory crash.\nAre you sure?"), FText::AsNumber(SizeX), FText::AsNumber(SizeY), FText::AsNumber(MemoryMb, &FloatFormat), FText::AsNumber(10.0f * MemoryMb, &FloatFormat));
		const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, Message);

		if (Choice == EAppReturnType::No)
		{
			SizeX = FMath::Clamp<int32>(SizeX, 1, WarnSize);
			SizeY = FMath::Clamp<int32>(SizeY, 1, WarnSize);
		}
	}

	const int32 MaxSize = 8192;

	SizeX = sizeX=FMath::Clamp<int32>(SizeX - (SizeX % GPixelFormats[pixelFormat].BlockSizeX), 1, MaxSize);
	SizeY = sizeY=FMath::Clamp<int32>(SizeY - (SizeY % GPixelFormats[pixelFormat].BlockSizeY), 1, MaxSize);

	// Always set SRGB back to 'on'; it will be turned off again in the call to Super::PostEditChangeProperty below if necessary
	if (PropertyChangedEvent.Property)
	{
		SRGB = true;
	}

	SamplerAddressMode = getSamplerAddressMode(addressMode);

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UResolvableTexture2DDynamic::PostLoad()
{
	Super::PostLoad();
}

void UResolvableTexture2DDynamic::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("ResolvableTexture2D Serialize"));
#endif
	// Update the parent information 
	EPixelFormat pixelFormat = Format = getPixelFormatFromResolveTextureFormat(textureFormat);
	SamplerAddressMode = getSamplerAddressMode(addressMode);

	float OriginalSizeX = SizeX = sizeX;
	float OriginalSizeY = SizeY = sizeY;

	if (!FPlatformProperties::SupportsWindowedMode())
	{
		// Clamp the render target size in order to avoid reallocating the scene render targets,
		// before the FTextureRenderTarget2DResource() is created in Super::PostLoad().
		SizeX = FMath::Min<int32>(SizeX, GSystemResolution.ResX);
		SizeY = FMath::Min<int32>(SizeY, GSystemResolution.ResY);
	}

	SizeX = FMath::Min<int32>(SizeX, GTextureRenderTarget2DMaxSizeX);
	SizeY = FMath::Min<int32>(SizeY, GTextureRenderTarget2DMaxSizeY);

	// Maintain aspect ratio if clamped
	if (SizeX != OriginalSizeX || SizeY != OriginalSizeY)
	{
		float ScaleX = SizeX / OriginalSizeX;
		float ScaleY = SizeY / OriginalSizeY;

		if (ScaleX < ScaleY)
		{
			SizeY = OriginalSizeY * ScaleX;
		}
		else
		{
			SizeX = OriginalSizeX * ScaleY;
		}
	}

	sizeX = SizeX;
	sizeY = SizeY;
}

float UResolvableTexture2DDynamic::GetSurfaceWidth() const
{
	return sizeX;
}

float UResolvableTexture2DDynamic::GetSurfaceHeight() const
{
	return sizeY;
}
