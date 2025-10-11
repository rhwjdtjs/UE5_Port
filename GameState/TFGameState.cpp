// Fill out your copyright notice in the Description page of Project Settings.

#include "TFGameState.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"

/////////////////////////////////////////////////////////////
// ���:
//   ��Ʈ��ũ���� �����Ǿ�� �� ������ ����Ѵ�.
//
// �˰���:
//   - Unreal Engine�� DOREPLIFETIME ��ũ�θ� ����Ͽ�
//     TopScorePlayers �迭�� ���� �� Ŭ���̾�Ʈ�� �ڵ� ����ȭ�Ѵ�.
//   - �� �迭�� ��Ƽ�÷��̾� ȯ�濡�� ��� Ŭ���̾�Ʈ�� ������
//     �ְ� ������ ������ �����ϵ��� �ϱ� �����̴�.
/////////////////////////////////////////////////////////////
void ATFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFGameState, TopScorePlayers);
}

/////////////////////////////////////////////////////////////
// ���:
//   �÷��̾��� ������ ����� �� ȣ��Ǿ�, �ְ� ������ �����Ѵ�.
//
// �˰���:
//   - ù ���:
//       TopScorePlayers �迭�� ������� ���, ���� �÷��̾ ù �ְ� �����ڷ� ���.
//       TopScore�� �ش� �÷��̾��� ������ ����.
//   - ���� ó��:
//       ���� �÷��̾��� ������ ���� TopScore�� ���ٸ�, �ߺ����� �ʰ� �迭�� �߰�.
//       (AddUnique�� �ߺ� ����)
//   - �ְ��� ����:
//       �÷��̾� ������ ���� TopScore���� ���ٸ�,
//       �迭�� ���� ���ο� �ְ� �����ڸ� ���.
//       TopScore�� �� ������ ����.
//
//   �� ������ ��ŷ 1��(�Ǵ� ���� 1��) ����� �����ϱ� ���� �ܼ� �� �˰����̴�.
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
