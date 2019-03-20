// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ResolvableTexture2DFactory.generated.h"

/**
 * 
 */
UCLASS()
class UResolvableTexture2DFactory : public UFactory
{
	GENERATED_BODY()

public:

	UResolvableTexture2DFactory();

	~UResolvableTexture2DFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
	
};
