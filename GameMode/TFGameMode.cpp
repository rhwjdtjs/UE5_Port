// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
#include "UnrealProject_7A/GameState/TFGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UnrealProject_7A/System/TFGameInstance.h"
namespace MatchState {
	const FName CoolDown = FName(TEXT("CoolDown")); //��� �ð��� ������ ���ڸ� �����ϴ� ����
}
ATFGameMode::ATFGameMode()
{
	bDelayedStart = true; //���� ������ ������Ų��.
	bUseSeamlessTravel = true; // �� Ȱ��ȭ
}

void ATFGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); //�θ� Ŭ������ Tick �Լ��� ȣ���Ѵ�.
	WarmupToStartMatch();
}

void ATFGameMode::BeginPlay()
{
	Super::BeginPlay(); //�θ� Ŭ������ BeginPlay �Լ��� ȣ���Ѵ�.
	LevelStartingTime = GetWorld()->GetTimeSeconds(); //���� ���� �ð��� ���� ���� �ð����� �����Ѵ�.
}
void ATFGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet(); //�θ� Ŭ������ OnMatchStateSet �Լ��� ȣ���Ѵ�.
	// ���� ��ȯ�� �� '�� ������ ���� ����'���� ����
	if (MatchState == MatchState::InProgress)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds(); // ��ġ ���� ����
	}
	else if (MatchState == MatchState::CoolDown)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds(); // ��ٿ� ���� ����
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ATFPlayerController* PC = Cast<ATFPlayerController>(*It))
		{
			// ����/�ð����� Ǫ�� (RPC �� ����!)
			PC->ClientJoinMatch(MatchState, WarmupTime, MatchTime, LevelStartingTime, CoolDownTime);

			
		}
	}
}
void ATFGameMode::WarmupToStartMatch()
{
	if (MatchState == MatchState::WaitingToStart) {
		//CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		CountdownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f) {
			UTFGameInstance* GI = GetGameInstance<UTFGameInstance>();
			if (GI && !GI->bFirstWarmupDone) {
				GI->bFirstWarmupDone = true;
				RestartGame();  // ó�� ���� ������ �� 1���� ����
			}
			else {
				StartMatch(); // ��� �ð��� ������ ������ �����Ѵ�.
			}
		}
	}
	else if (MatchState == MatchState::InProgress) {
		//CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		CountdownTime = MatchTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f) {
			SetMatchState(MatchState::CoolDown);
			//LevelStartingTime = GetWorld()->GetTimeSeconds();
		}
	}
	else if (MatchState == MatchState::CoolDown) {
		//CountdownTime = CoolDownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		CountdownTime = CoolDownTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f) {
			RestartGame(); // ��ٿ� ���� ���� �׻� ����
		}
	}
}
void ATFGameMode::PlayerEliminated(ATimeFractureCharacter* ElimmedCharacter, ATFPlayerController* VictimController, ATFPlayerController* AttackerController)
{
	ATFPlayerState* AttackerPlayerState = AttackerController ? Cast<ATFPlayerState>(AttackerController->PlayerState) : nullptr; //������ �÷��̾� ���¸� �ʱ�ȭ�Ѵ�.
	ATFPlayerState* VictimPlayerState = VictimController ? Cast<ATFPlayerState>(VictimController->PlayerState) : nullptr; //������ �÷��̾� ���¸� �ʱ�ȭ�Ѵ�.
	ATFGameState* TFGameState = GetGameState<ATFGameState>(); //���� ���¸� �����´�.
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && TFGameState) {
		AttackerPlayerState->AddToScore(1.f); //������ �÷��̾� ���¿� ������ �߰��Ѵ�.
		TFGameState->UpdateTopScorePlayers(AttackerPlayerState); //�ְ� ������ ���� �÷��̾���� ������Ʈ�Ѵ�.
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(); //���ŵ� ĳ������ Elim �Լ��� ȣ���Ѵ�.
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1); //������ �÷��̾� ���¿� óġ ���� �߰��Ѵ�.
	}
	if (AttackerController && VictimController)
	{
		FString KillerName = AttackerController->PlayerState->GetPlayerName();
		FString VictimName = VictimController->PlayerState->GetPlayerName();
		// ų �� ������׸� WBP_Kill �����ֱ�
		AttackerController->ClientShowKillWidget();

		// ���� ������׸� WBP_Killed �����ֱ�
		VictimController->ClientShowKilledWidget();

		// ��� Ŭ���̾�Ʈ�� ��ε�ĳ��Ʈ
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ATFPlayerController* KillerPC = Cast<ATFPlayerController>(*It))
				KillerPC->ClientAddKillFeedMessage(KillerName, VictimName);
		}
	}
}

void ATFGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) { // ���ŵ� ĳ���Ͱ� ��ȿ���� Ȯ���մϴ�.
        // ���ŵ� ĳ������ ���¸� �ʱ�ȭ�մϴ�. (��: ü��, ��ġ, ���� �� ���� ������ ĳ���Ͱ� ������ ��� ���¸� �⺻������ �ǵ����ϴ�)
        ElimmedCharacter->Reset();
        // ���ŵ� ĳ������ ���͸� ���忡�� ������ �ı��մϴ�. (�޸𸮿��� ���ŵǾ� �� �̻� ���ӿ� �������� �ʰ� �˴ϴ�)
        ElimmedCharacter->Destroy();
	}
	if (ElimmedController) {
		TArray<AActor*> PlayerStarts; //��� �÷��̾� ���� ������ ������ �迭�� �����մϴ�.
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts); //��� �÷��̾� ���� ������ �����ɴϴ�.
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); //�������� �÷��̾� ���� ������ �����մϴ�.
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

