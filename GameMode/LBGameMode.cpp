
#include "LBGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"

/////////////////////////////////////////////////////////////
// 기능:
//   새로운 플레이어가 로비에 접속(PostLogin)할 때 호출된다.
//   현재 접속 중인 플레이어 수를 확인하고, 서버라면 호스트의 UI를 업데이트한다.
//
// 알고리즘:
//   - GameState->PlayerArray.Num()을 통해 현재 접속 인원 계산.
//   - 서버(HasAuthority() == true)만 아래 로직 수행.
//   - GetWorld()->GetFirstPlayerController()로 호스트 컨트롤러를 얻는다.
//   - 인원이 조건(예: 1명 이상)에 도달하면
//     해당 컨트롤러의 ClientEnableStartButton()을 호출하여 UI 버튼을 활성화.
//   - 이 RPC는 Client 전용이므로, 서버 → 호스트 클라이언트로 실행된다.
/////////////////////////////////////////////////////////////
void ALBGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	UE_LOG(LogTemp, Warning, TEXT("now player: %d"), NumberOfPlayer);

	if (HasAuthority())  // 서버에서만 실행
	{
		// 첫 번째(호스트) PlayerController 가져오기
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (NumberOfPlayer >= 1) // 조건 충족 시
			{
				// 호스트 HUD/위젯에서 “Start” 버튼 활성화
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
// 기능:
//   서버에서 게임(매치)을 시작하는 함수.
//
// 알고리즘:
//   - Server RPC이므로 반드시 서버에서만 실행된다.
//   - GetWorld()로 월드 참조 획득 후 Seamless Travel 활성화.
//   - ServerTravel("/Game/Maps/InGameMap?listen") 호출로
//     클라이언트 접속을 유지한 채 인게임 맵으로 전환.
//   - “?listen” 옵션은 해당 인스턴스를 리슨 서버로 설정한다.
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
