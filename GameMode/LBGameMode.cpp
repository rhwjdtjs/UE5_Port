// Fill out your copyright notice in the Description page of Project Settings.


#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void ALBGameMode::PostLogin(APlayerController* NewPlayer) //플레이어 컨트롤러를 받아들인다
{
	Super::PostLogin(NewPlayer); //부모클래스의 PostLogin을 호출한다.

	/*
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num(); //게임 상태의 플레이어 배열의 개수를 가져온다.
	if (NumberOfPlayer == 2) {
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true; //서버 트래블을 사용한다.
			World->ServerTravel(FString("/Game/Maps/InGameMap?listen")); //서버 트래블을 통해 맵을 이동한다.
		}
	}
	*/
    int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
    UE_LOG(LogTemp, Warning, TEXT("now player: %d"), NumberOfPlayer);

    if (HasAuthority())  // 서버에서만
    {
        // 호스트 컨트롤러 찾아서 "버튼 활성화" 신호 주기
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (NumberOfPlayer >= 1) // 조건 충족했을 때만
            {
                // 호스트 HUD/위젯에서 버튼 활성화 시킬 수 있도록 Client RPC 호출
                ATFPlayerController* PlayerController = Cast<ATFPlayerController>(PC);
                if (PlayerController)
                {
                    PlayerController->ClientEnableStartButton();
                }
            }
        }
    }
}

void ALBGameMode::ServerStartMatch_Implementation()
{
    UWorld* World = GetWorld();
    if (World)
    {
        bUseSeamlessTravel = true;
        World->ServerTravel(FString("/Game/Maps/InGameMap?listen"));
    }
}
