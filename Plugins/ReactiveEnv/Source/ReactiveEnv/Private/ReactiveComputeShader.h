#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "ShaderParameterMacros.h"
#include "RHICommandList.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FReactiveConstantParameters, )
SHADER_PARAMETER(int, resolution)
SHADER_PARAMETER(float, heightFieldTexelSize)
SHADER_PARAMETER(float, snowMudTexelSize)
SHADER_PARAMETER(float, reactiveSize)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FReactiveDynamicParameters, )
SHADER_PARAMETER(float, travelSpeed)
SHADER_PARAMETER(float, damping)
SHADER_PARAMETER(float, impactStrength)
SHADER_PARAMETER(float, waveStrength)
SHADER_PARAMETER(float, heightFieldScale)
SHADER_PARAMETER(FVector, distanceOffset)
SHADER_PARAMETER(int, heightField2FrameRes)
SHADER_PARAMETER(int, heightField1FrameRes)
SHADER_PARAMETER(int, heightFieldOutRes)
SHADER_PARAMETER(int, offsetHeightField0FrameRes)
SHADER_PARAMETER(int, offsetHeightField1FrameRes)
SHADER_PARAMETER(int, heightFieldNormalRes)
SHADER_PARAMETER(int, waterLandSceneCaptureRes)
SHADER_PARAMETER(int, snowMudSceneCaptureRes)
SHADER_PARAMETER(int, impactCapturesRes)
SHADER_PARAMETER(int, snowResolvedTextureRes)
SHADER_PARAMETER(int, snowMudWriteOutRes)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FReactiveConstantParameters> FReactiveConstantParamUBRef;
typedef TUniformBufferRef<FReactiveDynamicParameters> FReactiveDynamicParamUBRef;

#define THREAD_GROUP_SIZE 32

struct FNecessarySurfaces
{
public:
	FShaderResourceViewRHIRef/*Texture2D<float>*/ heightField2Frame;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ heightField1Frame;

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ heightFieldOut;//OP

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ offsetHeightField0Frame;
	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ offsetHeightField1Frame;

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ heightFieldNormal;//OP From HeightFieldOut

	FShaderResourceViewRHIRef/*Texture2D<float>*/ waterLandSceneCapture;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ snowMudSceneCapture;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ impactCaptures;

	FShaderResourceViewRHIRef/*Texture2D<float>*/ snowResolvedTexture;
	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ snowMudWriteOut;// OP
};


class FReactiveComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FReactiveComputeShader, Global);

public:

	FReactiveComputeShader() {}

	explicit FReactiveComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << heightField2Frame;
		Ar << heightField1Frame;

		Ar << heightFieldOut;//OP

		Ar << offsetHeightField0Frame;
		Ar << offsetHeightField1Frame;

		Ar << heightFieldNormal;//OP From HeightFieldOut

		Ar << waterLandSceneCapture;
		Ar << snowMudSceneCapture;
		Ar << impactCaptures;

		Ar << snowResolvedTexture;
		Ar << snowMudWriteOut;// OP

		return bShaderHasOutdatedParams;
	}


	void setSurfaces(FRHICommandList& RHICmdList, FNecessarySurfaces& surfaces);
	void setUniformBuffers(FRHICommandList& RHICmdList,FReactiveDynamicParameters& VariableParameters,
		FReactiveConstantParameters& constanctParameter);
	void unbindBuffers(FRHICommandList& RHICmdList);
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);

private:

	FShaderResourceParameter/*Texture2D<float>*/ heightField2Frame;
	FShaderResourceParameter/*Texture2D<float>*/ heightField1Frame;

	FShaderResourceParameter/*RWTexture2D<float>*/ heightFieldOut;//OP

	FShaderResourceParameter/*RWTexture2D<float>*/ offsetHeightField0Frame;
	FShaderResourceParameter/*RWTexture2D<float>*/ offsetHeightField1Frame;

	FShaderResourceParameter/*RWTexture2D<float>*/ heightFieldNormal;//OP From HeightFieldOut

	FShaderResourceParameter/*Texture2D<float>*/ waterLandSceneCapture;
	FShaderResourceParameter/*Texture2D<float>*/ snowMudSceneCapture;
	FShaderResourceParameter/*Texture2D<float>*/ impactCaptures;

	FShaderResourceParameter/*Texture2D<float>*/ snowResolvedTexture;
	FShaderResourceParameter/*RWTexture2D<float>*/ snowMudWriteOut;// OP
};