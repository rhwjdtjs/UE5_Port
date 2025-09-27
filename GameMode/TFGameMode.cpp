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
	const FName CoolDown = FName(TEXT("CoolDown")); //경기 시간이 끝나고 승자를 결정하는 상태
}
ATFGameMode::ATFGameMode()
{
	bDelayedStart = true; //게임 시작을 지연시킨다.
	bUseSeamlessTravel = true; // 꼭 활성화
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
	// 상태 전환될 때 '그 상태의 시작 시점'으로 리셋
	if (MatchState == MatchState::InProgress)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds(); // 매치 시작 시점
	}
	else if (MatchState == MatchState::CoolDown)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds(); // 쿨다운 시작 시점
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ATFPlayerController* PC = Cast<ATFPlayerController>(*It))
		{
			// 상태/시간값만 푸시 (RPC 한 번만!)
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
				RestartGame();  // 처음 웜업 끝나면 딱 1번만 리셋
			}
			else {
				StartMatch(); // 대기 시간이 끝나면 게임을 시작한다.
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
			RestartGame(); // 쿨다운 끝날 때는 항상 리셋
		}
	}
}
void ATFGameMode::PlayerEliminated(ATimeFractureCharacter* ElimmedCharacter, ATFPlayerController* VictimController, ATFPlayerController* AttackerController)
{
	ATFPlayerState* AttackerPlayerState = AttackerController ? Cast<ATFPlayerState>(AttackerController->PlayerState) : nullptr; //공격자 플레이어 상태를 초기화한다.
	ATFPlayerState* VictimPlayerState = VictimController ? Cast<ATFPlayerState>(VictimController->PlayerState) : nullptr; //피해자 플레이어 상태를 초기화한다.
	ATFGameState* TFGameState = GetGameState<ATFGameState>(); //게임 상태를 가져온다.
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && TFGameState) {
		AttackerPlayerState->AddToScore(1.f); //공격자 플레이어 상태에 점수를 추가한다.
		TFGameState->UpdateTopScorePlayers(AttackerPlayerState); //최고 점수를 가진 플레이어들을 업데이트한다.
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(); //제거된 캐릭터의 Elim 함수를 호출한다.
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1); //피해자 플레이어 상태에 처치 수를 추가한다.
	}
	if (AttackerController && VictimController)
	{
		FString KillerName = AttackerController->PlayerState->GetPlayerName();
		FString VictimName = VictimController->PlayerState->GetPlayerName();
		// 킬 한 사람한테만 WBP_Kill 보여주기
		AttackerController->ClientShowKillWidget();

		// 죽은 사람한테만 WBP_Killed 보여주기
		VictimController->ClientShowKilledWidget();

		// 모든 클라이언트에 브로드캐스트
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ATFPlayerController* KillerPC = Cast<ATFPlayerController>(*It))
				KillerPC->ClientAddKillFeedMessage(KillerName, VictimName);
		}
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

