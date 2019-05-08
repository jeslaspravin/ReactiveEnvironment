
#include "ReactiveEnvEditor.h"
#include "ReactiveEnvActor.h"
#include "EditorViewportClient.h"
#include "Editor.h"
#include "AssetToolsModule.h"
#include "Textures/AssetTypeActions_ResolvableTexture2D.h"
#include "ReactiveEnvEditorSharedPCH.h"

IMPLEMENT_MODULE(FReactiveEnvEditor, ReactiveEnvEditor)

DEFINE_LOG_CATEGORY(LogReactiveEnvEditor)

void FReactiveEnvEditor::StartupModule()
{
	LOGGER(TEXT("Starting reactive environment editor module"));
	reactiveEnvConsoleCmds.Add(IConsoleManager::Get().RegisterConsoleCommand(TEXT("dumpRTs"), TEXT("Dumps the RTs in reactive environment actor"),
		FConsoleCommandDelegate::CreateRaw(this, &FReactiveEnvEditor::onDumpRTs)));

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		TSharedRef<IAssetTypeActions> resolvableTextureAssetTypeAction = MakeShareable(new FAssetTypeActions_ResolvableTexture2D());
		assetTools.RegisterAssetTypeActions(resolvableTextureAssetTypeAction);
	}
}

void FReactiveEnvEditor::ShutdownModule()
{
	for (IConsoleCommand* cmd : reactiveEnvConsoleCmds)
	{
		IConsoleManager::Get().UnregisterConsoleObject(cmd);
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (TSharedPtr<IAssetTypeActions> assetTypeAct : registeredAssetTypeActions)
		{
			assetTools.UnregisterAssetTypeActions(assetTypeAct.ToSharedRef());
			assetTypeAct.Reset();
		}
		registeredAssetTypeActions.Empty();
	}

	reactiveEnvConsoleCmds.Empty();
}

void FReactiveEnvEditor::onDumpRTs()
{

	if (GEditor && GEditor->GetAllViewportClients().Num()>0)
	{
		for(FEditorViewportClient* viewportClient : GEditor->GetAllViewportClients())
		{
			UWorld* world= viewportClient->GetWorld();
			if (world && AReactiveEnvActor::getInstance() && world->IsEditorWorld())
			{
				AReactiveEnvActor* reactiveActor = AReactiveEnvActor::getInstance();
				reactiveActor->dumpRenderTargets();
				return;
			}
		}
	}
}

