// Fill out your copyright notice in the Description page of Project Settings.

#include "ReactiveEnvActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Async/ParallelFor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "ReactiveEnvSharedPCH.h"
#include "ReactiveCompute.h"

DEFINE_LOG_CATEGORY(LogReactiveEnvActor)

DECLARE_CYCLE_STAT(TEXT("ReactiveEnv --> Capture List update"), STAT_CaptureListUpdate, STATGROUP_ReactiveEnv);
DECLARE_CYCLE_STAT(TEXT("ReactiveEnv --> Compute"), STAT_ReactiveCompute, STATGROUP_ReactiveEnv);


TWeakObjectPtr<AReactiveEnvActor> AReactiveEnvActor::instance= nullptr;

void AReactiveEnvActor::drawAllImpacts()
{
	if(impactsToDraw.Num()>0)
	{
		UCanvas* canvas;
		FVector2D canvasSize;
		FDrawToRenderTargetContext context;

		UKismetRenderingLibrary::ClearRenderTarget2D(this, impactsCaptureRT);

		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, impactsCaptureRT, canvas, canvasSize, context);
		
		for (int i = 0; i < impactsToDraw.Num(); i++)
		{
			FImpactData& impactData = impactsToDraw[i];

			FVector2D impactLocal = FVector2D(impactData.impactPoint - GetActorLocation());
			impactLocal = (impactLocal + (reactiveDistance*0.5f)) / reactiveDistance;// Gives normalized UV in XY coord

			if (impactLocal.X>=0 && impactLocal.X <= 1 && impactLocal.Y >= 0 && impactLocal.Y <= 1)
			{
				FVector2D drawAtLoc = impactLocal*impactsCaptureRT->SizeX;
				float distanceScale = reactiveDistance / (float)impactsCaptureRT->SizeX;
				float drawSize = 2 * impactData.size / distanceScale;// 2 x just to increase radius size to screen size
				drawAtLoc = (drawAtLoc - (drawSize*0.5f));// Had to do this as pivot in draw texture is not used for anchoring.
				canvas->K2_DrawTexture(impactData.drawTexture, drawAtLoc, FVector2D(drawSize, drawSize), FVector2D::ZeroVector);
			}
		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, context);
		impactsToDraw.Reset();
	}
}


void AReactiveEnvActor::clearResolvedTargets()
{
	waterHeightFieldNormalTexture->UpdateResource();
	snowMudTexture->UpdateResource();
	waterHeightFieldTexture->UpdateResource();
}

// Sets default values
AReactiveEnvActor::AReactiveEnvActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("Root"));

	landWaterSceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("LandWaterSceneCapture"));
	landWaterSceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	landWaterSceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	landWaterSceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	landWaterSceneCaptureComponent->bCaptureEveryFrame = true;
	landWaterSceneCaptureComponent->bCaptureOnMovement = false;
	landWaterSceneCaptureComponent->SetupAttachment(RootComponent);
	/*So that Texture rendered out will be of this coordinate,have to multiply with (1,1) to get proper texture relative coord for local coord
	*			-Y
	*			|
	*	UV(0,0)	|
	*-x			|			+x
	*----------------------
	*			|
	*			|
	*			|	UV(1,1)
	*			+y
	*/
	landWaterSceneCaptureComponent->SetWorldRotation(FRotator(-90, -90, 0));

	snowMudSceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("SnowMudSceneCapture"));
	snowMudSceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	snowMudSceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	snowMudSceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	snowMudSceneCaptureComponent->bCaptureEveryFrame = true;
	snowMudSceneCaptureComponent->bCaptureOnMovement = false;
	snowMudSceneCaptureComponent->SetupAttachment(RootComponent);
	/*So that Texture rendered out will be of this coordinate,have to multiply with (-1,1) to get proper texture relative coord for local coord
	*			-Y
	*			|
	*			|  UV(0,0)
	*-x			|			+x
	*----------------------
	*			|
	*			|
	*	UV(1,1)	|
	*			+y
	*/
	snowMudSceneCaptureComponent->SetWorldRotation(FRotator(90, 90, 0));
}

