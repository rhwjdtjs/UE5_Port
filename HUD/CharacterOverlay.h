// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatsAmount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoAmount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CarriedAmmoAmount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchCountDownText;
};
