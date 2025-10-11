#include "TFGameMode.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
#include "UnrealProject_7A/GameState/TFGameState.h"
#include "UnrealProject_7A/System/TFGameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace MatchState {
	const FName CoolDown = FName(TEXT("CoolDown")); // 경기 종료 후 승자 결정 상태
}

/////////////////////////////////////////////////////////////
// 기능:
//   게임모드 생성자. 기본 설정을 초기화한다.
//
// 알고리즘:
//   - bDelayedStart = true : BeginPlay 후 자동으로 게임이 시작되지 않도록 설정.
//   - bUseSeamlessTravel = true : 맵 이동 시 플레이어 연결이 끊기지 않게 한다.
/////////////////////////////////////////////////////////////
ATFGameMode::ATFGameMode()
{
	bDelayedStart = true;
	bUseSeamlessTravel = true;
}

/////////////////////////////////////////////////////////////
// 기능:
//   매 프레임마다 호출되어 경기 상태를 관리한다.
//
// 알고리즘:
//   - DeltaTime은 프레임 간 시간 간격.
//   - WarmupToStartMatch()를 호출하여 시간 기반 상태 전환을 수행한다.
/////////////////////////////////////////////////////////////
void ATFGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WarmupToStartMatch();
}

/////////////////////////////////////////////////////////////
// 기능:
//   게임이 시작될 때 한 번 호출된다.
//   레벨 시작 시간을 기준으로 타이머를 초기화한다.
//
// 알고리즘:
//   - GetWorld()->GetTimeSeconds()로 현재 월드의 경과 시간을 가져와
//     LevelStartingTime에 저장한다.
/////////////////////////////////////////////////////////////
void ATFGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

/////////////////////////////////////////////////////////////
// 기능:
//   매치 상태가 변경될 때 호출된다.
//   상태에 맞게 시작 시점을 초기화하고 모든 클라이언트에 상태를 동기화한다.
//
// 알고리즘:
//   - MatchState가 InProgress 또는 CoolDown일 때마다 LevelStartingTime을 갱신한다.
//   - 모든 PlayerController를 순회하며 ClientJoinMatch() RPC를 호출해
//     클라이언트의 HUD/UI에 새로운 상태와 시간을 전달한다.
/////////////////////////////////////////////////////////////
void ATFGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	if (MatchState == MatchState::InProgress)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds();
	}
	else if (MatchState == MatchState::CoolDown)
	{
		LevelStartingTime = GetWorld()->GetTimeSeconds();
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ATFPlayerController* PC = Cast<ATFPlayerController>(*It))
		{
			PC->ClientJoinMatch(MatchState, WarmupTime, MatchTime, LevelStartingTime, CoolDownTime);
		}
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   경기의 흐름을 관리한다. (대기 → 경기 → 쿨다운)
//
// 알고리즘:
//   1. WaitingToStart (웜업 단계)
//      - CountdownTime = WarmupTime - (현재시간 - 시작시간)
//      - 시간이 0 이하가 되면 첫 번째 웜업은 RestartGame() 호출,
//        그 이후는 StartMatch() 호출.
//   2. InProgress (진행 중)
//      - CountdownTime = MatchTime - (현재시간 - 시작시간)
//      - 시간이 0 이하가 되면 CoolDown 상태로 전환.
//   3. CoolDown (종료 후 대기)
//      - CountdownTime = CoolDownTime - (현재시간 - 시작시간)
//      - 시간이 0 이하가 되면 RestartGame() 호출.
//
//   UTFGameInstance의 bFirstWarmupDone을 통해 첫 웜업만 리셋되도록 제어한다.
/////////////////////////////////////////////////////////////
void ATFGameMode::WarmupToStartMatch()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			UTFGameInstance* GI = GetGameInstance<UTFGameInstance>();
			if (GI && !GI->bFirstWarmupDone)
			{
				GI->bFirstWarmupDone = true;
				RestartGame(); // 첫 웜업만 리셋
			}
			else
			{
				StartMatch(); // 실제 경기 시작
			}
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = MatchTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}
	else if (MatchState == MatchState::CoolDown)
	{
		CountdownTime = CoolDownTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			RestartGame(); // 경기 종료 후 항상 리셋
		}
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   플레이어가 제거되었을 때(사망 시) 호출된다.
//   점수 처리, HUD 알림, 킬 피드 표시 등을 수행한다.
//
// 알고리즘:
//   - 공격자와 희생자의 PlayerState를 가져온다.
//   - 공격자는 점수 +1, 희생자는 사망 수 +1.
//   - TFGameState::UpdateTopScorePlayers()로 최고 점수자 갱신.
//   - 캐릭터의 Elim() 함수 호출로 사망 애니메이션 등 실행.
//   - 각 클라이언트에 RPC 호출:
//       • 공격자 : ClientShowKillWidget()
//       • 희생자 : ClientShowKilledWidget()
//       • 전체 : ClientAddKillFeedMessage() 로 킬로그 표시
/////////////////////////////////////////////////////////////
void ATFGameMode::PlayerEliminated(ATimeFractureCharacter* ElimmedCharacter, ATFPlayerController* VictimController, ATFPlayerController* AttackerController)
{
	ATFPlayerState* AttackerPlayerState = AttackerController ? Cast<ATFPlayerState>(AttackerController->PlayerState) : nullptr;
	ATFPlayerState* VictimPlayerState = VictimController ? Cast<ATFPlayerState>(VictimController->PlayerState) : nullptr;
	ATFGameState* TFGameState = GetGameState<ATFGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && TFGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		TFGameState->UpdateTopScorePlayers(AttackerPlayerState);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (AttackerController && VictimController)
	{
		FString KillerName = AttackerController->PlayerState->GetPlayerName();
		FString VictimName = VictimController->PlayerState->GetPlayerName();

		AttackerController->ClientShowKillWidget();
		VictimController->ClientShowKilledWidget();

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ATFPlayerController* KillerPC = Cast<ATFPlayerController>(*It))
				KillerPC->ClientAddKillFeedMessage(KillerName, VictimName);
		}
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   캐릭터가 사망 후 다시 리스폰되도록 처리한다.
//
// 알고리즘:
//   - ElimmedCharacter:
//       • Reset() : 체력, 상태 등 초기화.
//       • Destroy() : 기존 액터 제거.
//   - ElimmedController:
//       • 모든 APlayerStart 액터를 가져와 랜덤 위치 선택.
//       • RestartPlayerAtPlayerStart() 호출로 새 캐릭터 생성.
/////////////////////////////////////////////////////////////
void ATFGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
