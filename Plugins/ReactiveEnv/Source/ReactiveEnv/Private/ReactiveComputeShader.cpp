#include "ReactiveComputeShader.h"

IMPLEMENT_UNIFORM_BUFFER_STRUCT(FReactiveConstantParameters, TEXT("RCSConstants"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FReactiveDynamicParameters, TEXT("RCSDynamic"))

// TODO Fill proper compute shader value here
IMPLEMENT_SHADER_TYPE(, FReactiveComputeShader, TEXT("/ReactiveEnv/Private/ReactiveComputeShader.usf"), TEXT("ReactiveComputeMainCS"), SF_Compute);


FReactiveComputeShader::FReactiveComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):FGlobalShader(Initializer)
{
	heightField2Frame.Bind(Initializer.ParameterMap,TEXT("heightField2Frame"), EShaderParameterFlags::SPF_Mandatory);
	heightField1Frame.Bind(Initializer.ParameterMap, TEXT("heightField1Frame"), EShaderParameterFlags::SPF_Mandatory);

	heightFieldOut.Bind(Initializer.ParameterMap, TEXT("heightFieldOut"), EShaderParameterFlags::SPF_Mandatory);

	offsetHeightField0Frame.Bind(Initializer.ParameterMap, TEXT("offsetHeightField0Frame"), EShaderParameterFlags::SPF_Mandatory);
	offsetHeightField1Frame.Bind(Initializer.ParameterMap, TEXT("offsetHeightField1Frame"), EShaderParameterFlags::SPF_Mandatory);

	heightFieldNormal.Bind(Initializer.ParameterMap, TEXT("heightFieldNormal"), EShaderParameterFlags::SPF_Mandatory);

	waterLandSceneCapture.Bind(Initializer.ParameterMap, TEXT("waterLandSceneCapture"), EShaderParameterFlags::SPF_Mandatory);
	snowMudSceneCapture.Bind(Initializer.ParameterMap, TEXT("snowMudSceneCapture"), EShaderParameterFlags::SPF_Mandatory);
	impactCaptures.Bind(Initializer.ParameterMap, TEXT("impactCaptures"), EShaderParameterFlags::SPF_Mandatory);

	snowResolvedTexture.Bind(Initializer.ParameterMap, TEXT("snowResolvedTexture"), EShaderParameterFlags::SPF_Mandatory);
	snowMudWriteOut.Bind(Initializer.ParameterMap, TEXT("snowMudWriteOut"),EShaderParameterFlags::SPF_Mandatory);
}

void FReactiveComputeShader::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
	OutEnvironment.SetDefine(TEXT("THREAD_GROUP_SIZE"), THREAD_GROUP_SIZE);
	OutEnvironment.SetDefine(TEXT("ENABLE_NON_UNIFORM_SIZE"), 0);
}

void FReactiveComputeShader::setSurfaces(FRHICommandList& RHICmdList, FNecessarySurfaces& surfaces)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (heightField2Frame.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, heightField2Frame.GetBaseIndex(), surfaces.heightField2Frame);

	if (heightField1Frame.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, heightField1Frame.GetBaseIndex(), surfaces.heightField1Frame);

	if (waterLandSceneCapture.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, waterLandSceneCapture.GetBaseIndex(), surfaces.waterLandSceneCapture);

	if (snowMudSceneCapture.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, snowMudSceneCapture.GetBaseIndex(), surfaces.snowMudSceneCapture);

	if (impactCaptures.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, impactCaptures.GetBaseIndex(), surfaces.impactCaptures);

	if (snowResolvedTexture.IsBound())
		RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, snowResolvedTexture.GetBaseIndex(), surfaces.snowResolvedTexture);

	if (heightFieldOut.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, heightFieldOut.GetBaseIndex(), surfaces.heightFieldOut);

	if (offsetHeightField0Frame.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, offsetHeightField0Frame.GetBaseIndex(), surfaces.offsetHeightField0Frame);

	if (offsetHeightField1Frame.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, offsetHeightField1Frame.GetBaseIndex(), surfaces.offsetHeightField1Frame);

	if (heightFieldNormal.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, heightFieldNormal.GetBaseIndex(), surfaces.heightFieldNormal);

	if (snowMudWriteOut.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, snowMudWriteOut.GetBaseIndex(), surfaces.snowMudWriteOut);
}

void FReactiveComputeShader::setUniformBuffers(FRHICommandList& RHICmdList, FReactiveDynamicParameters& VariableParameters, 
	FReactiveConstantParameters& constanctParameter)
{
	FReactiveDynamicParamUBRef dynamicParamUBRef;

	FReactiveConstantParamUBRef uniformConstantRHIRef = FReactiveConstantParamUBRef::CreateUniformBufferImmediate(constanctParameter, EUniformBufferUsage::UniformBuffer_SingleDraw);
	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FReactiveConstantParameters>(), uniformConstantRHIRef);

	dynamicParamUBRef = FReactiveDynamicParamUBRef::CreateUniformBufferImmediate(VariableParameters, EUniformBufferUsage::UniformBuffer_SingleDraw);
	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FReactiveDynamicParameters>(), dynamicParamUBRef);
}

void FReactiveComputeShader::unbindBuffers(FRHICommandList& RHICmdList)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();
	
	FShaderResourceViewRHIRef nullShaderResourceView = FShaderResourceViewRHIRef();
	FUnorderedAccessViewRHIRef nullUAV = FUnorderedAccessViewRHIRef();
	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, heightField2Frame.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, heightField1Frame.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, waterLandSceneCapture.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, snowMudSceneCapture.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, impactCaptures.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetShaderResourceViewParameter(ComputeShaderRHI, snowResolvedTexture.GetBaseIndex(), nullShaderResourceView);

	RHICmdList.SetUAVParameter(ComputeShaderRHI, heightFieldOut.GetBaseIndex(), nullUAV);

	RHICmdList.SetUAVParameter(ComputeShaderRHI, offsetHeightField0Frame.GetBaseIndex(), nullUAV);

	RHICmdList.SetUAVParameter(ComputeShaderRHI, offsetHeightField1Frame.GetBaseIndex(), nullUAV);

	RHICmdList.SetUAVParameter(ComputeShaderRHI, heightFieldNormal.GetBaseIndex(), nullUAV);

	RHICmdList.SetUAVParameter(ComputeShaderRHI, snowMudWriteOut.GetBaseIndex(), nullUAV);
}

bool FReactiveComputeShader::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return true;
}

