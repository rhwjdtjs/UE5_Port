// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Kismet/KismetMathLibrary.h" // �߰��� ��� ����

#include "GameFramework/CharacterMovementComponent.h" // �߰��� ��� ����

ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //ī�޶� �� ������Ʈ ����
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->bUsePawnControlRotation = true; //ī�޶� ���� ĳ������ ȸ���� ���󰡵��� ����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //��Ʈ������Ʈ�� ����
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //��Ʈ������Ʈ�� ����
	CombatComponent = CreateDefaultSubobject<UCBComponent>(TEXT("CombatComponent")); //���� ������Ʈ ����
	CombatComponent->SetIsReplicated(true); //���� �����ϰ� ����
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //ĳ���Ͱ� ũ���ġ�� �� �ֵ��� �����Ѵ�.
	// StandingCameraOffset = FVector(0.f, 30.f, 143.f);    //ī�޶��� ��� ��ġ�� �����Ѵ�. ĳ������ �Ӹ� ���� ī�޶� ��ġ�ϵ��� �Ѵ�.
	// CrouchingCameraOffset = FVector(0.f, 0.f, 90.f); // �� ���� ��ġ
}
void ATimeFractureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATimeFractureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATimeFractureCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATimeFractureCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATimeFractureCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ATimeFractureCharacter::EquipButton); //Ű������ EŰ�� ������ �� EquipButton �Լ��� ȣ���Ѵ�.
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATimeFractureCharacter::CrouchButton); //Ű������ CŰ�� ������ �� CrouchButton �Լ��� ȣ���Ѵ�.
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATimeFractureCharacter::AimButton); //��Ŭ��Ű�� ������ �� AimButton �Լ��� ȣ���Ѵ�.
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATimeFractureCharacter::AimButtonRelease); //��Ŭ��Ű�� ������ �� AimButtonRelease �Լ��� ȣ���Ѵ�.
	//������Ʈ ���ÿ� ����� Ű�� �̸��� ���ε��Ѵ�. this ->�� �Լ��� �ִ� �Լ��� �ҷ���
}


