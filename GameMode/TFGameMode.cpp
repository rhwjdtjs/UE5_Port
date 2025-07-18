// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/gameplayStatics.h"
#include "Gameframework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
void ATFGameMode::PlayerEliminated(ATimeFractureCharacter* ElimmedCharacter, ATFPlayerController* VictimController, ATFPlayerController* AttackerController)
{
	ATFPlayerState* AttackerPlayerState = AttackerController ? Cast<ATFPlayerState>(AttackerController->PlayerState) : nullptr; //공격자 플레이어 상태를 초기화한다.
	ATFPlayerState* VictimPlayerState = VictimController ? Cast<ATFPlayerState>(VictimController->PlayerState) : nullptr; //피해자 플레이어 상태를 초기화한다.

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState) {
		AttackerPlayerState->AddToScore(1.f); //공격자 플레이어 상태에 점수를 추가한다.
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(); //제거된 캐릭터의 Elim 함수를 호출한다.
	}
}

void ATFGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) { // 제거된 캐릭터가 유효한지 확인합니다.
        // 제거된 캐릭터의 상태를 초기화합니다. (예: 체력, 위치, 점수 등 게임 내에서 캐릭터가 가졌던 모든 상태를 기본값으로 되돌립니다)
        ElimmedCharacter->Reset();
        // 제거된 캐릭터의 액터를 월드에서 완전히 파괴합니다. (메모리에서 제거되어 더 이상 게임에 존재하지 않게 됩니다)
        ElimmedCharacter->Destroy();
	}
	if (ElimmedController) {
		TArray<AActor*> PlayerStarts; //모든 플레이어 시작 지점을 저장할 배열을 생성합니다.
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts); //모든 플레이어 시작 지점을 가져옵니다.
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); //랜덤으로 플레이어 시작 지점을 선택합니다.
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
