#include "Textures/AssetTypeActions_ResolvableTexture2D.h"
#include "Texture/ResolvableTexture2DDynamic.h"
#include "Interfaces/ITextureEditorModule.h"

uint32 FAssetTypeActions_ResolvableTexture2D::GetCategories()
{
	return EAssetTypeCategories::MaterialsAndTextures;
}

FText FAssetTypeActions_ResolvableTexture2D::GetName() const
{
	return NSLOCTEXT("ResolvableTexture2DDynamic", "EditorMenuName", "Resolvable Texture 2D");
}

UClass* FAssetTypeActions_ResolvableTexture2D::GetSupportedClass() const
{
	return UResolvableTexture2DDynamic::StaticClass();
}

FColor FAssetTypeActions_ResolvableTexture2D::GetTypeColor() const
{
	return FColor(128, 64, 64);
}

void FAssetTypeActions_ResolvableTexture2D::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Texture = Cast<UResolvableTexture2DDynamic>(*ObjIt);
		if (Texture != NULL)
		{
			ITextureEditorModule* TextureEditorModule = &FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
			TextureEditorModule->CreateTextureEditor(Mode, EditWithinLevelEditor, Texture);
		}
	}
}