// Called when the game starts or when spawned
void AReactiveEnvActor::BeginPlay()
{
	Super::BeginPlay();

	if (instance.IsValid())
		return;

	instance = this;

	createMaterialRTs();
	clearResolvedTargets();

	if (reactiveEnvMPC)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, reactiveEnvMPC, MPC_REACTIVE_DISTANCE, reactiveDistance);	

		check(reactiveRTResolution > 0);
		// From Marvel Master Infinite Reactive Water Plugin
		noiseReducingFactor = (40.0f / (reactiveRTResolution*reactiveRTResolution))*(reactiveRTResolution*256.0f);
		UKismetMaterialLibrary::SetScalarParameterValue(this, reactiveEnvMPC, MPC_REACTIVE_WATER_NOISE_RED_FACT, noiseReducingFactor);
	}

	// From Marvel Master Infinite Reactive Water Plugin
	travelSpeed = waveTravelSpeed / 100.f;
	travelSpeed = FMath::Clamp(travelSpeed,0.0f,travelSpeed/FMath::Clamp(reactiveRTResolution / 1024.0f, 0.0f, 999.0f));

	if (GetWorld())
	{
		FOnActorSpawned::FDelegate actorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &AReactiveEnvActor::onActorSpawned);
		actorSpawnedHandler = GetWorld()->AddOnActorSpawnedHandler(actorSpawnedDelegate);
		for (TActorIterator<AActor> actorItr(GetWorld()); actorItr; ++actorItr)
		{
			captureActorData(*actorItr,false);
		}
		// Update everything and sync with the scene capture
		updateAllCapturableActors();
	}

	if (landWaterSceneCaptureComponent->TextureTarget != landWaterCaptureRT)
	{
		landWaterSceneCaptureComponent->TextureTarget = landWaterCaptureRT;
	}

	if (snowMudSceneCaptureComponent->TextureTarget != snowMudCaptureRT)
	{
		snowMudSceneCaptureComponent->TextureTarget = snowMudCaptureRT;
	}

	reactiveCompute = new FReactiveCompute(this,GetWorld()->Scene->GetFeatureLevel());


	ACharacter* player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (player)
	{
		SetActorLocation(player->GetActorLocation());
	}
}

void AReactiveEnvActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetWorld() && actorSpawnedHandler.IsValid())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(actorSpawnedHandler);
	}
	for (TMap<TWeakObjectPtr<AActor>, FCapturingActorData>::TIterator itr = actorCapturingMap.CreateIterator(); itr; ++itr)
	{
		itr.Key()->OnDestroyed.RemoveAll(this);
	}
	actorCapturingMap.Empty();
}

void AReactiveEnvActor::BeginDestroy()
{
	Super::BeginDestroy();

	if (reactiveCompute)
		delete reactiveCompute;
	reactiveCompute = nullptr;

	if (instance == this)
		instance = nullptr;

	destroyMaterialRTs();
}

void AReactiveEnvActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	landWaterSceneCaptureComponent->OrthoWidth = reactiveDistance;
	landWaterSceneCaptureComponent->SetRelativeLocation(FVector(0, 0, topSceneCaptureOffset));

	/*So that Texture rendered out will be of this coordinate,have to multiply with (1,1) to get proper texture relative coord for local coord
	*			-Y
	*			|
	*	UV(0,0)	| 
	*-x			|			+x
	*----------------------
	*			|
	*			|
	*			|	UV(1,1)
	*			+y
	*/
	landWaterSceneCaptureComponent->SetWorldRotation(FRotator(-90, -90, 0));


	snowMudSceneCaptureComponent->OrthoWidth = reactiveDistance;
	snowMudSceneCaptureComponent->SetRelativeLocation(FVector(0, 0, -bottomSceneCaptureOffset));

	/*So that Texture rendered out will be of this coordinate,have to multiply with (-1,1) to get proper texture relative coord for local coord
	*			-Y
	*			|
	*			|  UV(0,0)
	*-x			|			+x
	*----------------------
	*			|
	*			|
	*	UV(1,1)	|
	*			+y
	*/
	snowMudSceneCaptureComponent->SetWorldRotation(FRotator(90, 90, 0));
}

// Called every frame
void AReactiveEnvActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO : Change in case this becomes buggy
	if (instance == this)
	{
		updateAllCapturableActors();
		ACharacter* player=UGameplayStatics::GetPlayerCharacter(this, 0);
		if(player)
		{
			REACTIVEENV_SCOPE_CYCLE_COUNTER(STAT_ReactiveCompute);

			//Relative Displacement of all other object with respect to player on texture is only working properly on snapping the player to texture texel
			float gridSize = reactiveDistance * (1.0f / reactiveRTResolution);
			displacementOffset =FVector(FMath::FloorToInt(player->GetActorLocation().X/gridSize),
				FMath::FloorToInt(player->GetActorLocation().Y / gridSize),0);
			displacementOffset = ((displacementOffset + 0.5f)*gridSize)-GetActorLocation();
			AddActorWorldOffset(displacementOffset);

			speedPerFrame = travelSpeed *travelSpeedMultiplier* DeltaTime;

			UKismetMaterialLibrary::SetVectorParameterValue(this, reactiveEnvMPC, MPC_REACTIVE_ORIGIN, FLinearColor(GetActorLocation()));
			// Start draw operations
			drawAllImpacts();
			if (reactiveCompute)
				reactiveCompute->computeReactive();
		}
	}
}

