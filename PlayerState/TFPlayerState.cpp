// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerState.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Net/UnrealNetwork.h" //네트워크 관련 헤더 파일을 포함시킨다.

void ATFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //부모 클래스의 GetLifetimeReplicatedProps 함수를 호출한다.

	DOREPLIFETIME(ATFPlayerState, Defeats); //Defeats 변수를 네트워크로 복제할 수 있도록 설정한다.
}
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

void ATFPlayerState::OnRep_Defeats()
{
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //플레이어 컨트롤러의 SetHUDDefeats 함수를 호출하여 HUD에 처치 수를 설정한다.
		}
	}
}

void ATFPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount; //Defeats 변수를 증가시킨다.
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //플레이어 컨트롤러의 SetHUDDefeats 함수를 호출하여 HUD에 처치 수를 설정한다.
		}
	}
}

