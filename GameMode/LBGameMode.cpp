// Fill out your copyright notice in the Description page of Project Settings.


#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALBGameMode::PostLogin(APlayerController* NewPlayer) //�÷��̾� ��Ʈ�ѷ��� �޾Ƶ��δ�
{
	Super::PostLogin(NewPlayer); //�θ�Ŭ������ PostLogin�� ȣ���Ѵ�.

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num(); //���� ������ �÷��̾� �迭�� ������ �����´�.
	if (NumberOfPlayer == 2) {
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true; //���� Ʈ������ ����Ѵ�.
			World->ServerTravel(FString("/Game/Maps/InGameMap?listen")); //���� Ʈ������ ���� ���� �̵��Ѵ�.
		}
	}
}