void AReactiveEnvActor::updateAllCapturableActors()
{
	REACTIVEENV_SCOPE_CYCLE_COUNTER(STAT_CaptureListUpdate);
	landWaterSceneCaptureComponent->ShowOnlyComponents.Reset();
	snowMudSceneCaptureComponent->ShowOnlyComponents.Reset();

	for (TArray<TWeakObjectPtr<UPrimitiveComponent>>::TIterator snowCompItr= snowMudCaptureComponents.CreateIterator();snowCompItr;++snowCompItr)
	{
		if (snowCompItr->IsValid())
		{
			snowMudSceneCaptureComponent->ShowOnlyComponents.Add(*snowCompItr);
		}
		else
			snowCompItr.RemoveCurrent();
	}

	for (TArray<TWeakObjectPtr<UPrimitiveComponent>>::TIterator waterCompItr = waterCaptureComponents.CreateIterator(); waterCompItr; ++waterCompItr)
	{
		if (waterCompItr->IsValid())
		{
			landWaterSceneCaptureComponent->ShowOnlyComponents.Add(*waterCompItr);
		}
		else
			waterCompItr.RemoveCurrent();
	}

	TArray<TWeakObjectPtr<AActor>> keys;

	int numTask = FMath::CeilToInt(actorCapturingMap.GetKeys(keys) / (float)updateActorPerTask);

	ParallelFor(numTask, [&keys,this](uint32 taskIdx) {
		uint32 startIndex = taskIdx * this->updateActorPerTask;
		uint32 endIndex = FMath::Min(startIndex + this->updateActorPerTask, (uint32)keys.Num());

		for (uint32 idx = startIndex; idx < endIndex; idx++)
		{
			this->updateToSceneCapture(this->actorCapturingMap[keys[idx]]);
		}
	});
}

void AReactiveEnvActor::onActorSpawned(AActor* actor)
{

	captureActorData(actor);
}

void AReactiveEnvActor::captureActorData(AActor* actor, bool bUpdateToSceneCapture/* = true*/)
{
	actor->OnDestroyed.AddDynamic(this, &AReactiveEnvActor::onActorDestroyed);
	bool bIsReactive = actor->Tags.Contains(REACTIVE);
	bool bIsReactiveIdle = actor->Tags.Contains(REACTIVE_IDLE);

	bool bIsReactiveSnowMud = actor->Tags.Contains(REACTIVE_SNOW_MUD);
	bool bIsReactiveWater = actor->Tags.Contains(REACTIVE_WATER);

	// Adding actors that have reactive tags and components that have reactive tags
	if (bIsReactiveIdle || bIsReactive)
	{
		FCapturingActorData actorData;
		actorData.bSkipOnIdleActor = bIsReactiveIdle;

		TInlineComponentArray<UPrimitiveComponent*> childComponents;
		actor->GetComponents<UPrimitiveComponent>(childComponents);
		for (TInlineComponentArray<UPrimitiveComponent*>::TIterator itr = childComponents.CreateIterator(); itr; ++itr)
		{
			bool bIsCompReactive = (*itr)->ComponentTags.Contains(REACTIVE);
			bool bIsCompReactiveLocal = (*itr)->ComponentTags.Contains(REACTIVE_LOCAL);

			if (bIsCompReactiveLocal || bIsCompReactive)
			{
				actorData.captureComponents.Add(*itr);
			}
		}

		FCapturingActorData& addedCaptureData=actorCapturingMap.Add(actor, actorData);

		if(bUpdateToSceneCapture)
			updateToSceneCapture(addedCaptureData);
	}
	else if( bIsReactiveSnowMud || bIsReactiveWater)// Find components that needs to be added to capture every frame
	{
		TInlineComponentArray<UPrimitiveComponent*> childComponents;
		actor->GetComponents<UPrimitiveComponent>(childComponents);
		bool bSetRTs = false;
		for (TInlineComponentArray<UPrimitiveComponent*>::TIterator itr = childComponents.CreateIterator(); itr; ++itr)
		{
			if ((*itr)->ComponentTags.Contains(REACTIVE_WATER))
			{
				waterCaptureComponents.Add(*itr);
				bSetRTs = true;
			}

			if ((*itr)->ComponentTags.Contains(REACTIVE_SNOW_MUD))
			{
				snowMudCaptureComponents.Add(*itr);
				bSetRTs = true;
			}

			if (bSetRTs)
			{
				// TODO : Set RTs for the primitive
				bSetRTs = false;
			}
		}
	}
}

void AReactiveEnvActor::onActorDestroyed(AActor* actor)
{
	actor->OnDestroyed.RemoveAll(this);
	actorCapturingMap.Remove(actor);
}

