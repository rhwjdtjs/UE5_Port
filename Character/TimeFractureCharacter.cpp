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

ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //카메라 붐 컴포넌트 생성
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->bUsePawnControlRotation = true; //카메라 붐이 캐릭터의 회전을 따라가도록 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //루트컴포넌트에 부착
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //루트컴포넌트에 부착
	CombatComponent = CreateDefaultSubobject<UCBComponent>(TEXT("CombatComponent")); //전투 컴포넌트 생성
	CombatComponent->SetIsReplicated(true); //복제 가능하게 설정
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //캐릭터가 크라우치할 수 있도록 설정한다.
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
	//프로젝트 세팅에 저장된 키의 이름을 바인드한다. this ->이 함수의 있는 함수를 불러옴
}


void ATimeFractureCharacter::MoveForward(float Value)//"컨트롤러가 바라보는 방향을 기준으로 캐릭터의 전방 벡터를 구하는 것"
{
	if (Controller != nullptr && Value != 0.f) //플레이어가 컨트롤러를 갖고 있나 && 움직이고있나
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
	
}
void ATimeFractureCharacter::MoveRight(float Value)
{
	//컨트롤러의 전방으로 보낸다.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //컨트롤러의 회전을 담당한다.
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //전방벡터를 가져와서 초기화 , 
	//로테이터로 회전 매트릭스를 만들 수 있고 회전 매트릭스에 정보가 담겨있음, frotator에서 frotationmatrix를 만들고 이를 단위축이라 부르고 fvector를 반환한다.
	//지면과 평행한 방향을 나타냄
	AddMovementInput(Direction, Value); //방향과 값을 취하여 캐릭터가 해당방향으로 이동하게 한다. , 속도와 가속도가 필요하다.
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
	if (bIsCrouched) {
		UnCrouch(); //크라우치 상태가 아니면 크라우치를 해제한다.
	}
	Crouch(); //크라우치 버튼을 누르면 크라우치한다.
}
void ATimeFractureCharacter::AimButton()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(true); //전투 컴포넌트의 조준 여부를 true로 설정한다.
	}
}
void ATimeFractureCharacter::AimButtonRelease()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(false); //전투 컴포넌트의 조준 여부를 false로 설정한다.
	}
}
void ATimeFractureCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	FRotator CurrentAimRotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);

	// 항상 Delta 계산
	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, BaseAimRotation);
	AO_YAW = DeltaAimRotation.Yaw;
	AO_PITCH = GetBaseAimRotation().Pitch;

	// 이동 중이면 BaseAimRotation 업데이트
	if (Speed > 0.f)
	{
		BaseAimRotation = CurrentAimRotation;
		bUseControllerRotationYaw = false; // 이동 중에는 컨트롤러 회전을 사용하지 않도록 설정
	}

	// 이동 시 컨트롤러 회전 따라가게 할지 여부는 스타일에 따라
	bUseControllerRotationYaw = (Speed > 0.f);
}
void ATimeFractureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATimeFractureCharacter, OverlappingWeapon, COND_OwnerOnly); //OVERLAPPINGWEAPON을 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
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

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalOffset = CameraBoom->SocketOffset;
}
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float TargetArmLength = IsAiming() ? AimCameraOffset : normalAimCameraOffset; // 앉으면 더 가까이
	float NewArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector TargetSocketOffset = bIsCrouched ?CrouchingCameraOffset : NormalOffset;
	FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = NewSocketOffset;
	AimOffset(DeltaTime); //조준 오프셋을 계산한다.
}

