
#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"

/////////////////////////////////////////////////////////////
// ���:
//   ���ο� �÷��̾ �κ� ����(PostLogin)�� �� ȣ��ȴ�.
//   ���� ���� ���� �÷��̾� ���� Ȯ���ϰ�, ������� ȣ��Ʈ�� UI�� ������Ʈ�Ѵ�.
//
// �˰���:
//   - GameState->PlayerArray.Num()�� ���� ���� ���� �ο� ���.
//   - ����(HasAuthority() == true)�� �Ʒ� ���� ����.
//   - GetWorld()->GetFirstPlayerController()�� ȣ��Ʈ ��Ʈ�ѷ��� ��´�.
//   - �ο��� ����(��: 1�� �̻�)�� �����ϸ�
//     �ش� ��Ʈ�ѷ��� ClientEnableStartButton()�� ȣ���Ͽ� UI ��ư�� Ȱ��ȭ.
//   - �� RPC�� Client �����̹Ƿ�, ���� �� ȣ��Ʈ Ŭ���̾�Ʈ�� ����ȴ�.
/////////////////////////////////////////////////////////////
void ALBGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	UE_LOG(LogTemp, Warning, TEXT("now player: %d"), NumberOfPlayer);

	if (HasAuthority())  // ���������� ����
	{
		// ù ��°(ȣ��Ʈ) PlayerController ��������
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (NumberOfPlayer >= 1) // ���� ���� ��
			{
				// ȣ��Ʈ HUD/�������� ��Start�� ��ư Ȱ��ȭ
				ATFPlayerController* PlayerController = Cast<ATFPlayerController>(PC);
				if (PlayerController)
				{
					PlayerController->ClientEnableStartButton();
				}
			}
		}
	}
}


/////////////////////////////////////////////////////////////
// ���:
//   �������� ����(��ġ)�� �����ϴ� �Լ�.
//
// �˰���:
//   - Server RPC�̹Ƿ� �ݵ�� ���������� ����ȴ�.
//   - GetWorld()�� ���� ���� ȹ�� �� Seamless Travel Ȱ��ȭ.
//   - ServerTravel("/Game/Maps/InGameMap?listen") ȣ���
//     Ŭ���̾�Ʈ ������ ������ ä �ΰ��� ������ ��ȯ.
//   - ��?listen�� �ɼ��� �ش� �ν��Ͻ��� ���� ������ �����Ѵ�.
/////////////////////////////////////////////////////////////
void ALBGameMode::ServerStartMatch_Implementation()
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(FString("/Game/Maps/InGameMap?listen"));
	}
}
