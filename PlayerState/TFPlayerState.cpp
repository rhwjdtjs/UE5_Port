// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerState.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "UnrealProject_7A/HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h" //��Ʈ��ũ ���� ��� ������ ���Խ�Ų��.

void ATFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //�θ� Ŭ������ GetLifetimeReplicatedProps �Լ��� ȣ���Ѵ�.

	DOREPLIFETIME(ATFPlayerState, Defeats); //Defeats ������ ��Ʈ��ũ�� ������ �� �ֵ��� �����Ѵ�.
}
void ATFPlayerState::OnRep_Score()
{
	Super::OnRep_Score(); //�θ� Ŭ������ OnRep_Score �Լ��� ȣ���Ѵ�.

	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����͸� �����Ѵ�.
		if(TFPlayerController) {
			TFPlayerController->SetHUDScore(GetScore()); //�÷��̾� ��Ʈ�ѷ��� SetHUDScore �Լ��� ȣ���Ͽ� HUD�� ������ �����Ѵ�.
			TFPlayerController->UpdateScoreboard();
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

void ATFPlayerState::OnRep_Defeats()
{
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����͸� �����Ѵ�.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //�÷��̾� ��Ʈ�ѷ��� SetHUDDefeats �Լ��� ȣ���Ͽ� HUD�� óġ ���� �����Ѵ�.
			TFPlayerController->UpdateScoreboard();
		}
	}
}

void ATFPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount; //Defeats ������ ������Ų��.
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //ĳ������ �����͸� �����Ѵ�.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����͸� �����Ѵ�.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //�÷��̾� ��Ʈ�ѷ��� SetHUDDefeats �Լ��� ȣ���Ͽ� HUD�� óġ ���� �����Ѵ�.
		}
	}
}

void ATFPlayerState::OnRep_PlayerName()
{
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn) return;
	TFCharacter = Cast<ATimeFractureCharacter>(OwnerPawn);
	if (TFCharacter)
		if (UOverheadWidget* W = TFCharacter->GetOverheadWidget())
			W->ShowPlayerNetRole(OwnerPawn);

	if (ATFPlayerController* PC = Cast<ATFPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->UpdateScoreboard(); // �̸� ���� �Ϸ�Ǹ� ���ھ�� �ٽ� ����
	}
}




