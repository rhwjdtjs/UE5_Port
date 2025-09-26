// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddToScore(float ScoreAmount);
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToDefeats(int32 DefeatAmount);
	virtual void OnRep_PlayerName() override;
	FORCEINLINE int32 GetDefeats() const { return Defeats; }
private:
	UPROPERTY()
	class ATimeFractureCharacter* TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	UPROPERTY()
	class ATFPlayerController* TFPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����͸� �����Ѵ�.
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
};
