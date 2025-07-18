// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerState.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void ATFPlayerState::OnRep_Score()
{
	Super::OnRep_Score(); //부모 클래스의 OnRep_Score 함수를 호출한다.

	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if(TFPlayerController) {
			TFPlayerController->SetHUDScore(GetScore()); //플레이어 컨트롤러의 SetHUDScore 함수를 호출하여 HUD에 점수를 설정한다.
		}
	}
}

void ATFPlayerState::AddToScore(float ScoreAmount)
{
	if (HasAuthority())
	{
		SetScore(GetScore() + ScoreAmount);
		// 서버에서는 OnRep_Score가 자동으로 호출되지 않으므로, 직접 호출하여 HUD를 업데이트합니다.
		// 클라이언트에서는 점수가 복제된 후 자동으로 OnRep_Score가 호출됩니다.
		OnRep_Score();
	}
	
}