void ATimeFractureCharacter::MoveForward(float Value)//"��Ʈ�ѷ��� �ٶ󺸴� ������ �������� ĳ������ ���� ���͸� ���ϴ� ��"
{
	if (Controller != nullptr && Value != 0.f) //�÷��̾ ��Ʈ�ѷ��� ���� �ֳ� && �����̰��ֳ�
	{
		//��Ʈ�ѷ��� �������� ������.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //��Ʈ�ѷ��� ȸ���� ����Ѵ�.
		//YawRotation: ��Ʈ�ѷ��� Yaw ��(�¿� ȸ���� ���)�� ������ FRotator ����
		//��ġ(X�� ȸ��), ��(Z�� ȸ��)�� ������ �� ĳ���Ͱ� ����� �������θ� ȸ���ϰ� �Ϸ��� �ǵ�
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); //���溤�͸� �����ͼ� �ʱ�ȭ , 
		//RotationMatrix(YawRotation): �� �������͸� ������� ȸ�� ����� ����
		//�� ȸ�� ����� ĳ���Ͱ� �ٶ󺸴� ���� ���� ���� ���������� ��� ����
		// .GetUnitAxis(EAxis::X): ȸ�� ��Ŀ��� X��(���� ����) �� ���� ���͸� ������
		//��, ���� �ٶ󺸴� ���� �������� ������ ������� �˷��ִ� ����
		//�������ͷ� ȸ�� ��Ʈ������ ���� �� �ְ� ȸ�� ��Ʈ������ ������ �������, frotator���� frotationmatrix�� ����� �̸� �������̶� �θ��� fvector�� ��ȯ�Ѵ�.
		//����� ������ ������ ��Ÿ��
		AddMovementInput(Direction, Value); //����� ���� ���Ͽ� ĳ���Ͱ� �ش�������� �̵��ϰ� �Ѵ�. , �ӵ��� ���ӵ��� �ʿ��ϴ�.
	}
	
}
void ATimeFractureCharacter::MoveRight(float Value)
{
	//��Ʈ�ѷ��� �������� ������.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //��Ʈ�ѷ��� ȸ���� ����Ѵ�.
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //���溤�͸� �����ͼ� �ʱ�ȭ , 
	//�������ͷ� ȸ�� ��Ʈ������ ���� �� �ְ� ȸ�� ��Ʈ������ ������ �������, frotator���� frotationmatrix�� ����� �̸� �������̶� �θ��� fvector�� ��ȯ�Ѵ�.
	//����� ������ ������ ��Ÿ��
	AddMovementInput(Direction, Value); //����� ���� ���Ͽ� ĳ���Ͱ� �ش�������� �̵��ϰ� �Ѵ�. , �ӵ��� ���ӵ��� �ʿ��ϴ�.
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
		if (HasAuthority()) { //�������� ����Ǵ� ���
			CombatComponent->EquipWeapon(OverlappingWeapon);//��ġ�� ���⸦ �����Ѵ�.
		}
		else { //Ŭ���̾�Ʈ���� ����Ǵ� ���
			ServerEquipButton(); //�������� ���� ��ư�� ������.
		}
	}
}
void ATimeFractureCharacter::CrouchButton()
{
	if (bIsCrouched) {
		UnCrouch(); //ũ���ġ ���°� �ƴϸ� ũ���ġ�� �����Ѵ�.
	}
	Crouch(); //ũ���ġ ��ư�� ������ ũ���ġ�Ѵ�.
}
void ATimeFractureCharacter::AimButton()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(true); //���� ������Ʈ�� ���� ���θ� true�� �����Ѵ�.
	}
}
void ATimeFractureCharacter::AimButtonRelease()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(false); //���� ������Ʈ�� ���� ���θ� false�� �����Ѵ�.
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

	// �׻� Delta ���
	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, BaseAimRotation);
	AO_YAW = DeltaAimRotation.Yaw;
	AO_PITCH = GetBaseAimRotation().Pitch;

	// �̵� ���̸� BaseAimRotation ������Ʈ
	if (Speed > 0.f)
	{
		BaseAimRotation = CurrentAimRotation;
		bUseControllerRotationYaw = false; // �̵� �߿��� ��Ʈ�ѷ� ȸ���� ������� �ʵ��� ����
	}

	// �̵� �� ��Ʈ�ѷ� ȸ�� ���󰡰� ���� ���δ� ��Ÿ�Ͽ� ����
	bUseControllerRotationYaw = (Speed > 0.f);
}
void ATimeFractureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATimeFractureCharacter, OverlappingWeapon, COND_OwnerOnly); //OVERLAPPINGWEAPON�� �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
}
void ATimeFractureCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//ĳ������ �Ӽ��� �ʱ�ȭ�ϴ� �Լ�
	//�� �Լ��� ��� ������Ʈ�� �ʱ�ȭ�� �Ŀ� ȣ��ȴ�.
	//���� �� �Լ����� ĳ������ �Ӽ��� �ʱ�ȭ�ϸ�, ��� ������Ʈ�� �غ�� ���¿��� �Ӽ��� �ʱ�ȭ�ȴ�.
	//�� �Լ��� ������ Ŭ���̾�Ʈ ��ο��� ȣ��ȴ�.
	//���� �� �Լ����� ĳ������ �Ӽ��� �ʱ�ȭ�ϸ�, ������ Ŭ���̾�Ʈ ��ο��� ĳ������ �Ӽ��� �����ϰ� �ʱ�ȭ�ȴ�.
	//�� �Լ��� ��� ������Ʈ�� �ʱ�ȭ�� �Ŀ� ȣ��ȴ�.
	//���� �� �Լ����� ĳ������ �Ӽ��� �ʱ�ȭ�ϸ�, ��� ������Ʈ�� �غ�� ���¿��� �Ӽ��� �ʱ�ȭ�ȴ�.
	if (CombatComponent) {
		CombatComponent->Character = this; //ĳ���͸� �����Ѵ�.
	}
}
void ATimeFractureCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) //��ġ�� ���Ⱑ �����ϸ�
	{
		OverlappingWeapon->ShowPickupWidget(true); //��ġ�� ���� ������ ǥ���Ѵ�.
	}
	if (LastWeapon) //��ġ�� ���Ⱑ �����ϸ�
	{
		LastWeapon->ShowPickupWidget(false); //��ġ�� ���� ������ �����.
	}
}

void ATimeFractureCharacter::ServerEquipButton_Implementation()
{
	if (CombatComponent) {
		CombatComponent->EquipWeapon(OverlappingWeapon); //��ġ�� ���⸦ �����Ѵ�.
	}
}

void ATimeFractureCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false); //��ġ�� ���� ������ �����.
	}
	OverlappingWeapon = Weapon; //��ġ�� ���⸦ �����Ѵ�.
	if (IsLocallyControlled()) //���ÿ��� �����ϴ� ���
	{
		if (OverlappingWeapon) //��ġ�� ���Ⱑ �����ϸ�
		{
			OverlappingWeapon->ShowPickupWidget(true); //��ġ�� ���� ������ ǥ���Ѵ�.
		}
		
	}

}

bool ATimeFractureCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon); //���� ������Ʈ�� �����ϰ�, ���� ������Ʈ�� ���Ⱑ �����Ǿ� �ִ��� Ȯ���Ѵ�.
}

bool ATimeFractureCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bisAiming); //���� ������Ʈ�� �����ϰ�, ���� ������Ʈ�� ���� ���θ� ��ȯ�Ѵ�.
}

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalOffset = CameraBoom->SocketOffset;
}
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float TargetArmLength = IsAiming() ? AimCameraOffset : normalAimCameraOffset; // ������ �� ������
	float NewArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector TargetSocketOffset = bIsCrouched ?CrouchingCameraOffset : NormalOffset;
	FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = NewSocketOffset;
	AimOffset(DeltaTime); //���� �������� ����Ѵ�.
}

