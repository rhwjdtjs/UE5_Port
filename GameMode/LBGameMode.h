#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LBGameMode.generated.h"

/**
 * �κ�(Game Lobby)�� ���Ӹ�� Ŭ����
 * - �÷��̾ ������ �� ó���ϴ� ������
 * - ���� ������ ��ġ(����)�� �����ϴ� ������ ����Ѵ�.
 */
UCLASS()
class UNREALPROJECT_7A_API ALBGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override; // �÷��̾� ��Ʈ�ѷ��� �޾Ƶ��δ�.
	UFUNCTION(Server, Reliable)
	void ServerStartMatch(); // �������� ��ġ�� �����ϴ� �Լ�
};
