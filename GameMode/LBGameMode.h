#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LBGameMode.generated.h"

/**
 * 로비(Game Lobby)용 게임모드 클래스
 * - 플레이어가 접속할 때 처리하는 로직과
 * - 서버 측에서 매치(게임)를 시작하는 역할을 담당한다.
 */
UCLASS()
class UNREALPROJECT_7A_API ALBGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override; // 플레이어 컨트롤러를 받아들인다.
	UFUNCTION(Server, Reliable)
	void ServerStartMatch(); // 서버에서 매치를 시작하는 함수
};
