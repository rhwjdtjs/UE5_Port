// Fill out your copyright notice in the Description page of Project Settings.


#include "TFGameState.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h" // TFPlayerState ��������� ���Խ�Ų��.
void ATFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFGameState, TopScorePlayers); // TopScorePlayers ������ �����Ѵ�.
}

void ATFGameState::UpdateTopScorePlayers(ATFPlayerState* ScoringPlayer)
{
	if (TopScorePlayers.Num() == 0) {
		TopScorePlayers.Add(ScoringPlayer); // TopScorePlayers �迭�� ��������� ScoringPlayer�� �߰��Ѵ�.
		TopScore = ScoringPlayer->GetScore(); // TopScore�� ScoringPlayer�� ������ �����Ѵ�.
	}
	else if (ScoringPlayer->GetScore()==TopScore) {
		TopScorePlayers.AddUnique(ScoringPlayer); // ScoringPlayer�� ������ TopScore�� ������ TopScorePlayers �迭�� �߰��Ѵ�.
	}
	else if (ScoringPlayer->GetScore() > TopScore) {
		TopScorePlayers.Empty(); // TopScorePlayers �迭�� ����.top
		TopScorePlayers.Add(ScoringPlayer); // ScoringPlayer�� TopScorePlayers �迭�� �߰��Ѵ�.
		TopScore = ScoringPlayer->GetScore(); // TopScore�� ScoringPlayer�� ������ �����Ѵ�.
	}
}
