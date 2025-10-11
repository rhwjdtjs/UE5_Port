// Fill out your copyright notice in the Description page of Project Settings.

#include "TFGameState.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"

/////////////////////////////////////////////////////////////
// 기능:
//   네트워크에서 복제되어야 할 변수를 등록한다.
//
// 알고리즘:
//   - Unreal Engine의 DOREPLIFETIME 매크로를 사용하여
//     TopScorePlayers 배열을 서버 → 클라이언트로 자동 동기화한다.
//   - 이 배열은 멀티플레이어 환경에서 모든 클라이언트가 동일한
//     최고 점수자 정보를 공유하도록 하기 위함이다.
/////////////////////////////////////////////////////////////
void ATFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFGameState, TopScorePlayers);
}

/////////////////////////////////////////////////////////////
// 기능:
//   플레이어의 점수가 변경될 때 호출되어, 최고 점수를 갱신한다.
//
// 알고리즘:
//   - 첫 등록:
//       TopScorePlayers 배열이 비어있을 경우, 현재 플레이어를 첫 최고 점수자로 등록.
//       TopScore를 해당 플레이어의 점수로 설정.
//   - 동점 처리:
//       현재 플레이어의 점수가 기존 TopScore와 같다면, 중복되지 않게 배열에 추가.
//       (AddUnique로 중복 방지)
//   - 최고점 갱신:
//       플레이어 점수가 기존 TopScore보다 높다면,
//       배열을 비우고 새로운 최고 점수자를 등록.
//       TopScore를 새 점수로 갱신.
//
//   이 로직은 랭킹 1위(또는 공동 1위) 목록을 관리하기 위한 단순 비교 알고리즘이다.
/////////////////////////////////////////////////////////////
void ATFGameState::UpdateTopScorePlayers(ATFPlayerState* ScoringPlayer)
{
	if (TopScorePlayers.Num() == 0)
	{
		TopScorePlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScorePlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScorePlayers.Empty();
		TopScorePlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}
