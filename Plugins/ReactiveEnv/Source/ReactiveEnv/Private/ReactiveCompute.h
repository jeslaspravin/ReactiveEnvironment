#pragma once

#include "CoreMinimal.h"
#include "ReactiveComputeShader.h"

class AReactiveEnvActor;

struct FHeightFieldResources
{
public:
	FHeightFieldResources() {}

	FHeightFieldResources(FShaderResourceViewRHIRef srv, FUnorderedAccessViewRHIRef uav)
	{
		SRV = srv;
		UAV = uav;
	}

	FShaderResourceViewRHIRef SRV;

	FUnorderedAccessViewRHIRef UAV;

	void release()
	{
		SRV->Release();

		UAV->Release();
	}
};

class FReactiveCompute 
{
private:

	bool bIsUnloading = false;
	bool bIsComputing = false;
	bool bSnapshot = false;
	uint32 executionCounter;
	uint8 heightFieldTextureIndex;
	uint8 heightShiftIndex;

	ERHIFeatureLevel::Type featureLevel;

	TWeakObjectPtr<AReactiveEnvActor> ownerReactiveActor;
	// Pre created Resources

	FTexture2DRHIRef snowMudSceneCaptureCurrent;
	FShaderResourceViewRHIRef snowMudSceneCaptureSRV;
	FTexture2DRHIRef waterLandSceneCaptureCurrent;
	FShaderResourceViewRHIRef waterLandSceneCaptureSRV;
	FTexture2DRHIRef snowResolvedTextureCurrent;
	FShaderResourceViewRHIRef snowResolvedTextureSRV;
	FTexture2DRHIRef impactsCapturedCurrent;
	FShaderResourceViewRHIRef impactsCapturedSRV;

	// End Pre created Resources

	FTexture2DRHIRef heightFieldTextures[2][3];
	FHeightFieldResources heightFieldResources[2][3];

	FTexture2DRHIRef heightFieldNormalTexture;
	FUnorderedAccessViewRHIRef heightFieldNormalUAV;

	FTexture2DRHIRef snowWriteOutTexture;
	FUnorderedAccessViewRHIRef snowWriteOutUAV;

	FReactiveDynamicParameters dynamicParameters;
	FReactiveConstantParameters constantParams;

private:

	void computeInternal();

	void safeReleaseResources();

	void snapshot(FRHICommandListImmediate& RHICmdList);

	void updateDynamicParameters(FRHICommandListImmediate& RHICmdList, TShaderMapRef<FReactiveComputeShader>& shader);

	void setTextures(FRHICommandListImmediate& RHICmdList, TShaderMapRef<FReactiveComputeShader>& shader);

public:

	FReactiveCompute(AReactiveEnvActor* ownerActor, ERHIFeatureLevel::Type shaderFeatureLevel);

	~FReactiveCompute();

	void saveSnapshotFrame();

	void computeReactive();


};