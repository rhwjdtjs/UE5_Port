// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/gameplayStatics.h"
#include "Gameframework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
ATFGameMode::ATFGameMode()
{
	bDelayedStart = true; //게임 시작을 지연시킨다.
}

void ATFGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); //부모 클래스의 Tick 함수를 호출한다.
	WarmupToStartMatch();
}

void ATFGameMode::BeginPlay()
{
	Super::BeginPlay(); //부모 클래스의 BeginPlay 함수를 호출한다.
	LevelStartingTime = GetWorld()->GetTimeSeconds(); //레벨 시작 시간을 현재 월드 시간으로 설정한다.
}
void ATFGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet(); //부모 클래스의 OnMatchStateSet 함수를 호출한다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) { //현재 월드의 모든 플레이어 컨트롤러를 반복한다.
		ATFPlayerController* TFPlayer = Cast<ATFPlayerController>(*It); //현재 플레이어 컨트롤러를 ATFPlayerController로 캐스팅한다.
		if (TFPlayer) {
			TFPlayer->OnMatchStateSet(MatchState); //플레이어 컨트롤러의 OnMatchStateSet 함수를 호출한다.
		}
	}
}
void ATFGameMode::WarmupToStartMatch()
{
	if (MatchState == MatchState::WaitingToStart) {
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime; //게임 시작 전 대기 시간을 설정한다.
		if (CountdownTime <= 0.f) {
			StartMatch(); //대기 시간이 끝나면 게임을 시작한다.
		}
	}
}
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
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1); //피해자 플레이어 상태에 처치 수를 추가한다.
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

