// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerState.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void ATFPlayerState::OnRep_Score()
{
	Super::OnRep_Score(); //�θ� Ŭ������ OnRep_Score �Լ��� ȣ���Ѵ�.

	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����͸� �����Ѵ�.
		if(TFPlayerController) {
			TFPlayerController->SetHUDScore(GetScore()); //�÷��̾� ��Ʈ�ѷ��� SetHUDScore �Լ��� ȣ���Ͽ� HUD�� ������ �����Ѵ�.
		}
	}
}

void ATFPlayerState::AddToScore(float ScoreAmount)
{
	if (HasAuthority())
	{
		SetScore(GetScore() + ScoreAmount);
		// ���������� OnRep_Score�� �ڵ����� ȣ����� �����Ƿ�, ���� ȣ���Ͽ� HUD�� ������Ʈ�մϴ�.
		// Ŭ���̾�Ʈ������ ������ ������ �� �ڵ����� OnRep_Score�� ȣ��˴ϴ�.
		OnRep_Score();
	}
	
}
