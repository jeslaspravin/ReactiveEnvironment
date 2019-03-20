// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"
#include "ResolvableTexture2DDynamic.generated.h"


/** Subset of EPixelFormat exposed to UResolvableTexture2DDynamic */
UENUM()
enum EResolveTextureFormat
{
	/** R channel, 8 bit per channel fixed point, range [0, 1]. */
	RsTF_R8,
	/** RG channels, 8 bit per channel fixed point, range [0, 1]. */
	RsTF_RG8,
	/** RGBA channels, 8 bit per channel fixed point, range [0, 1]. */
	RsTF_RGBA8,
	/** R channel, 16 bit per channel floating point, range [-65504, 65504] */
	RsTF_R16f,
	/** RG channels, 16 bit per channel floating point, range [-65504, 65504] */
	RsTF_RG16f,
	/** RGBA channels, 16 bit per channel floating point, range [-65504, 65504] */
	RsTF_RGBA16f,
	/** R channel, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
	RsTF_R32f,
	/** RG channels, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
	RsTF_RG32f,
	/** RGBA channels, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
	RsTF_RGBA32f,
	/** RGBA channels, 10 bit per channel fixed point and 2 bit of alpha */
	RsTF_RGB10A2
};

inline EPixelFormat getPixelFormatFromResolveTextureFormat(EResolveTextureFormat RsTFormat)
{
	switch (RsTFormat)
	{
	case RsTF_R8: return PF_G8; break;
	case RsTF_RG8: return PF_R8G8; break;
	case RsTF_RGBA8: return PF_B8G8R8A8; break;

	case RsTF_R16f: return PF_R16F; break;
	case RsTF_RG16f: return PF_G16R16F; break;
	case RsTF_RGBA16f: return PF_FloatRGBA; break;

	case RsTF_R32f: return PF_R32_FLOAT; break;
	case RsTF_RG32f: return PF_G32R32F; break;
	case RsTF_RGBA32f: return PF_A32B32G32R32F; break;
	case RsTF_RGB10A2: return PF_A2B10G10R10; break;
	}

	ensureMsgf(false, TEXT("Unhandled EResolveTextureFormat entry %u"), (uint32)RsTFormat);
	return PF_Unknown;
}

inline ESamplerAddressMode getSamplerAddressMode(TextureAddress RsTAddress)
{
	switch (RsTAddress)
	{
	case TA_Wrap: return ESamplerAddressMode::AM_Wrap; break;
	case TA_Clamp:return ESamplerAddressMode::AM_Clamp; break;
	case TA_Mirror:return ESamplerAddressMode::AM_Mirror; break;
	}


	ensureMsgf(false, TEXT("Unhandled TextureAddress entry %u"), (uint32)RsTAddress);
	return ESamplerAddressMode::AM_Clamp;
}

/**
 * 
 */
UCLASS(hidecategories = Object, hidecategories = Texture, hidecategories = Compression, hidecategories = Adjustments, hidecategories = Compositing)
class REACTIVEENV_API UResolvableTexture2DDynamic : public UTexture2DDynamic
{
	GENERATED_BODY()

protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;

	virtual void Serialize(FArchive& Ar) override;

public:

	UResolvableTexture2DDynamic(const FObjectInitializer& ObjectInitializer);

	/** The width of the texture.(Resize at runtime is restricted) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ResolvableTexture, AssetRegistrySearchable)
		int32 sizeX;

	/** The height of the texture.(Resize at runtime is restricted) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ResolvableTexture, AssetRegistrySearchable)
		int32 sizeY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ResolvableTexture, AssetRegistrySearchable)
		TEnumAsByte<enum TextureAddress> addressMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ResolvableTexture, AssetRegistrySearchable)
		TEnumAsByte<enum EResolveTextureFormat> textureFormat;

	void setSizeX(int32 size) { sizeX = SizeX = size; }

	void setSizeY(int32 size) { sizeY = SizeY = size; }

	virtual float GetSurfaceWidth() const override;

	virtual float GetSurfaceHeight() const override;
};
