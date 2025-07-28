// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/gameplayStatics.h"
#include "Gameframework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
ATFGameMode::ATFGameMode()
{
	bDelayedStart = true; //���� ������ ������Ų��.
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
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) { //���� ������ ��� �÷��̾� ��Ʈ�ѷ��� �ݺ��Ѵ�.
		ATFPlayerController* TFPlayer = Cast<ATFPlayerController>(*It); //���� �÷��̾� ��Ʈ�ѷ��� ATFPlayerController�� ĳ�����Ѵ�.
		if (TFPlayer) {
			TFPlayer->OnMatchStateSet(MatchState); //�÷��̾� ��Ʈ�ѷ��� OnMatchStateSet �Լ��� ȣ���Ѵ�.
		}
	}
}
void ATFGameMode::WarmupToStartMatch()
{
	if (MatchState == MatchState::WaitingToStart) {
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime; //���� ���� �� ��� �ð��� �����Ѵ�.
		if (CountdownTime <= 0.f) {
			StartMatch(); //��� �ð��� ������ ������ �����Ѵ�.
		}
	}
}
void ATFGameMode::PlayerEliminated(ATimeFractureCharacter* ElimmedCharacter, ATFPlayerController* VictimController, ATFPlayerController* AttackerController)
{
	ATFPlayerState* AttackerPlayerState = AttackerController ? Cast<ATFPlayerState>(AttackerController->PlayerState) : nullptr; //������ �÷��̾� ���¸� �ʱ�ȭ�Ѵ�.
	ATFPlayerState* VictimPlayerState = VictimController ? Cast<ATFPlayerState>(VictimController->PlayerState) : nullptr; //������ �÷��̾� ���¸� �ʱ�ȭ�Ѵ�.

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState) {
		AttackerPlayerState->AddToScore(1.f); //������ �÷��̾� ���¿� ������ �߰��Ѵ�.
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(); //���ŵ� ĳ������ Elim �Լ��� ȣ���Ѵ�.
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1); //������ �÷��̾� ���¿� óġ ���� �߰��Ѵ�.
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

