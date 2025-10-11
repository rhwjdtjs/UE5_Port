// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerState.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "UnrealProject_7A/HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h" //네트워크 관련 헤더 파일을 포함시킨다.

// ============================================================
//  TFPlayerState.cpp — 기능 및 알고리즘 설명 문서화 버전
// ============================================================


// ============================================================
// [네트워크 복제 등록] GetLifetimeReplicatedProps()
// ------------------------------------------------------------
// 기능 요약 : 
//   - Defeats(처치 수) 변수를 네트워크 복제 대상으로 등록한다.
// 사용 알고리즘 : 
//   1. 부모 클래스의 복제 설정 상속
//   2. DOREPLIFETIME 매크로를 이용해 Defeats를 클라이언트에 복제
// ============================================================
void ATFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //부모 클래스의 GetLifetimeReplicatedProps 함수를 호출한다.

	DOREPLIFETIME(ATFPlayerState, Defeats); //Defeats 변수를 네트워크로 복제할 수 있도록 설정한다.
}
// ============================================================
// [점수 복제 응답] OnRep_Score()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 점수가 변경되어 클라이언트로 복제될 때 자동 호출된다.
//   - HUD 점수와 스코어보드를 실시간 갱신한다.
// 사용 알고리즘 : 
//   1. GetPawn()으로 캐릭터 참조 확보
//   2. 캐릭터를 통해 PlayerController 캐싱
//   3. Controller의 SetHUDScore() 호출 → HUD 반영
//   4. UpdateScoreboard() 호출 → UI 갱신
// ============================================================
void ATFPlayerState::OnRep_Score()
{
	Super::OnRep_Score(); //부모 클래스의 OnRep_Score 함수를 호출한다.

	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if(TFPlayerController) {
			TFPlayerController->SetHUDScore(GetScore()); //플레이어 컨트롤러의 SetHUDScore 함수를 호출하여 HUD에 점수를 설정한다.
			TFPlayerController->UpdateScoreboard();
		}
	}
}
// ============================================================
// [점수 추가] AddToScore()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 점수를 누적시키고 HUD 갱신을 직접 트리거한다.
// 사용 알고리즘 : 
//   1. HasAuthority() 검사로 서버인지 확인
//   2. 점수 누적 (SetScore())
//   3. OnRep_Score() 수동 호출 (HUD 업데이트 보장)
// ============================================================
void ATFPlayerState::AddToScore(float ScoreAmount)
{
	if (HasAuthority())
	{
		SetScore(GetScore() + ScoreAmount);
		// 서버에서는 OnRep_Score가 자동으로 호출되지 않으므로, 직접 호출하여 HUD를 업데이트합니다.
		// 클라이언트에서는 점수가 복제된 후 자동으로 OnRep_Score가 호출됩니다.
		OnRep_Score();
	}
	
}
// ============================================================
// [처치 수 복제 응답] OnRep_Defeats()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 Defeats 값이 갱신되어 클라이언트로 복제될 때 호출된다.
//   - HUD 처치 수 및 스코어보드 갱신.
// 사용 알고리즘 : 
//   1. 캐릭터 참조 확보 → PlayerController 참조 갱신
//   2. HUD의 SetHUDDefeats() 호출
//   3. UpdateScoreboard()로 UI 갱신
// ============================================================
void ATFPlayerState::OnRep_Defeats()
{
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //플레이어 컨트롤러의 SetHUDDefeats 함수를 호출하여 HUD에 처치 수를 설정한다.
			TFPlayerController->UpdateScoreboard();
		}
	}
}
// ============================================================
// [처치 수 추가] AddToDefeats()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 Defeats(처치 수)를 누적하고 HUD에 반영한다.
// 사용 알고리즘 : 
//   1. Defeats 값 증가
//   2. 캐릭터 및 Controller 참조 확보
//   3. HUD의 SetHUDDefeats() 호출 → 실시간 갱신
// ============================================================
void ATFPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount; //Defeats 변수를 증가시킨다.
	TFCharacter = TFCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetPawn()) : TFCharacter; //캐릭터의 포인터를 저장한다.
	if (TFCharacter) {
		TFPlayerController = TFPlayerController == nullptr ? Cast<ATFPlayerController>(TFCharacter->Controller) : TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
		if (TFPlayerController) {
			TFPlayerController->SetHUDDefeats(Defeats); //플레이어 컨트롤러의 SetHUDDefeats 함수를 호출하여 HUD에 처치 수를 설정한다.
		}
	}
}
// ============================================================
// [플레이어 이름 복제 응답] OnRep_PlayerName()
// ------------------------------------------------------------
// 기능 요약 : 
//   - PlayerState의 이름이 서버에서 변경되어 복제될 때 호출된다.
//   - OverheadWidget 및 스코어보드 이름 갱신 담당.
// 사용 알고리즘 : 
//   1. GetPawn()으로 캐릭터 확인
//   2. 캐릭터의 OverheadWidget 가져오기
//   3. ShowPlayerNetRole()로 UI 반영
//   4. PlayerController의 UpdateScoreboard() 호출
// ============================================================
void ATFPlayerState::OnRep_PlayerName()
{
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn) return;
	TFCharacter = Cast<ATimeFractureCharacter>(OwnerPawn);
	if (TFCharacter)
		if (UOverheadWidget* W = TFCharacter->GetOverheadWidget())
			W->ShowPlayerNetRole(OwnerPawn);

	if (ATFPlayerController* PC = Cast<ATFPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->UpdateScoreboard(); // 이름 복제 완료되면 스코어보드 다시 갱신
	}
}




