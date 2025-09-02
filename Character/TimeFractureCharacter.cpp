// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
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
	// StandingCameraOffset = FVector(0.f, 30.f, 143.f);    //카메라의 상대 위치를 설정한다. 캐릭터의 머리 위에 카메라가 위치하도록 한다.
	// CrouchingCameraOffset = FVector(0.f, 0.f, 90.f); // 더 낮은 위치
}
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
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump); //스페이스바를 눌렀을 때 점프
	//프로젝트 세팅에 저장된 키의 이름을 바인드한다. this ->이 함수의 있는 함수를 불러옴
}


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
void ATimeFractureCharacter::Turn(float Value)
{
	AddControllerYawInput(Value); 
}
void ATimeFractureCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATimeFractureCharacter::EquipButton()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		if (HasAuthority()) { //서버에서 실행되는 경우
			CombatComponent->EquipWeapon(OverlappingWeapon);//겹치는 무기를 장착한다.
		}
		else { //클라이언트에서 실행되는 경우
			ServerEquipButton(); //서버에서 장착 버튼을 누른다.
		}
	}
}
void ATimeFractureCharacter::CrouchButton()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (bIsCrouched) {
		UnCrouch(); //크라우치 상태가 아니면 크라우치를 해제한다.
	}
	Crouch(); //크라우치 버튼을 누르면 크라우치한다.
}
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
void ATimeFractureCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return; //게임플레이가 비활성화된 경우 이동하지 않음
	if (CombatComponent) {
		CombatComponent->Reload(); //전투 컴포넌트의 재장전을 호출한다.
	}
}
void ATimeFractureCharacter::GrenadeButtonPressed()
{
	if (CombatComponent) {
		CombatComponent->ThrowGrenade(); //전투 컴포넌트의 수류탄 투척을 호출한다.
	}
}
void ATimeFractureCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCursor)
{
	if (bisElimmed) return;
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth); //체력을 감소시키고, 최소값은 0, 최대값은 최대 체력으로 제한한다.
	UpdateHUDHealth(); //HUD의 체력을 업데이트한다.
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
void ATimeFractureCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState(); //부모 클래스의 OnRep_PlayerState 함수를 호출한다.
	PollInit(); //플레이어 상태를 초기화한다.
}
void ATimeFractureCharacter::ServerSetActorRotation_Implementation(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
}
void ATimeFractureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);//부모 클래스의 복제 속성을 가져온다.
	DOREPLIFETIME_CONDITION(ATimeFractureCharacter, OverlappingWeapon, COND_OwnerOnly); //OVERLAPPINGWEAPON을 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
	DOREPLIFETIME(ATimeFractureCharacter, Health); //Health를 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
	DOREPLIFETIME(ATimeFractureCharacter, bDisableGameplay);//bDisableGameplay를 복제한다.
	DOREPLIFETIME(ATimeFractureCharacter, MoveRotation);//bisElimmed를 복제한다.
}
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
	}
}
void ATimeFractureCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon) {
		CombatComponent->EquippedWeapon->DropWeapon(); //장착된 무기를 떨어뜨린다.
	}
	MulticastElim(); //서버에서 클라이언트로 제거를 알린다.
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ATimeFractureCharacter::ElimTimerFinished, ElimDelay);
}
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
void ATimeFractureCharacter::ElimTimerFinished()
{
	ATFGameMode* TFGameMode = GetWorld()->GetAuthGameMode<ATFGameMode>(); //현재 게임 모드를 ATFGameMode로 캐스팅한다.
	if (TFGameMode) {
		TFGameMode->RequestRespawn(this, Controller); //게임 모드에 제거된 캐릭터의 재생성을 요청한다.
	}
}
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

void ATimeFractureCharacter::ServerEquipButton_Implementation()
{
	if (CombatComponent) {
		CombatComponent->EquipWeapon(OverlappingWeapon); //겹치는 무기를 장착한다.
	}
}

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



void ATimeFractureCharacter::OnRep_Health()
{
	UpdateHUDHealth(); //HUD의 체력을 업데이트한다.
	PlayHitReactMontage(); //피격 애니메이션을 재생한다.
}

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
	UpdateHUDHealth();
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
void ATimeFractureCharacter::UpdateHUDHealth()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//플레이어 컨트롤러를 캐스팅하여 설정한다.
	if (TfPlayerController) {
		TfPlayerController->SetHUDHealth(Health, MaxHealth); //플레이어 컨트롤러의 HUD에 체력을 설정한다.
	}
}
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TfPlayerState == nullptr)
	{
		PollInit();
	}
	float TargetArmLength = IsAiming() ? AimCameraOffset : normalAimCameraOffset; // 앉으면 더 가까이
	float NewArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector TargetSocketOffset = bIsCrouched ? CrouchingCameraOffset : NormalOffset;
	FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = NewSocketOffset;
	AimOffset(DeltaTime); //조준 오프셋을 계산한다.
	HideCameraIfCharacterClose(); //캐릭터가 가까이 있을 때 카메라를 숨긴다.
}

