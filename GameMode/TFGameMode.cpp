// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/gameplayStatics.h"
#include "Gameframework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
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
