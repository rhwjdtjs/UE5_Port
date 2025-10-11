// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Kismet/KismetMathLibrary.h" // 추가된 헤더 파일
#include "GameFramework/CharacterMovementComponent.h" // 추가된 헤더 파일
#include "TFAniminstance.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "UnrealProject_7A/GameMode/TFGameMode.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h" // 캡슐 컴포넌트 헤더 파일 추가
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
#include "UnrealProject_7A/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealProject_7A/TFComponents/BuffComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "UnrealProject_7A/HUD/ChatWidget.h"
#include "UnrealProject_7A/HUD/TFHUD.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "TFAniminstance.h"
#include "Sound/SoundCue.h"
#include "UnrealProject_7A/TFComponents/WireComponent.h"
// ============================================================
// [생성자] ATimeFractureCharacter::ATimeFractureCharacter()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 캐릭터의 핵심 구성요소(카메라, 컴포넌트, 충돌, 입력 가능 설정 등)를 초기화
//   - 전투, 버프, 와이어 시스템과 카메라 붐, 위젯 등의 부착 및 복제 설정
// 사용 알고리즘 : 
//   1. Tick 활성화 및 카메라 붐(SpringArm) 생성 → 메쉬에 부착
//   2. FollowCamera 생성 후 SpringArm의 끝(Socket)에 부착
//   3. 전투/버프 컴포넌트 생성 후 복제 활성화
//   4. 크라우치, 충돌 채널, 메쉬 반응 설정
//   5. 수류탄/와이어 등 부속 컴포넌트 초기화
// ============================================================
ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
    // Replace the line causing the error with the following:
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //카메라 붐 컴포넌트 생성
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->bUsePawnControlRotation = false; //카메라 붐이 캐릭터의 회전을 따라가도록 설정
	bUseControllerRotationYaw = false; // 카메라 방향을 유지
	GetCharacterMovement()->bOrientRotationToMovement =false; // 이동 방향으로 회전하지 않음
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //루트컴포넌트에 부착
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //루트컴포넌트에 부착
	CombatComponent = CreateDefaultSubobject<UCBComponent>(TEXT("CombatComponent")); //전투 컴포넌트 생성
	CombatComponent->SetIsReplicated(true); //복제 가능하게 설정
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent")); //버프 컴포넌트 생성
	BuffComponent->SetIsReplicated(true); //복제 가능하게 설정
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //캐릭터가 크라우치할 수 있도록 설정한다.
	GetMesh()->SetCollisionObjectType(ECC_SkelatalMesh); //메쉬의 충돌 객체 유형을 스켈레탈 메쉬로 설정한다.
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //메쉬가 시야 채널에 반응하지 않도록 설정한다.
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); //메쉬가 시야 채널에 반응하지 않도록 설정한다.

	AttachedGrenade = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket")); //수류탄 메쉬를 캐릭터의 메쉬에 부착한다.
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision); //수류탄 메쉬의 충돌을 비활성화한다.

	WireComponent = CreateDefaultSubobject<UWireComponent>(TEXT("WireComponent")); //와이어 컴포넌트 생성
	// StandingCameraOffset = FVector(0.f, 30.f, 143.f);    //카메라의 상대 위치를 설정한다. 캐릭터의 머리 위에 카메라가 위치하도록 한다.
	// CrouchingCameraOffset = FVector(0.f, 0.f, 90.f); // 더 낮은 위치
}
// ============================================================
// [입력 초기화] ATimeFractureCharacter::SetupPlayerInputComponent()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 모든 조작 키, 마우스 입력, UI 인터랙션 등을 언리얼 입력 시스템에 등록
// 사용 알고리즘 : 
//   1. 축 입력 : MoveForward, MoveRight, Turn, LookUp
//   2. 액션 입력 : Equip, Fire, Reload, Grenade, Dive 등
//   3. UI 입력 : Chat, ChatSubmit, ChatCancel (채팅 인터페이스 조작)
//   4. 특수 입력 : Wire (와이어 발사/해제), SwapWeapon
// ============================================================
void ATimeFractureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATimeFractureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATimeFractureCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATimeFractureCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATimeFractureCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ATimeFractureCharacter::EquipButton); //키보드의 E키를 눌렀을 때 EquipButton 함수를 호출한다.
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATimeFractureCharacter::CrouchButton); //키보드의 C키를 눌렀을 때 CrouchButton 함수를 호출한다.
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATimeFractureCharacter::AimButton); //우클릭키를 눌렀을 때 AimButton 함수를 호출한다.
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATimeFractureCharacter::AimButtonRelease); //우클릭키를 떼었을 때 AimButtonRelease 함수를 호출한다.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATimeFractureCharacter::FireButtonPressed); //우클릭키를 눌렀을 때 AimButton 함수를 호출한다.
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATimeFractureCharacter::FireButtonReleased); //우클릭키를 떼었을 때 AimButtonRelease 함수를 호출한다.
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATimeFractureCharacter::ReloadButtonPressed); //키보드의 C키를 눌렀을 때 CrouchButton 함수를 호출한다.
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ATimeFractureCharacter::GrenadeButtonPressed); //G키를 눌렀을 때 수류탄 투척 애니메이션 재생
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATimeFractureCharacter::Jump); //스페이스바를 눌렀을 때 점프
    PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &ATimeFractureCharacter::SwapButtonPressed);
	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &ATimeFractureCharacter::HandleChatKey);
	PlayerInputComponent->BindAction("ChatCancel", IE_Pressed, this, &ATimeFractureCharacter::HandleChatCancel);
	PlayerInputComponent->BindAction("ChatSubmit", IE_Pressed, this, &ATimeFractureCharacter::HandleChatSubmit);
	PlayerInputComponent->BindAction("Dive", IE_Pressed, this, &ATimeFractureCharacter::Dive);
	PlayerInputComponent->BindAction("Wire", IE_Pressed, this, &ATimeFractureCharacter::WireButtonPressed); //와이어 액션 바인딩
	//프로젝트 세팅에 저장된 키의 이름을 바인드한다. this ->이 함수의 있는 함수를 불러옴
}
// ============================================================
// [채팅 열기] HandleChatKey()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 채팅 키 입력 시 채팅 위젯이 없으면 생성 후 열기
// 사용 알고리즘 : 
//   1. PlayerController → HUD → ChatWidget 접근
//   2. 위젯 미생성 시 생성 후 Viewport에 추가
//   3. 닫혀 있다면 OpenChat() 호출
// ============================================================
void ATimeFractureCharacter::HandleChatKey()//0913 채팅
{
	ATFPlayerController* PC = Cast<ATFPlayerController>(Controller);
	if (PC)
	{
		ATFHUD* HUD = Cast<ATFHUD>(PC->GetHUD());
		if (HUD && HUD->ChatWidgetClass)
		{
			if (!HUD->ChatWidget) // 아직 생성 안 됐다면
			{
				HUD->ChatWidget = CreateWidget<UChatWidget>(PC, HUD->ChatWidgetClass);
				if (HUD->ChatWidget)
				{
					HUD->ChatWidget->AddToViewport();
				}
			}

			if (HUD->ChatWidget && !HUD->ChatWidget->IsChatOpen())
			{
				HUD->ChatWidget->OpenChat();
			}
		}
	}
}
// ============================================================
// [채팅 입력 전송] HandleChatSubmit()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 엔터키 입력 시 현재 채팅 내용을 서버로 전송하고 UI 닫기
// 사용 알고리즘 : 
//   1. HUD의 ChatWidget 확인 후 SubmitAndClose() 호출
// ============================================================
void ATimeFractureCharacter::HandleChatSubmit()//0913 채팅
{
	ATFPlayerController* PC = Cast<ATFPlayerController>(Controller);
	if (PC)
	{
		ATFHUD* HUD = Cast<ATFHUD>(PC->GetHUD());
		if (HUD && HUD->ChatWidget && HUD->ChatWidget->IsChatOpen())
		{
			HUD->ChatWidget->SubmitAndClose();   // Enter → 전송 후 닫기
		}
	}
}
// ============================================================
// [채팅 취소] HandleChatCancel()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 입력 중 채팅을 취소하고 입력 UI를 닫는다
// 사용 알고리즘 : 
//   1. HUD의 ChatWidget 확인 후 CancelAndClose() 호출
// ============================================================
void ATimeFractureCharacter::HandleChatCancel() // y → 취소0913 채팅
{
	ATFPlayerController* PC = Cast<ATFPlayerController>(Controller);
	if (PC)
	{
		ATFHUD* HUD = Cast<ATFHUD>(PC->GetHUD());
		if (HUD && HUD->ChatWidget && HUD->ChatWidget->IsChatOpen())
		{
			HUD->ChatWidget->CancelAndClose();
		}
	}
}
// ============================================================
// [구르기 입력 처리] Dive()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 플레이어가 회피(구르기) 입력 시 서버 RPC를 통해 모든 클라이언트에 반영
// 사용 알고리즘 : 
//   1. 재장전, 와이어, 중복 회피 여부 검사
//   2. 로컬 클라 → ServerDivePressed() RPC 호출
//   3. 서버 → MulticastDive()로 모든 클라에 실행
// ============================================================
void ATimeFractureCharacter::Dive() //0914 구르기
{
	if (CombatComponent->CombatState == ECombatState::ECS_Reloading) return;
	if (CombatComponent->EquippedWeapon == nullptr) return;
	if (bIsDodging) return;
	if (WireComponent && WireComponent->IsAttached())
	{
		return;
	}
	// 로컬에서만 서버에 요청
	if (!HasAuthority())
	{
		ServerDivePressed();
		return;
	}

	// 서버는 바로 실행
	MulticastDive();
}
// ============================================================
// [서버 RPC] ServerDivePressed()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트의 Dive 입력을 서버가 수신하여 모든 클라이언트로 전파
// 사용 알고리즘 : 
//   - MulticastDive() 호출
// ============================================================
void ATimeFractureCharacter::ServerDivePressed_Implementation() //0914 구르기
{
	if (CombatComponent->CombatState == ECombatState::ECS_Reloading) return;
	if (bIsDodging) return;
	if (WireComponent && WireComponent->IsAttached())
	{
		return;
	}
	MulticastDive(); // 서버가 모든 클라에 전달
}
// ============================================================
// [멀티캐스트 RPC] MulticastDive()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 모든 클라이언트에서 구르기 애니메이션 실행 및 상태 제어
// 사용 알고리즘 : 
//   1. AnimInstance 가져와 DiveMontage 실행
//   2. 애니메이션 종료 시 콜백으로 상태 복구
// ============================================================
void ATimeFractureCharacter::MulticastDive_Implementation() //0914 구르기
{
	if (CombatComponent->CombatState == ECombatState::ECS_Reloading) return;
	if (bIsDodging) return;

	UTFAniminstance* TFAnim = Cast<UTFAniminstance>(GetMesh()->GetAnimInstance());
	if (!TFAnim || TFAnim->bIsInAir || TFAnim->bIsCrouching) return;

	if (DiveMontage)
	{
		bDisableGameplay = true;
		bIsDodging = true;

		TFAnim->Montage_Play(DiveMontage);
		TFAnim->Montage_JumpToSection(FName("Dive"));

		// 애니 끝나면 초기화
		FOnMontageEnded EndDelegate;
		EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
			bDisableGameplay = false;
			bIsDodging = false;
			});
		TFAnim->Montage_SetEndDelegate(EndDelegate, DiveMontage);
	}
}
// ============================================================
// [전진 이동] MoveForward()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 컨트롤러 방향을 기준으로 전후 이동
// 사용 알고리즘 : 
//   1. Yaw 회전값으로 FRotationMatrix 생성
//   2. 전방 단위 벡터를 계산하여 AddMovementInput()
//   3. 무기 미장착 시 회전 보간 처리 (RInterpTo)
// ============================================================
void ATimeFractureCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f && CombatComponent && CombatComponent->EquippedWeapon) //플레이어가 컨트롤러를 갖고 있나 && 움직이고있나
	{
		//컨트롤러의 전방으로 보낸다.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //컨트롤러의 회전을 담당한다.
		//YawRotation: 컨트롤러의 Yaw 값(좌우 회전만 고려)을 가지고 FRotator 생성
		//피치(X축 회전), 롤(Z축 회전)은 무시함 → 캐릭터가 지면과 수평으로만 회전하게 하려는 의도
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); //전방벡터를 가져와서 초기화 , 
		//RotationMatrix(YawRotation): 이 로테이터를 기반으로 회전 행렬을 생성
		//이 회전 행렬은 캐릭터가 바라보는 방향 정보 등을 수학적으로 담고 있음
		// .GetUnitAxis(EAxis::X): 회전 행렬에서 X축(전방 방향) 의 단위 벡터를 가져옴
		//즉, 현재 바라보는 방향 기준으로 전방이 어디인지 알려주는 벡터
		//로테이터로 회전 매트릭스를 만들 수 있고 회전 매트릭스에 정보가 담겨있음, frotator에서 frotationmatrix를 만들고 이를 단위축이라 부르고 fvector를 반환한다.
		//지면과 평행한 방향을 나타냄
		AddMovementInput(Direction, Value); //방향과 값을 취하여 캐릭터가 해당방향으로 이동하게 한다. , 속도와 가속도가 필요하다.
	}
	else if(Controller != nullptr && FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		float TargetYaw = Controller->GetControlRotation().Yaw;
		if (Value < 0.f)
			TargetYaw += 180.f;

		FRotator TargetRotation(0.f, TargetYaw, 0.f);

		// 현재 회전에서 목표 회전으로 부드럽게 보간
		float InterpSpeed = 10.f; // 값이 클수록 더 빠르게 회전
		MoveRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		if (HasAuthority())
		{
			SetActorRotation(MoveRotation);
		}
		else
		{
			SetActorRotation(MoveRotation); // 클라에서도 즉시 회전 적용
			ServerSetActorRotation(MoveRotation);
		}

		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, 1.f);
	}
}
// ============================================================
// [좌우 이동] MoveRight()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 컨트롤러 방향을 기준으로 좌우 이동
// 사용 알고리즘 : 
//   1. 컨트롤러의 Yaw 회전으로 방향 벡터 생성
//   2. AddMovementInput()으로 이동
// ============================================================
void ATimeFractureCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (CombatComponent && CombatComponent->EquippedWeapon) {
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //컨트롤러의 회전을 담당한다.
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //전방벡터를 가져와서 초기화 , 
		//로테이터로 회전 매트릭스를 만들 수 있고 회전 매트릭스에 정보가 담겨있음, frotator에서 frotationmatrix를 만들고 이를 단위축이라 부르고 fvector를 반환한다.
		//지면과 평행한 방향을 나타냄
		AddMovementInput(Direction, Value); //방향과 값을 취하여 캐릭터가 해당방향으로 이동하게 한다. , 속도와 가속도가 필요하다.
	}
	else if (Controller != nullptr && FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		float TargetYaw = Controller->GetControlRotation().Yaw;
		if (Value > 0.f)
			TargetYaw += 90.f;
		else if (Value < 0.f)
			TargetYaw -= 90.f;

		FRotator TargetRotation(0.f, TargetYaw, 0.f);

		float InterpSpeed = 10.f;
		MoveRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		if (HasAuthority())
		{
			SetActorRotation(MoveRotation);
		}
		else
		{
			SetActorRotation(MoveRotation); // 클라에서도 즉시 회전 적용
			ServerSetActorRotation(MoveRotation);
		}

		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, 1.f);
	}
}
// ============================================================
// [카메라 회전 입력] Turn(), LookUp()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 마우스 혹은 패드의 회전 입력을 처리하여 카메라 방향 변경
// 사용 알고리즘 : 
//   1. AddControllerYawInput(), AddControllerPitchInput() 이용
// ============================================================
void ATimeFractureCharacter::Turn(float Value)
{
	AddControllerYawInput(Value); 
}
void ATimeFractureCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
// ============================================================
// [무기 장착 입력] EquipButton()
// ------------------------------------------------------------
// 기능 요약 : 
//   - E키 입력 시 무기 장착 또는 교체 요청
// 사용 알고리즘 : 
//   1. 서버 권한일 경우 → 직접 EquipWeapon() 호출
//   2. 클라이언트일 경우 → ServerEquipButton() RPC 요청
// ============================================================
void ATimeFractureCharacter::EquipButton()
{
	if (bDisableGameplay) return;
	if (CombatComponent) {
		if (HasAuthority()) { // 서버
			if (OverlappingWeapon) {
				CombatComponent->EquipWeapon(OverlappingWeapon);
			}
		//	else if (CombatComponent->ShouldSwapWeapons()) {
		//		CombatComponent->SwapWeapons();
	//		}
		}
		else { // 클라이언트
			ServerEquipButton();
		}
	}
}
// ============================================================
// [앉기 입력] CrouchButton()
// ------------------------------------------------------------
// 기능 요약 : 
//   - C키 입력 시 캐릭터의 크라우치 상태를 토글
// 사용 알고리즘 : 
//   - bIsCrouched 값 확인 후 Crouch() / UnCrouch() 호출
// ============================================================
void ATimeFractureCharacter::CrouchButton()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (bIsCrouched) {
		UnCrouch(); //크라우치 상태가 아니면 크라우치를 해제한다.
	}
	Crouch(); //크라우치 버튼을 누르면 크라우치한다.
}
// ============================================================
// [조준 시작 / 해제 입력] AimButton(), AimButtonRelease()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 우클릭 누르면 조준 시작, 떼면 조준 해제
// 사용 알고리즘 : 
//   - CombatComponent->SetAiming(bool) 호출
// ============================================================
void ATimeFractureCharacter::AimButton()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->SetAiming(true); //전투 컴포넌트의 조준 여부를 true로 설정한다.
	}
}
void ATimeFractureCharacter::AimButtonRelease()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->SetAiming(false); //전투 컴포넌트의 조준 여부를 false로 설정한다.
	}
}
// ============================================================
// [조준 오프셋 계산] AimOffset()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 현재 카메라 회전값을 기준으로 Pitch/Yaw 차이를 계산하여 애니메이션 보정에 사용
// 사용 알고리즘 : 
//   1. GetVelocity()로 이동 속도 확인
//   2. 회전 보간 후 AO_PITCH, AO_YAW 계산
// ============================================================
void ATimeFractureCharacter::AimOffset(float DeltaTime)
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
		FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bUseControllerRotationYaw = true;
	if (Speed > 0.f) // 움직일때만 좌우 yaw적용
	{
		BaseAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_YAW = 0.f;
		//bUseControllerRotationYaw = true;
	}

	AO_PITCH = GetBaseAimRotation().Pitch;
	if (AO_PITCH > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_PITCH = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_PITCH);
	}
}
// ============================================================
// [발사 입력 처리] FireButtonPressed(), FireButtonReleased()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 좌클릭으로 발사, 클릭 해제 시 중단
// 사용 알고리즘 : 
//   - CombatComponent->FireButtonPressed(bool) 호출
// ============================================================
void ATimeFractureCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(true); //전투 컴포넌트의 발사 버튼을 눌렀다고 설정한다.
	}
}
void ATimeFractureCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(false); //전투 컴포넌트의 발사 버튼을 떼었다고 설정한다.
	}
}
// ============================================================
// [재장전 입력 처리] ReloadButtonPressed()
// ------------------------------------------------------------
// 기능 요약 : 
//   - R키 입력 시 재장전 명령 전달
// 사용 알고리즘 : 
//   - CombatComponent->Reload() 호출
// ============================================================
void ATimeFractureCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->Reload(); //전투 컴포넌트의 재장전을 호출한다.
	}
}
// ============================================================
// [수류탄 투척 입력] GrenadeButtonPressed()
// ------------------------------------------------------------
// 기능 요약 : 
//   - G키 입력 시 수류탄 던지기 요청
// 사용 알고리즘 : 
//   - CombatComponent->ThrowGrenade() 호출
// ============================================================
void ATimeFractureCharacter::GrenadeButtonPressed()
{
	if (CombatComponent) {
		CombatComponent->ThrowGrenade(); //전투 컴포넌트의 수류탄 투척을 호출한다.
	}
}
// ============================================================
// [와이어 발사 / 해제 입력] WireButtonPressed(), WireButtonReleased()
// ------------------------------------------------------------
// 기능 요약 : 
//   - WireComponent를 이용해 와이어 사출 및 해제 처리
// 사용 알고리즘 : 
//   - FireWire(), ReleaseWire() 호출
// ============================================================
void ATimeFractureCharacter::WireButtonPressed()
{
	if (WireComponent) {
		WireComponent->FireWire();
	}
}
void ATimeFractureCharacter::WireButtonReleased()
{
	if (WireComponent) {
		WireComponent->ReleaseWire();
	}
}
// ============================================================
// [피해 처리] ReceiveDamage()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 피격 시 체력/실드 감소, 사운드/이펙트 재생 및 사망 판정 수행
// 사용 알고리즘 : 
//   1. 실드 우선 차감 → 남은 피해를 체력에 반영
//   2. HUD 갱신 및 피격 반응 애니메이션 재생
//   3. 체력 0일 경우 GameMode에 PlayerEliminated() 요청
// ============================================================
void ATimeFractureCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCursor)
{
	if (bisElimmed) return;
	float DamageToHealth = Damage;
	if (Shield > 0.f) {
		if (Shield >= Damage) {
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield); //실드를 감소시키고, 최소값은 0, 최대값은 최대 실드로 제한한다.
			DamageToHealth = 0.f; //실드가 남아있으면 체력에 데미지를 주지 않는다.
		}
		else {
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);//실드가 부족하면 남은 데미지를 체력에 준다.
			Shield = 0.f; //실드를 0으로 만든다.
		}
	}
	if (ATFPlayerController* AttackerController = Cast<ATFPlayerController>(InstigatorController))
	{
		AttackerController->ClientPlayHitConfirmSound(HitConfirmSound);
	}
	if (IsLocallyControlled())
	{
		BloodScreen(); // 본인 화면에 블러드 스크린 표시
	}
	else {
		ClientShowBloodScreen();
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth); //체력을 감소시키고, 최소값은 0, 최대값은 최대 체력으로 제한한다.
	MulticastHitCharacterSound();
	UpdateHUDHealth(); //HUD의 체력을 업데이트한다.
	UpdateHUDShield(); //HUD의 실드를 업데이트한다.
	PlayHitReactMontage(); //피격 애니메이션을 재생한다.
	if (Health == 0.f)	//체력이 0이 되면{
	{
		ATFGameMode* TFGameMode = GetWorld()->GetAuthGameMode<ATFGameMode>(); //현재 게임 모드를 ATFGameMode로 캐스팅한다.
		if (TFGameMode)
		{
            // 아래 코드는 TfPlayerController가 nullptr(아직 할당되지 않음)이면 Controller를 ATFPlayerController로 캐스팅하여 할당하고,
            // 이미 할당되어 있다면 기존 값을 그대로 사용한다.
            // 즉, TfPlayerController가 비어있을 때만 새로 캐스팅하여 할당하는 역할을 한다.
            // 아래 코드는 TfPlayerController가 nullptr(아직 할당되지 않음)이면 Controller를 ATFPlayerController로 캐스팅하여 할당하고,
            // 이미 할당되어 있다면 기존 값을 그대로 사용한다.
            // 즉, TfPlayerController가 비어있을 때만 새로 캐스팅하여 할당하는 역할을 한다.
            // 장점:
            // 1. 불필요한 캐스팅을 반복하지 않아 성능이 최적화된다.
            // 2. 코드가 간결해진다.
            // 3. 이미 올바르게 할당된 포인터를 재사용하므로 안정성이 높아진다.
            TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController; //플레이어 컨트롤러를 가져온다.
			ATFPlayerController* AttackerController = Cast<ATFPlayerController>(InstigatorController); //공격자의 컨트롤러를 가져온다.
			TFGameMode->PlayerEliminated(this, TfPlayerController, AttackerController);
		}
	}
	
}
// ============================================================
// [플레이어 상태 초기화] PollInit()
// ------------------------------------------------------------
// 기능 요약 : 
//   - PlayerState가 복제 완료된 뒤 HUD 초기화 및 점수 동기화 수행
// 사용 알고리즘 : 
//   - 서버: Score/Defeats 초기화
//   - 클라: HUD Score 수동 갱신
// ============================================================
void ATimeFractureCharacter::PollInit()
{
	if (TfPlayerState == nullptr)
	{
		TfPlayerState = GetPlayerState<ATFPlayerState>();
		if (TfPlayerState)
		{
			if (HasAuthority())
			{
				// 서버에서만 점수 초기화 (복제됨)
				TfPlayerState->AddToScore(0.f);
				TfPlayerState->AddToDefeats(0); //처치 수 초기화
			}
			else
			{
				// 클라이언트에서는 직접 HUD 업데이트
				TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;
				if (TfPlayerController)
				{
					TfPlayerController->SetHUDScore(TfPlayerState->GetScore());
					TfPlayerState->AddToDefeats(0); //처치 수 초기화
				}
			}
		}
	}
}
// ============================================================
// [OnRep PlayerState] OnRep_PlayerState()
// ------------------------------------------------------------
// 기능 요약 : 
//   - PlayerState 복제 시 호출되어 오버헤드 이름, HUD 재설정
// ============================================================
void ATimeFractureCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState(); //부모 클래스의 OnRep_PlayerState 함수를 호출한다.
	EnsureOverheadWidgetLocal();
	RefreshOverheadName();
	PollInit();
}
// ============================================================
// [서버 RPC] ServerSetActorRotation()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 캐릭터의 회전 정보를 서버에 전달하여 동기화
// ============================================================
void ATimeFractureCharacter::ServerSetActorRotation_Implementation(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
}
// ============================================================
// [복제 변수 등록] GetLifetimeReplicatedProps()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 언리얼 복제 시스템에 캐릭터 관련 변수 등록
// 사용 알고리즘 : 
//   - DOREPLIFETIME / DOREPLIFETIME_CONDITION 매크로로 선언
// ============================================================
void ATimeFractureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);//부모 클래스의 복제 속성을 가져온다.
	DOREPLIFETIME_CONDITION(ATimeFractureCharacter, OverlappingWeapon, COND_OwnerOnly); //OVERLAPPINGWEAPON을 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
	DOREPLIFETIME(ATimeFractureCharacter, Health); //Health를 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
	DOREPLIFETIME(ATimeFractureCharacter, Shield); //Shield를 복제한다.
	DOREPLIFETIME(ATimeFractureCharacter, bDisableGameplay);//bDisableGameplay를 복제한다.
	DOREPLIFETIME(ATimeFractureCharacter, MoveRotation);//bisElimmed를 복제한다.
	DOREPLIFETIME(ATimeFractureCharacter, WireComponent);
}
// ============================================================
// [컴포넌트 초기화 후 설정] PostInitializeComponents()
// ------------------------------------------------------------
// 기능 요약 : 
//   - CombatComponent 및 BuffComponent 초기값 세팅
// 사용 알고리즘 : 
//   - 이동속도, 점프력, 캐릭터 포인터 설정
// ============================================================
void ATimeFractureCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//캐릭터의 속성을 초기화하는 함수
	//이 함수는 모든 컴포넌트가 초기화된 후에 호출된다.
	//따라서 이 함수에서 캐릭터의 속성을 초기화하면, 모든 컴포넌트가 준비된 상태에서 속성이 초기화된다.
	//이 함수는 서버와 클라이언트 모두에서 호출된다.
	//따라서 이 함수에서 캐릭터의 속성을 초기화하면, 서버와 클라이언트 모두에서 캐릭터의 속성이 동일하게 초기화된다.
	//이 함수는 모든 컴포넌트가 초기화된 후에 호출된다.
	//따라서 이 함수에서 캐릭터의 속성을 초기화하면, 모든 컴포넌트가 준비된 상태에서 속성이 초기화된다.
	if (CombatComponent) {
		CombatComponent->Character = this; //캐릭터를 설정한다.
	}
	if (BuffComponent) {
		BuffComponent->Character = this; //캐릭터를 설정한다.	
		BuffComponent->SetInitialSpeeds(CombatComponent->baseWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched); //캐릭터의 초기 속도를 설정한다.
		BuffComponent->SetInitialJump(GetCharacterMovement()->JumpZVelocity); //캐릭터의 초기 점프 속도를 설정한다.
	}
}
// ============================================================
// [사망 동기화 RPC] MulticastElim()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트 전역에 캐릭터 사망 연출 및 입력 비활성화 전파
// 사용 알고리즘 : 
//   - 이동/충돌 비활성화, 애니메이션 재생, 스코프 숨김 처리
// ============================================================
void ATimeFractureCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon) {
		if (CombatComponent->EquippedWeapon) {
			if (CombatComponent->EquippedWeapon->bDestroyWeapon) {
				CombatComponent->EquippedWeapon->Destroy(); //장착된 무기를 파괴한다.
			}
			else {
				CombatComponent->EquippedWeapon->DropWeapon(); //장착된 무기를 떨어뜨린다.
			}
		}
		if(CombatComponent->SecondaryWeapon)
		{
			if (CombatComponent->SecondaryWeapon->bDestroyWeapon) {
				CombatComponent->SecondaryWeapon->Destroy(); //보조 무기를 파괴한다.
			}
			else {
				CombatComponent->SecondaryWeapon->DropWeapon(); //보조 무기를 떨어뜨린다.
			}
		}
	}
	MulticastElim(); //서버에서 클라이언트로 제거를 알린다.
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ATimeFractureCharacter::ElimTimerFinished, ElimDelay);
}
// ============================================================
// [사망 동기화 RPC] MulticastElim()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트 전역에 캐릭터 사망 연출 및 입력 비활성화 전파
// 사용 알고리즘 : 
//   - 이동/충돌 비활성화, 애니메이션 재생, 스코프 숨김 처리
// ============================================================
void ATimeFractureCharacter::MulticastElim_Implementation()
{
	if (TfPlayerController) {
		TfPlayerController->SetHUDWeaponAmmo(0); //HUD의 무기 탄약을 0으로 설정한다.
	}
	bisElimmed = true; //캐릭터가 제거되었음을 표시한다.
	PlayElimMontage(); //피격 애니메이션을 재생한다.
	//캐릭터 이동 비활성화
	GetCharacterMovement()->DisableMovement(); //캐릭터의 이동을 비활성화한다.
	GetCharacterMovement()->StopMovementImmediately(); //캐릭터의 이동을 즉시 중지한다.
	bDisableGameplay = true; //게임플레이를 비활성화한다.
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(false); //전투 컴포넌트의 발사 버튼을 떼었다고 설정한다.
	}
	//콜리전 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캐릭터의 캡슐 콜리전을 비활성화한다.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캐릭터의 메쉬 콜리전을 비활성화한다.
	bool bHideSniperScope = IsLocallyControlled() && CombatComponent && CombatComponent->bisAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope) {
		ShowSniperScopeWidget(false); //로컬에서 제어하는 경우 스나이퍼 스코프 위젯을 숨긴다.
	}
}
// ============================================================
// [리스폰 타이머 완료 처리] ElimTimerFinished()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 사망 후 일정 시간이 지나면 GameMode에 Respawn 요청
// ============================================================
void ATimeFractureCharacter::ElimTimerFinished()
{
	ATFGameMode* TFGameMode = GetWorld()->GetAuthGameMode<ATFGameMode>(); //현재 게임 모드를 ATFGameMode로 캐스팅한다.
	if (TFGameMode) {
		TFGameMode->RequestRespawn(this, Controller); //게임 모드에 제거된 캐릭터의 재생성을 요청한다.
	}
}
// ============================================================
// [리스폰 타이머 완료 처리] ElimTimerFinished()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 사망 후 일정 시간이 지나면 GameMode에 Respawn 요청
// ============================================================
void ATimeFractureCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) //겹치는 무기가 존재하면
	{
		OverlappingWeapon->ShowPickupWidget(true); //겹치는 무기 위젯을 표시한다.
	}
	if (LastWeapon) //겹치는 무기가 존재하면
	{
		LastWeapon->ShowPickupWidget(false); //겹치는 무기 위젯을 숨긴다.
	}
}
// ============================================================
// [서버 RPC] ServerEquipButton()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트가 장비 장착 요청 시 서버에서 처리
// 사용 알고리즘 : 
//   - OverlappingWeapon 존재 시 EquipWeapon() 호출
// ============================================================
void ATimeFractureCharacter::ServerEquipButton_Implementation()
{
	if (CombatComponent) {
		if (OverlappingWeapon) {
			CombatComponent->EquipWeapon(OverlappingWeapon); //겹치는 무기를 장착한다.
		}
	//	else if(CombatComponent->ShouldSwapWeapons())
	//	{
	//		CombatComponent->SwapWeapons(); //무기를 교체한다.
	//	}
	}
}
// ============================================================
// [카메라 근접 시 메쉬 숨김] HideCameraIfCharacterClose()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 1인칭 시점 시 카메라와 캐릭터가 가까워지면 메쉬 숨김
// 사용 알고리즘 : 
//   1. 카메라와 본체 거리 계산
//   2. 특정 임계값 이하이면 메쉬와 무기 비가시화
// ============================================================
void ATimeFractureCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return; //로컬에서 제어하지 않는 경우 함수를 종료한다.
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold) {
		GetMesh()->SetVisibility(false); //카메라가 캐릭터와 가까이 있을 때 메쉬를 숨긴다.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true; //장착된 무기의 메쉬를 숨긴다.
		}
	}
	else {
		GetMesh()->SetVisibility(true); //카메라가 캐릭터와 가까이 있을 때 메쉬를 숨긴다.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false; //장착된 무기의 메쉬를 숨긴다.
		}
	}
}
// ============================================================
// [실드 복제 응답] OnRep_Shield()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 실드 값 변동 시 HUD 갱신 및 피격 반응 재생
// ============================================================
void ATimeFractureCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield(); //HUD의 체력을 업데이트한다.
	if (Shield < LastShield) {
		PlayHitReactMontage(); //피격 애니메이션을 재생한다.
	}
}
void ATimeFractureCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth(); //HUD의 체력을 업데이트한다.
	if (Health < LastHealth) {
		PlayHitReactMontage(); //피격 애니메이션을 재생한다.
	}
}
// ============================================================
// [피격 사운드 전파] MulticastHitCharacterSound()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 피격 시 모든 클라이언트에서 피격 사운드 재생
// ============================================================
void ATimeFractureCharacter::MulticastHitCharacterSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, HitCharacterSound, GetActorLocation()); //장착 사운드를 재생한다.
}
// ============================================================
// [오버헤드 위젯 로컬 보장] EnsureOverheadWidgetLocal()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버·클라 관계없이 오버헤드 이름표 위젯이 정상 생성되도록 강제 초기화
// 사용 알고리즘 : 
//   1. WidgetClass 확인 → 없으면 생성
//   2. OwnerPlayer 설정 및 DrawAtDesiredSize 활성화
// ============================================================
void ATimeFractureCharacter::EnsureOverheadWidgetLocal()
{
	if (!OverheadWidget) return;

	// 가시성/세팅 강제
	OverheadWidget->SetVisibility(true, true);
	OverheadWidget->SetHiddenInGame(false);
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidget->SetDrawAtDesiredSize(true);
	OverheadWidget->SetOwnerNoSee(false);
	OverheadWidget->SetOnlyOwnerSee(false);

	// 이미 생성되어 있으면 패스
	if (OverheadWidget->GetUserWidgetObject()) return;

	TSubclassOf<UUserWidget> WC = OverheadWidget->GetWidgetClass();
	if (!WC) return;

	// ★ OwningPlayer 우선 시도(리스너 서버의 로컬 PC)
	APlayerController* LocalPC = nullptr;
	if (GEngine)
		LocalPC = GEngine->GetFirstLocalPlayerController(GetWorld());

	UUserWidget* NewUW = nullptr;
	if (LocalPC)
	{
		NewUW = CreateWidget<UUserWidget>(LocalPC, WC); // OwningPlayer 버전
	}
	if (!NewUW)
	{
		NewUW = CreateWidget<UUserWidget>(GetWorld(), WC); // WorldContext 버전
	}
	if (!NewUW) return;

	OverheadWidget->SetWidget(NewUW);
	OverheadWidget->InitWidget();
	NewUW->SetVisibility(ESlateVisibility::HitTestInvisible);
	UE_LOG(LogTemp, Warning, TEXT("NOT CLIENT"));
}
// ============================================================
// [오버헤드 이름 갱신] RefreshOverheadName()
// ------------------------------------------------------------
// 기능 요약 : 
//   - PlayerState 이름을 오버헤드 위젯에 반영
// 사용 알고리즘 : 
//   - BP/CPP 위젯 모두 대응 (‘DisplayText’ 이름 탐색 후 텍스트 갱신)
// ============================================================
void ATimeFractureCharacter::RefreshOverheadName()
{
	 if (!OverheadWidget) return;

    // 서버 뷰포트용 인스턴스 보장
    EnsureOverheadWidgetLocal();

    UUserWidget* UW = OverheadWidget->GetUserWidgetObject();
    if (!UW) return;

    const FString Name = (GetPlayerState() ? GetPlayerState()->GetPlayerName() : FString());

    // 1) C++ UOverheadWidget이면 기존 함수 사용
    if (UOverheadWidget* OW = Cast<UOverheadWidget>(UW))
    {
        OW->SetDisplayText(Name);          // 또는 OW->ShowPlayerNetRole(this);
        OW->SetVisibility(ESlateVisibility::HitTestInvisible);
        return;
    }

    // 2) BP UserWidget이어도 'DisplayText'를 찾아서 직접 세팅 (폴백)
    if (UTextBlock* TB = Cast<UTextBlock>(UW->GetWidgetFromName(TEXT("DisplayText"))))
    {
        TB->SetText(FText::FromString(Name));
        UW->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
	UE_LOG(LogTemp, Warning, TEXT("NOT CLIENT"));
}
// ============================================================
// [기본 무기 생성] SpawnDefaultWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 리스폰 또는 시작 시 기본 무기를 생성 후 장착
// 사용 알고리즘 : 
//   1. GameMode/World 참조 확인
//   2. DefaultWeaponClass 스폰 → EquipWeapon()
// ============================================================
void ATimeFractureCharacter::SpawnDefaultWeapon()
{
	ATFGameMode* TFGameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); //현재 게임 모드를 ATFGameMode로 캐스팅한다.
	UWorld* World = GetWorld();
	if (TFGameMode && World && !bisElimmed && DefaultWeaponClass) {
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true; //기본 무기는 제거될 때 파괴된다.
		if (CombatComponent) {
			CombatComponent->EquipWeapon(StartingWeapon); //전투 컴포넌트의 무기를 장착한다.
			
		}

	}
}
void ATimeFractureCharacter::UpdateHUDAmmo()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//플레이어 컨트롤러를 캐스팅하여 설정한다.
	if (TfPlayerController && CombatComponent && CombatComponent->EquippedWeapon) {
		TfPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo); //HUD의 탄약을 업데이트한다.
		TfPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo()); //HUD의 무기 탄약을 업데이트한다.
	}
}
// ============================================================
// [무기 겹침 설정] SetOverlappingWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 현재 플레이어가 닿은 무기 참조를 등록하고 위젯 표시
// ============================================================
void ATimeFractureCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false); //겹치는 무기 위젯을 숨긴다.
	}
	OverlappingWeapon = Weapon; //겹치는 무기를 설정한다.
	if (IsLocallyControlled()) //로컬에서 제어하는 경우
	{
		if (OverlappingWeapon) //겹치는 무기가 존재하면
		{
			OverlappingWeapon->ShowPickupWidget(true); //겹치는 무기 위젯을 표시한다.
		}
		
	}

}

