// Fill out your copyright notice in the Description page of Project Settings.


#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALBGameMode::PostLogin(APlayerController* NewPlayer) //플레이어 컨트롤러를 받아들인다
{
	Super::PostLogin(NewPlayer); //부모클래스의 PostLogin을 호출한다.

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num(); //게임 상태의 플레이어 배열의 개수를 가져온다.
	if (NumberOfPlayer == 2) {
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true; //서버 트래블을 사용한다.
			World->ServerTravel(FString("/Game/Maps/InGameMap?listen")); //서버 트래블을 통해 맵을 이동한다.
		}
	}
}
