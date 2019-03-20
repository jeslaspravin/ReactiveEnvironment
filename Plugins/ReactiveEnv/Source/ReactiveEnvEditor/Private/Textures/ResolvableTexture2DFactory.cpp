// Fill out your copyright notice in the Description page of Project Settings.

#include "ResolvableTexture2DFactory.h"
#include "Texture/ResolvableTexture2DDynamic.h"

UResolvableTexture2DFactory::UResolvableTexture2DFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = false;
	SupportedClass = UResolvableTexture2DDynamic::StaticClass();
}

UResolvableTexture2DFactory::~UResolvableTexture2DFactory()
{

}

UObject* UResolvableTexture2DFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	auto NewTexture = NewObject<UResolvableTexture2DDynamic>(InParent,InClass, InName, Flags);
	if (NewTexture != NULL)
	{
		NewTexture->Filter = TF_Default;
		NewTexture->SamplerAddressMode = AM_Clamp;
		NewTexture->addressMode = TA_Clamp;
		NewTexture->SRGB = true;

		// Disable compression
		NewTexture->CompressionSettings = TC_Default;
#if WITH_EDITORONLY_DATA
		NewTexture->CompressionNone = true;
		NewTexture->MipGenSettings = TMGS_NoMipmaps;
		NewTexture->CompressionNoAlpha = true;
		NewTexture->DeferCompression = false;
#endif // #if WITH_EDITORONLY_DATA

		NewTexture->bNoTiling = false;

		NewTexture->sizeX = 256;
		NewTexture->sizeY = 256;
		NewTexture->textureFormat = EResolveTextureFormat::RsTF_RGBA8;
		NewTexture->Init(256, 256, EPixelFormat::PF_B8G8R8A8, true);
	}
	return NewTexture;
}