bool ATimeFractureCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon); //전투 컴포넌트가 존재하고, 전투 컴포넌트의 무기가 장착되어 있는지 확인한다.
}

bool ATimeFractureCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bisAiming); //전투 컴포넌트가 존재하고, 전투 컴포넌트의 조준 여부를 반환한다.
}

UOverheadWidget* ATimeFractureCharacter::GetOverheadWidget() const
{
	if (OverheadWidget)
		return Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject());
	return nullptr;
}

ECombatState ATimeFractureCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX; //전투 컴포넌트가 존재하지 않으면 최대 전투 상태를 반환한다.
	return CombatComponent->CombatState; //전투 컴포넌트의 전투 상태를 반환한다.
}

AWeapon* ATimeFractureCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr)
		return nullptr;

	return CombatComponent->EquippedWeapon; //전투 컴포넌트의 무기를 반환한다.
}

void ATimeFractureCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if (animInstance && FireWeaponMontage) {
		animInstance->Montage_Play(FireWeaponMontage); //애니메이션 몽타주를 재생한다.
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip"); //조준 상태에 따라 섹션 이름을 설정한다.
		animInstance->Montage_JumpToSection(SectionName); //애니메이션 몽타주의 섹션으로 점프한다.
	}
}

void ATimeFractureCharacter::PlayElimMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if (animInstance && ElimMontage) {
		animInstance->Montage_Play(ElimMontage); //애니메이션 몽타주를 재생한다.
	}
}

void ATimeFractureCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || CombatComponent->CombatState == ECombatState::ECS_Reloading || 
		CombatComponent->CombatState==ECombatState::ECS_ThrowingGrenade) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if (animInstance && HitReactMontage) {
		animInstance->Montage_Play(HitReactMontage); //애니메이션 몽타주를 재생한다.
		FName SectionName("FromFront");
		animInstance->Montage_JumpToSection(SectionName); //애니메이션 몽타주의 섹션으로 점프한다.
	}
}

void ATimeFractureCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if (animInstance && ReloadMontage) {
		animInstance->Montage_Play(ReloadMontage); //애니메이션 몽타주를 재생한다.
		FName SectionName;
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		animInstance->Montage_JumpToSection(SectionName); //애니메이션 몽타주의 섹션으로 점프한다.
	}
}

void ATimeFractureCharacter::MultiCastHit_Implementation()
{
	PlayHitReactMontage(); //히트 리액트 몽타주를 재생한다.
}

FVector ATimeFractureCharacter::GetHitTarget() const
{
	if(CombatComponent==nullptr) 	return FVector();
	return CombatComponent->HitTarget; //전투 컴포넌트의 히트 타겟을 반환한다.

}


void ATimeFractureCharacter::PlayThrowGrendadeMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if(animInstance && ThrowGrenadeMontage) {
		animInstance->Montage_Play(ThrowGrenadeMontage); //애니메이션 몽타주를 재생한다.
	}
}

