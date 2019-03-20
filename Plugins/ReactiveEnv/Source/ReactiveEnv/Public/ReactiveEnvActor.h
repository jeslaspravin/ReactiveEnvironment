// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Texture/ResolvableTexture2DDynamic.h"
#include "ReactiveEnvActor.generated.h"

class FReactiveCompute;

DECLARE_LOG_CATEGORY_EXTERN(LogReactiveEnvActor,Log,All)

const FName REACTIVE = FName("reactive");// In case of actor or component that needs to be considered reactive
const FName REACTIVE_IDLE = FName("reactiveIdle");// In case of actor that when idle will not consider its components
const FName REACTIVE_LOCAL = FName("reactiveLocal");// In case of component that needs to be considered reactive even when idle
const FName REACTIVE_SNOW_MUD = FName("reactiveLand");// For components/actor that is reactive surface of either mud or snow or similar components
const FName REACTIVE_WATER = FName("reactiveWater");// For components/actor that is reactive surface of either water or water plants or similar components


const FName MPC_REACTIVE_ORIGIN = FName("ReactiveOrigin");// Capture origin only x,y matters
const FName MPC_REACTIVE_DISTANCE = FName("ReactiveDistance");// Distance up to which from center of player to enable reactive environment
const FName MPC_REACTIVE_WATER_NOISE_RED_FACT = FName("NoiseReductionFactor");// Noise reduction factor

USTRUCT(BlueprintType)
struct REACTIVEENV_API FCapturingActorData
{
	GENERATED_BODY()

public:

	FCapturingActorData():bSkipOnIdleActor(false){}

	TArray<TWeakObjectPtr<UPrimitiveComponent>> captureComponents;

	bool bSkipOnIdleActor;
};


USTRUCT(BlueprintType)
struct REACTIVEENV_API FImpactData
{
	GENERATED_BODY()

public:
	FImpactData() {}

	FImpactData(FVector pt, float radius) :impactPoint(pt),size(radius) {}

	UPROPERTY(BlueprintReadWrite)
		FVector impactPoint=FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
		float size=0;

	UPROPERTY(BlueprintReadWrite)
		UTexture* drawTexture=nullptr;

	inline FString toString();
};

UCLASS()
class REACTIVEENV_API AReactiveEnvActor : public AActor
{
	GENERATED_BODY()

private:


	FDelegateHandle actorSpawnedHandler;

	static TWeakObjectPtr<AReactiveEnvActor> instance;

	UPROPERTY(Transient)
		TArray<FImpactData> impactsToDraw;

	FVector displacementOffset;

	FReactiveCompute* reactiveCompute=nullptr;

	// From Marvel Master Infinite Reactive Water Plugin
	float noiseReducingFactor;

	float travelSpeed;
	float speedPerFrame;

protected:

		TMap<TWeakObjectPtr<AActor>, FCapturingActorData> actorCapturingMap;

		// Components that will get reacted like snow mud
		TArray<TWeakObjectPtr<UPrimitiveComponent>> snowMudCaptureComponents;

		// Components that will get reacted like water and water plants
		TArray<TWeakObjectPtr<UPrimitiveComponent>> waterCaptureComponents;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float waveDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float waveTravelSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float travelSpeedMultiplier=1;

	// For Impact waves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float impactStrengthMultiplier = 1;
	// For non impact generated waves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float waveStrengthMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
		float heightFieldNormalScale = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capturing")
		float minSpeedForIdling=0.1f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
		int32 reactiveDistance=8192;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
		uint8 updateActorPerTask=5;
	/*
	 *	Resolution at which to create water reactive texture,This is the resolution at which height field UAV,Normal textures will be created with.
	 */ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
		int32 reactiveRTResolution = 2048;

	// Capture data starts

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SceneCapture")
		USceneCaptureComponent2D* landWaterSceneCaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SceneCapture")
		USceneCaptureComponent2D* snowMudSceneCaptureComponent;

	// Land impact texture for mud and snow(Capture only)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UTextureRenderTarget2D* snowMudCaptureRT;

	// Texture target that has info of object above water and land, will be useful for grass simulation(Not grass in water)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UTextureRenderTarget2D* landWaterCaptureRT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SceneCapture")
		float topSceneCaptureOffset=2000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SceneCapture")
		float bottomSceneCaptureOffset = 2000;

	// Capture data ends

	// Resolved Textures starts

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UTextureRenderTarget2D* impactsCaptureRT;

	/*
	 *	Water height field generated texture
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UResolvableTexture2DDynamic* waterHeightFieldTexture;

	/*
	*	Water height Field's generated texture.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UResolvableTexture2DDynamic* waterHeightFieldNormalTexture;

	/*
	* Snow and Mud texture that will be usable for creating snow effect
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Textures")
		UResolvableTexture2DDynamic* snowMudTexture;

	// Resolved Textures ends

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
		UMaterialParameterCollection* reactiveEnvMPC;

private:

	/*
	* Draws all the impacts accumulated in this frame.
	*/
	void drawAllImpacts();

	void clearResolvedTargets();
	
public:	
	// Sets default values for this actor's properties
	AReactiveEnvActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void BeginDestroy() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void updateAllCapturableActors();

	void onActorSpawned(AActor* actor);

	void captureActorData(AActor* actor, bool updateToSceneCapture = true);

	UFUNCTION()
	void onActorDestroyed(AActor* actor);

	void createMaterialRTs();

	void destroyMaterialRTs();

	UTextureRenderTarget2D* getReactiveWaterRT();
	UTextureRenderTarget2D* getReactiveSnowMudRT();
	FVector getDisplacementOffset() { return displacementOffset; }
	// Use this over variable
	UFUNCTION(BlueprintCallable,Category="Wave Simulation")
	float getWaveTravelSpeed();

	void updateToSceneCapture(FCapturingActorData &captureData);

	static AReactiveEnvActor* getInstance()
	{
		return instance.IsValid() ? instance.Get(false) : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Impact")
		static void addManualImpact(FImpactData impactData);

#if WITH_EDITOR
	void dumpRenderTargets();
#endif

};
