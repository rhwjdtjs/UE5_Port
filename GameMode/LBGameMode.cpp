// Fill out your copyright notice in the Description page of Project Settings.


#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void ALBGameMode::PostLogin(APlayerController* NewPlayer) //�÷��̾� ��Ʈ�ѷ��� �޾Ƶ��δ�
{
	Super::PostLogin(NewPlayer); //�θ�Ŭ������ PostLogin�� ȣ���Ѵ�.

	/*
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num(); //���� ������ �÷��̾� �迭�� ������ �����´�.
	if (NumberOfPlayer == 2) {
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true; //���� Ʈ������ ����Ѵ�.
			World->ServerTravel(FString("/Game/Maps/InGameMap?listen")); //���� Ʈ������ ���� ���� �̵��Ѵ�.
		}
	}
	*/
    int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
    UE_LOG(LogTemp, Warning, TEXT("now player: %d"), NumberOfPlayer);

    if (HasAuthority())  // ����������
    {
        // ȣ��Ʈ ��Ʈ�ѷ� ã�Ƽ� "��ư Ȱ��ȭ" ��ȣ �ֱ�
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (NumberOfPlayer >= 1) // ���� �������� ����
            {
                // ȣ��Ʈ HUD/�������� ��ư Ȱ��ȭ ��ų �� �ֵ��� Client RPC ȣ��
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
