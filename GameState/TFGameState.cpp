// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameState.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h" // TFPlayerState 헤더파일을 포함시킨다.
void ATFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFGameState, TopScorePlayers); // TopScorePlayers 변수를 복제한다.
}

void ATFGameState::UpdateTopScorePlayers(ATFPlayerState* ScoringPlayer)
{
	if (TopScorePlayers.Num() == 0) {
		TopScorePlayers.Add(ScoringPlayer); // TopScorePlayers 배열이 비어있으면 ScoringPlayer를 추가한다.
		TopScore = ScoringPlayer->GetScore(); // TopScore를 ScoringPlayer의 점수로 설정한다.
	}
	else if (ScoringPlayer->GetScore()==TopScore) {
		TopScorePlayers.AddUnique(ScoringPlayer); // ScoringPlayer의 점수가 TopScore와 같으면 TopScorePlayers 배열에 추가한다.
	}
	else if (ScoringPlayer->GetScore() > TopScore) {
		TopScorePlayers.Empty(); // TopScorePlayers 배열을 비운다.top
		TopScorePlayers.Add(ScoringPlayer); // ScoringPlayer를 TopScorePlayers 배열에 추가한다.
		TopScore = ScoringPlayer->GetScore(); // TopScore를 ScoringPlayer의 점수로 설정한다.
	}
}