void AReactiveEnvActor::createMaterialRTs()
{
	if (!snowMudCaptureRT)
	{
		snowMudCaptureRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, reactiveRTResolution, reactiveRTResolution, RTF_R8);
	}
	reactiveRTResolution = snowMudCaptureRT->SizeX;

	if (!landWaterCaptureRT)
	{
		landWaterCaptureRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, reactiveRTResolution, reactiveRTResolution, RTF_R8);
	}
	reactiveRTResolution = reactiveRTResolution < landWaterCaptureRT->SizeX ? landWaterCaptureRT->SizeX : reactiveRTResolution;

	if (!impactsCaptureRT)
	{
		impactsCaptureRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, reactiveRTResolution, reactiveRTResolution, RTF_R8);
	}
	reactiveRTResolution = reactiveRTResolution < impactsCaptureRT->SizeX? landWaterCaptureRT->SizeX : reactiveRTResolution;
	// TODO : Create other Material RTs
}

void AReactiveEnvActor::destroyMaterialRTs()
{
	if (snowMudCaptureRT && !snowMudCaptureRT->IsAsset())
	{
		snowMudCaptureRT->ConditionalBeginDestroy();
		snowMudCaptureRT = nullptr;
	}

	if (landWaterCaptureRT && !landWaterCaptureRT->IsAsset())
	{
		landWaterCaptureRT->ConditionalBeginDestroy();
		landWaterCaptureRT = nullptr;
	}

	if (impactsCaptureRT && !impactsCaptureRT->IsAsset())
	{
		impactsCaptureRT->ConditionalBeginDestroy();
		impactsCaptureRT = nullptr;
	}
}

UTextureRenderTarget2D* AReactiveEnvActor::getReactiveWaterRT()
{
	return landWaterCaptureRT;
}

UTextureRenderTarget2D* AReactiveEnvActor::getReactiveSnowMudRT()
{
	return snowMudCaptureRT;
}

float AReactiveEnvActor::getWaveTravelSpeed()
{
	return speedPerFrame;
}

void AReactiveEnvActor::updateToSceneCapture(FCapturingActorData &captureData)
{
	if (captureData.bSkipOnIdleActor)
	{
		float actorVelocity = captureData.captureComponents.Num() > 0 && captureData.captureComponents[0].IsValid() ?
			captureData.captureComponents[0]->GetOwner()->GetVelocity().Size() : 0;
		if (actorVelocity >= minSpeedForIdling)
		{
			landWaterSceneCaptureComponent->ShowOnlyComponents.Append(captureData.captureComponents);
			snowMudSceneCaptureComponent->ShowOnlyComponents.Append(captureData.captureComponents);
		}
		else
		{
			for (TWeakObjectPtr<UPrimitiveComponent> weakCompPtr : captureData.captureComponents)
			{
				landWaterSceneCaptureComponent->ShowOnlyComponents.Remove(weakCompPtr);
				snowMudSceneCaptureComponent->ShowOnlyComponents.Remove(weakCompPtr);
			}
		}
	}
	else
	{
		for (TWeakObjectPtr<UPrimitiveComponent> weakCompPtr : captureData.captureComponents)
		{
			// If velocity is high enough or if has tag reactive local then add to visible components
			if (weakCompPtr.IsValid() && (weakCompPtr->GetComponentVelocity().Size() >= minSpeedForIdling 
				|| weakCompPtr->ComponentHasTag(REACTIVE_LOCAL)))
			{
				landWaterSceneCaptureComponent->ShowOnlyComponents.Add(weakCompPtr);
			}
			else
			{
				landWaterSceneCaptureComponent->ShowOnlyComponents.Remove(weakCompPtr);
			}
			// Mud and snow needs to be impacted even with out velocity
			snowMudSceneCaptureComponent->ShowOnlyComponents.Add(weakCompPtr);
		}
	}
}

void AReactiveEnvActor::addManualImpact(FImpactData impactData)
{
	if(getInstance())
		getInstance()->impactsToDraw.Add(impactData);
}

#if WITH_EDITOR

void AReactiveEnvActor::dumpRenderTargets()
{
	// TODO: Dumping logics
	UE_LOG(LogReactiveEnvActor, Log, TEXT("Dumping RTs"));
	//ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(FDumpRTs, AReactiveEnvActor*, ReactiveEnvActor, this, {


	//	});
}


#endif

FString FImpactData::toString()
{
	TArray<FStringFormatArg> formatArgs = { FStringFormatArg(impactPoint.ToString()), FStringFormatArg(size),
		FStringFormatArg(drawTexture->GetName()) };
	return FString::Format(TEXT("Impact Point {0} Impact Size {1} Texture {2}"), formatArgs);
}
