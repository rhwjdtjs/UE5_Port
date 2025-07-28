// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Alert.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UAlert : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AlertText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InfoText;
};
