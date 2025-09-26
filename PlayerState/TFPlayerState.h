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
	class ATimeFractureCharacter* TFCharacter; //캐릭터의 포인터를 저장한다.
	UPROPERTY()
	class ATFPlayerController* TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
};
