// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"

ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), FName("Head")); //소켓네임을 이용해서 followcamera를 cameraarm에 부착
	FollowCamera->bUsePawnControlRotation = true; //카메라회전하면 몸도회전

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //루트컴포넌트에 부착

	CombatComponent = CreateDefaultSubobject<UCBComponent>(TEXT("CombatComponent")); //전투 컴포넌트 생성
	CombatComponent->SetIsReplicated(true); //복제 가능하게 설정
}
void ATimeFractureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATimeFractureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATimeFractureCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATimeFractureCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATimeFractureCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ATimeFractureCharacter::EquipButton); //키보드의 E키를 눌렀을 때 EquipButton 함수를 호출한다.
	//프로젝트 세팅에 저장된 키의 이름을 바인드한다. this ->이 함수의 있는 함수를 불러옴
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
	if (CombatComponent && HasAuthority()) {
		CombatComponent->EquipWeapon(OverlappingWeapon); //겹치는 무기를 장착한다.
	}
}

void ATimeFractureCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) //겹치는 무기가 존재하면
	{
		OverlappingWeapon->ShowPickupWidget(false); //겹치는 무기 위젯을 표시한다.
	}
	if (LastWeapon) //겹치는 무기가 존재하면
	{
		LastWeapon->ShowPickupWidget(false); //겹치는 무기 위젯을 숨긴다.
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

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
}
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