void ATimeFractureCharacter::ClientShowBloodScreen_Implementation()
{
	BloodScreen(); // 본인 클라에서만 실행
}

void ATimeFractureCharacter::Jump()
{
	if (bDisableGameplay) return;
	Super::Jump();
}
// ============================================================
// [초기화] BeginPlay()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 오버헤드 위젯 초기화, 기본 무기 스폰, HUD 설정, 데미지 델리게이트 바인딩
// 사용 알고리즘 : 
//   1. 위젯 OwnerPlayer 지정 (리스너 서버 대비)
//   2. HUD Health/Shield 초기 갱신
//   3. 서버면 OnTakeAnyDamage 바인딩
// ============================================================
void ATimeFractureCharacter::Destroyed()
{
	Super::Destroyed();
	ATFGameMode* TFGameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); //현재 게임 모드를 ATFGameMode로 캐스팅한다.
	bool bMatchNotInProgress = TFGameMode && TFGameMode->GetMatchState() !=MatchState::InProgress; //게임 모드가 존재하고, 게임이 진행 중이지 않은 경우
	if(CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress) {
		CombatComponent->EquippedWeapon->Destroy(); //장착된 무기를 파괴한다.
	}
}

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalOffset = CameraBoom->SocketOffset;
	// 바로 시도
	if (OverheadWidget)
	{
		OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
		OverheadWidget->SetDrawAtDesiredSize(true);
		OverheadWidget->SetOwnerNoSee(false);
		OverheadWidget->SetOnlyOwnerSee(false);

		if (GEngine)
		{
			if (APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(GetWorld()))
			{
				if (ULocalPlayer* LP = LocalPC->GetLocalPlayer())
				{
					// ★★★ 핵심: 리슨서버의 로컬 플레이어를 OwnerPlayer로 지정
					OverheadWidget->SetOwnerPlayer(LP);
				}
			}
		}
	}
	EnsureOverheadWidgetLocal();
	RefreshOverheadName();
	
	SpawnDefaultWeapon(); //기본 무기를 생성한다.
	UpdateHUDAmmo(); //HUD의 탄약을 업데이트한다.
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority()) //서버에서 실행되는 경우
	{
		bReplicates = true;
		SetReplicateMovement(true); //캐릭터의 이동이 네트워크를 통해 복제되도록 설정
		OnTakeAnyDamage.AddDynamic(this, &ATimeFractureCharacter::ReceiveDamage); //피해를 받았을 때 호출되는 함수를 바인딩한다.
	}
	if (AttachedGrenade) {
		AttachedGrenade->SetVisibility(false); //수류탄 메쉬를 숨긴다.
	}
}
void ATimeFractureCharacter::SwapButtonPressed()
{
	if(CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		if (HasAuthority()) { // 서버
			CombatComponent->SwapWeapons();
		}
		else { // 클라이언트
			ServerSwapButtonPressed();
		}
	}
}
void ATimeFractureCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (HasAuthority())
	{
		EnsureOverheadWidgetLocal();
		RefreshOverheadName();
	}
}
void ATimeFractureCharacter::ServerSwapButtonPressed_Implementation()
{
	if(CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
	}
}
void ATimeFractureCharacter::UpdateHUDHealth()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//플레이어 컨트롤러를 캐스팅하여 설정한다.
	if (TfPlayerController) {
		TfPlayerController->SetHUDHealth(Health, MaxHealth); //플레이어 컨트롤러의 HUD에 체력을 설정한다.
	}
}
void ATimeFractureCharacter::UpdateHUDShield()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//플레이어 컨트롤러를 캐스팅하여 설정한다.
	if (TfPlayerController) {
		TfPlayerController->SetHUDShield(Shield, MaxShield); //플레이어 컨트롤러의 HUD에 체력을 설정한다.
	}
}
// ============================================================
// [매 프레임 업데이트] Tick()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 각종 HUD, 조준 카메라, 회피 이동, 크로스헤어 오프셋, 무기 탄약 동기화 등
// 사용 알고리즘 : 
//   1. PollInit()으로 PlayerState 초기화 보장
//   2. 회피 중이면 전방 이동 지속
//   3. 카메라 거리·오프셋 보간 (FInterpTo, VInterpTo)
//   4. AimOffset() 호출
// ============================================================
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TfPlayerState == nullptr)
	{
		PollInit();
	}
	if (bIsDodging)
	{
		FVector ForwardDir = GetActorForwardVector();
		AddMovementInput(ForwardDir,1.f);
	}
	float TargetArmLength = IsAiming() ? AimCameraOffset : normalAimCameraOffset; // 앉으면 더 가까이
	float NewArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector TargetSocketOffset = bIsCrouched ? CrouchingCameraOffset : NormalOffset;
	FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = NewSocketOffset;
	AimOffset(DeltaTime); //조준 오프셋을 계산한다.
	HideCameraIfCharacterClose(); //캐릭터가 가까이 있을 때 카메라를 숨긴다.
	UpdateHUDAmmo();
}

