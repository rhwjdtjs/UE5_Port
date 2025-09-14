// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Kismet/KismetMathLibrary.h" // �߰��� ��� ����
#include "GameFramework/CharacterMovementComponent.h" // �߰��� ��� ����
#include "TFAniminstance.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
#include "UnrealProject_7A/GameMode/TFGameMode.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h" // ĸ�� ������Ʈ ��� ���� �߰�
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
ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
    // Replace the line causing the error with the following:
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //ī�޶� �� ������Ʈ ����
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->bUsePawnControlRotation = false; //ī�޶� ���� ĳ������ ȸ���� ���󰡵��� ����
	bUseControllerRotationYaw = false; // ī�޶� ������ ����
	GetCharacterMovement()->bOrientRotationToMovement =false; // �̵� �������� ȸ������ ����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //��Ʈ������Ʈ�� ����
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //��Ʈ������Ʈ�� ����
	CombatComponent = CreateDefaultSubobject<UCBComponent>(TEXT("CombatComponent")); //���� ������Ʈ ����
	CombatComponent->SetIsReplicated(true); //���� �����ϰ� ����
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent")); //���� ������Ʈ ����
	BuffComponent->SetIsReplicated(true); //���� �����ϰ� ����
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //ĳ���Ͱ� ũ���ġ�� �� �ֵ��� �����Ѵ�.
	GetMesh()->SetCollisionObjectType(ECC_SkelatalMesh); //�޽��� �浹 ��ü ������ ���̷�Ż �޽��� �����Ѵ�.
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //�޽��� �þ� ä�ο� �������� �ʵ��� �����Ѵ�.
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); //�޽��� �þ� ä�ο� �������� �ʵ��� �����Ѵ�.

	AttachedGrenade = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket")); //����ź �޽��� ĳ������ �޽��� �����Ѵ�.
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision); //����ź �޽��� �浹�� ��Ȱ��ȭ�Ѵ�.
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
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATimeFractureCharacter::FireButtonPressed); //��Ŭ��Ű�� ������ �� AimButton �Լ��� ȣ���Ѵ�.
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATimeFractureCharacter::FireButtonReleased); //��Ŭ��Ű�� ������ �� AimButtonRelease �Լ��� ȣ���Ѵ�.
		PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATimeFractureCharacter::ReloadButtonPressed); //Ű������ CŰ�� ������ �� CrouchButton �Լ��� ȣ���Ѵ�.
		PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ATimeFractureCharacter::GrenadeButtonPressed); //GŰ�� ������ �� ����ź ��ô �ִϸ��̼� ���
		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATimeFractureCharacter::Jump); //�����̽��ٸ� ������ �� ����
    PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &ATimeFractureCharacter::SwapButtonPressed);
	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &ATimeFractureCharacter::HandleChatKey);
	PlayerInputComponent->BindAction("ChatCancel", IE_Pressed, this, &ATimeFractureCharacter::HandleChatCancel);
	PlayerInputComponent->BindAction("ChatSubmit", IE_Pressed, this, &ATimeFractureCharacter::HandleChatSubmit);
	PlayerInputComponent->BindAction("Dive", IE_Pressed, this, &ATimeFractureCharacter::Dive);
	//������Ʈ ���ÿ� ����� Ű�� �̸��� ���ε��Ѵ�. this ->�� �Լ��� �ִ� �Լ��� �ҷ���
}

void ATimeFractureCharacter::HandleChatKey()//0913 ä��
{
	ATFPlayerController* PC = Cast<ATFPlayerController>(Controller);
	if (PC)
	{
		ATFHUD* HUD = Cast<ATFHUD>(PC->GetHUD());
		if (HUD && HUD->ChatWidgetClass)
		{
			if (!HUD->ChatWidget) // ���� ���� �� �ƴٸ�
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

void ATimeFractureCharacter::HandleChatSubmit()//0913 ä��
{
	ATFPlayerController* PC = Cast<ATFPlayerController>(Controller);
	if (PC)
	{
		ATFHUD* HUD = Cast<ATFHUD>(PC->GetHUD());
		if (HUD && HUD->ChatWidget && HUD->ChatWidget->IsChatOpen())
		{
			HUD->ChatWidget->SubmitAndClose();   // Enter �� ���� �� �ݱ�
		}
	}
}
void ATimeFractureCharacter::HandleChatCancel() // y �� ���0913 ä��
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
void ATimeFractureCharacter::Dive() //0914 ������
{
	if (CombatComponent->EquippedWeapon == nullptr) return;
	if (bIsDodging) return;

	// ���ÿ����� ������ ��û
	if (!HasAuthority())
	{
		ServerDivePressed();
		return;
	}

	// ������ �ٷ� ����
	MulticastDive();
}
void ATimeFractureCharacter::ServerDivePressed_Implementation() //0914 ������
{
	MulticastDive(); // ������ ��� Ŭ�� ����
}
void ATimeFractureCharacter::MulticastDive_Implementation() //0914 ������
{
	if (bIsDodging) return;

	UTFAniminstance* TFAnim = Cast<UTFAniminstance>(GetMesh()->GetAnimInstance());
	if (!TFAnim || TFAnim->bIsInAir || TFAnim->bIsCrouching) return;

	if (DiveMontage)
	{
		bDisableGameplay = true;
		bIsDodging = true;

		TFAnim->Montage_Play(DiveMontage);
		TFAnim->Montage_JumpToSection(FName("Dive"));

		// �ִ� ������ �ʱ�ȭ
		FOnMontageEnded EndDelegate;
		EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) {
			bDisableGameplay = false;
			bIsDodging = false;
			});
		TFAnim->Montage_SetEndDelegate(EndDelegate, DiveMontage);
	}
}
void ATimeFractureCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f && CombatComponent && CombatComponent->EquippedWeapon) //�÷��̾ ��Ʈ�ѷ��� ���� �ֳ� && �����̰��ֳ�
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
	else if(Controller != nullptr && FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		float TargetYaw = Controller->GetControlRotation().Yaw;
		if (Value < 0.f)
			TargetYaw += 180.f;

		FRotator TargetRotation(0.f, TargetYaw, 0.f);

		// ���� ȸ������ ��ǥ ȸ������ �ε巴�� ����
		float InterpSpeed = 10.f; // ���� Ŭ���� �� ������ ȸ��
		MoveRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		if (HasAuthority())
		{
			SetActorRotation(MoveRotation);
		}
		else
		{
			SetActorRotation(MoveRotation); // Ŭ�󿡼��� ��� ȸ�� ����
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
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //��Ʈ�ѷ��� ȸ���� ����Ѵ�.
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //���溤�͸� �����ͼ� �ʱ�ȭ , 
		//�������ͷ� ȸ�� ��Ʈ������ ���� �� �ְ� ȸ�� ��Ʈ������ ������ �������, frotator���� frotationmatrix�� ����� �̸� �������̶� �θ��� fvector�� ��ȯ�Ѵ�.
		//����� ������ ������ ��Ÿ��
		AddMovementInput(Direction, Value); //����� ���� ���Ͽ� ĳ���Ͱ� �ش�������� �̵��ϰ� �Ѵ�. , �ӵ��� ���ӵ��� �ʿ��ϴ�.
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
			SetActorRotation(MoveRotation); // Ŭ�󿡼��� ��� ȸ�� ����
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
	if (bDisableGameplay) return;
	if (CombatComponent) {
		if (HasAuthority()) { // ����
			if (OverlappingWeapon) {
				CombatComponent->EquipWeapon(OverlappingWeapon);
			}
		//	else if (CombatComponent->ShouldSwapWeapons()) {
		//		CombatComponent->SwapWeapons();
	//		}
		}
		else { // Ŭ���̾�Ʈ
			ServerEquipButton();
		}
	}
}
void ATimeFractureCharacter::CrouchButton()
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (bIsCrouched) {
		UnCrouch(); //ũ���ġ ���°� �ƴϸ� ũ���ġ�� �����Ѵ�.
	}
	Crouch(); //ũ���ġ ��ư�� ������ ũ���ġ�Ѵ�.
}
void ATimeFractureCharacter::AimButton()
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent) {
		CombatComponent->SetAiming(true); //���� ������Ʈ�� ���� ���θ� true�� �����Ѵ�.
	}
}
void ATimeFractureCharacter::AimButtonRelease()
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent) {
		CombatComponent->SetAiming(false); //���� ������Ʈ�� ���� ���θ� false�� �����Ѵ�.
	}
}
void ATimeFractureCharacter::AimOffset(float DeltaTime)
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
		FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bUseControllerRotationYaw = true;
	if (Speed > 0.f) // �����϶��� �¿� yaw����
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
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(true); //���� ������Ʈ�� �߻� ��ư�� �����ٰ� �����Ѵ�.
	}
}
void ATimeFractureCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(false); //���� ������Ʈ�� �߻� ��ư�� �����ٰ� �����Ѵ�.
	}
}
void ATimeFractureCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return; //�����÷��̰� ��Ȱ��ȭ�� ��� �̵����� ����
	if (CombatComponent) {
		CombatComponent->Reload(); //���� ������Ʈ�� �������� ȣ���Ѵ�.
	}
}
void ATimeFractureCharacter::GrenadeButtonPressed()
{
	if (CombatComponent) {
		CombatComponent->ThrowGrenade(); //���� ������Ʈ�� ����ź ��ô�� ȣ���Ѵ�.
	}
}
void ATimeFractureCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCursor)
{
	if (bisElimmed) return;
	float DamageToHealth = Damage;
	if (Shield > 0.f) {
		if (Shield >= Damage) {
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield); //�ǵ带 ���ҽ�Ű��, �ּҰ��� 0, �ִ밪�� �ִ� �ǵ�� �����Ѵ�.
			DamageToHealth = 0.f; //�ǵ尡 ���������� ü�¿� �������� ���� �ʴ´�.
		}
		else {
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);//�ǵ尡 �����ϸ� ���� �������� ü�¿� �ش�.
			Shield = 0.f; //�ǵ带 0���� �����.
		}
	}
	if (ATFPlayerController* AttackerController = Cast<ATFPlayerController>(InstigatorController))
	{
		AttackerController->ClientPlayHitConfirmSound(HitConfirmSound);
	}
	if (IsLocallyControlled())
	{
		BloodScreen(); // ���� ȭ�鿡 ���� ��ũ�� ǥ��
	}
	else {
		ClientShowBloodScreen();
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth); //ü���� ���ҽ�Ű��, �ּҰ��� 0, �ִ밪�� �ִ� ü������ �����Ѵ�.
	UGameplayStatics::PlaySoundAtLocation(this, HitCharacterSound, GetActorLocation()); //���� ���带 ����Ѵ�.
	UpdateHUDHealth(); //HUD�� ü���� ������Ʈ�Ѵ�.
	UpdateHUDShield(); //HUD�� �ǵ带 ������Ʈ�Ѵ�.
	PlayHitReactMontage(); //�ǰ� �ִϸ��̼��� ����Ѵ�.
	if (Health == 0.f)	//ü���� 0�� �Ǹ�{
	{
		ATFGameMode* TFGameMode = GetWorld()->GetAuthGameMode<ATFGameMode>(); //���� ���� ��带 ATFGameMode�� ĳ�����Ѵ�.
		if (TFGameMode)
		{
            // �Ʒ� �ڵ�� TfPlayerController�� nullptr(���� �Ҵ���� ����)�̸� Controller�� ATFPlayerController�� ĳ�����Ͽ� �Ҵ��ϰ�,
            // �̹� �Ҵ�Ǿ� �ִٸ� ���� ���� �״�� ����Ѵ�.
            // ��, TfPlayerController�� ������� ���� ���� ĳ�����Ͽ� �Ҵ��ϴ� ������ �Ѵ�.
            // �Ʒ� �ڵ�� TfPlayerController�� nullptr(���� �Ҵ���� ����)�̸� Controller�� ATFPlayerController�� ĳ�����Ͽ� �Ҵ��ϰ�,
            // �̹� �Ҵ�Ǿ� �ִٸ� ���� ���� �״�� ����Ѵ�.
            // ��, TfPlayerController�� ������� ���� ���� ĳ�����Ͽ� �Ҵ��ϴ� ������ �Ѵ�.
            // ����:
            // 1. ���ʿ��� ĳ������ �ݺ����� �ʾ� ������ ����ȭ�ȴ�.
            // 2. �ڵ尡 ����������.
            // 3. �̹� �ùٸ��� �Ҵ�� �����͸� �����ϹǷ� �������� ��������.
            TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController; //�÷��̾� ��Ʈ�ѷ��� �����´�.
			ATFPlayerController* AttackerController = Cast<ATFPlayerController>(InstigatorController); //�������� ��Ʈ�ѷ��� �����´�.
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
				// ���������� ���� �ʱ�ȭ (������)
				TfPlayerState->AddToScore(0.f);
				TfPlayerState->AddToDefeats(0); //óġ �� �ʱ�ȭ
			}
			else
			{
				// Ŭ���̾�Ʈ������ ���� HUD ������Ʈ
				TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;
				if (TfPlayerController)
				{
					TfPlayerController->SetHUDScore(TfPlayerState->GetScore());
					TfPlayerState->AddToDefeats(0); //óġ �� �ʱ�ȭ
				}
			}
		}
	}
}
void ATimeFractureCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState(); //�θ� Ŭ������ OnRep_PlayerState �Լ��� ȣ���Ѵ�.
	EnsureOverheadWidgetLocal();
	RefreshOverheadName();
	PollInit();
}
void ATimeFractureCharacter::ServerSetActorRotation_Implementation(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
}
void ATimeFractureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);//�θ� Ŭ������ ���� �Ӽ��� �����´�.
	DOREPLIFETIME_CONDITION(ATimeFractureCharacter, OverlappingWeapon, COND_OwnerOnly); //OVERLAPPINGWEAPON�� �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
	DOREPLIFETIME(ATimeFractureCharacter, Health); //Health�� �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
	DOREPLIFETIME(ATimeFractureCharacter, Shield); //Shield�� �����Ѵ�.
	DOREPLIFETIME(ATimeFractureCharacter, bDisableGameplay);//bDisableGameplay�� �����Ѵ�.
	DOREPLIFETIME(ATimeFractureCharacter, MoveRotation);//bisElimmed�� �����Ѵ�.
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
	if (BuffComponent) {
		BuffComponent->Character = this; //ĳ���͸� �����Ѵ�.	
		BuffComponent->SetInitialSpeeds(CombatComponent->baseWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched); //ĳ������ �ʱ� �ӵ��� �����Ѵ�.
		BuffComponent->SetInitialJump(GetCharacterMovement()->JumpZVelocity); //ĳ������ �ʱ� ���� �ӵ��� �����Ѵ�.
	}
}
void ATimeFractureCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon) {
		if (CombatComponent->EquippedWeapon) {
			if (CombatComponent->EquippedWeapon->bDestroyWeapon) {
				CombatComponent->EquippedWeapon->Destroy(); //������ ���⸦ �ı��Ѵ�.
			}
			else {
				CombatComponent->EquippedWeapon->DropWeapon(); //������ ���⸦ ����߸���.
			}
		}
		if(CombatComponent->SecondaryWeapon)
		{
			if (CombatComponent->SecondaryWeapon->bDestroyWeapon) {
				CombatComponent->SecondaryWeapon->Destroy(); //���� ���⸦ �ı��Ѵ�.
			}
			else {
				CombatComponent->SecondaryWeapon->DropWeapon(); //���� ���⸦ ����߸���.
			}
		}
	}
	MulticastElim(); //�������� Ŭ���̾�Ʈ�� ���Ÿ� �˸���.
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ATimeFractureCharacter::ElimTimerFinished, ElimDelay);
}
void ATimeFractureCharacter::MulticastElim_Implementation()
{
	if (TfPlayerController) {
		TfPlayerController->SetHUDWeaponAmmo(0); //HUD�� ���� ź���� 0���� �����Ѵ�.
	}
	bisElimmed = true; //ĳ���Ͱ� ���ŵǾ����� ǥ���Ѵ�.
	PlayElimMontage(); //�ǰ� �ִϸ��̼��� ����Ѵ�.
	//ĳ���� �̵� ��Ȱ��ȭ
	GetCharacterMovement()->DisableMovement(); //ĳ������ �̵��� ��Ȱ��ȭ�Ѵ�.
	GetCharacterMovement()->StopMovementImmediately(); //ĳ������ �̵��� ��� �����Ѵ�.
	bDisableGameplay = true; //�����÷��̸� ��Ȱ��ȭ�Ѵ�.
	if (CombatComponent) {
		CombatComponent->FireButtonPressed(false); //���� ������Ʈ�� �߻� ��ư�� �����ٰ� �����Ѵ�.
	}
	//�ݸ��� ��Ȱ��ȭ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ĳ������ ĸ�� �ݸ����� ��Ȱ��ȭ�Ѵ�.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ĳ������ �޽� �ݸ����� ��Ȱ��ȭ�Ѵ�.
	bool bHideSniperScope = IsLocallyControlled() && CombatComponent && CombatComponent->bisAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope) {
		ShowSniperScopeWidget(false); //���ÿ��� �����ϴ� ��� �������� ������ ������ �����.
	}
}

void ATimeFractureCharacter::ElimTimerFinished()
{
	ATFGameMode* TFGameMode = GetWorld()->GetAuthGameMode<ATFGameMode>(); //���� ���� ��带 ATFGameMode�� ĳ�����Ѵ�.
	if (TFGameMode) {
		TFGameMode->RequestRespawn(this, Controller); //���� ��忡 ���ŵ� ĳ������ ������� ��û�Ѵ�.
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
		if (OverlappingWeapon) {
			CombatComponent->EquipWeapon(OverlappingWeapon); //��ġ�� ���⸦ �����Ѵ�.
		}
	//	else if(CombatComponent->ShouldSwapWeapons())
	//	{
	//		CombatComponent->SwapWeapons(); //���⸦ ��ü�Ѵ�.
	//	}
	}
}

void ATimeFractureCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return; //���ÿ��� �������� �ʴ� ��� �Լ��� �����Ѵ�.
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold) {
		GetMesh()->SetVisibility(false); //ī�޶� ĳ���Ϳ� ������ ���� �� �޽��� �����.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true; //������ ������ �޽��� �����.
		}
	}
	else {
		GetMesh()->SetVisibility(true); //ī�޶� ĳ���Ϳ� ������ ���� �� �޽��� �����.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false; //������ ������ �޽��� �����.
		}
	}
}
void ATimeFractureCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield(); //HUD�� ü���� ������Ʈ�Ѵ�.
	if (Shield < LastShield) {
		PlayHitReactMontage(); //�ǰ� �ִϸ��̼��� ����Ѵ�.
	}
}
void ATimeFractureCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth(); //HUD�� ü���� ������Ʈ�Ѵ�.
	if (Health < LastHealth) {
		PlayHitReactMontage(); //�ǰ� �ִϸ��̼��� ����Ѵ�.
	}
}
void ATimeFractureCharacter::EnsureOverheadWidgetLocal()
{
	if (!OverheadWidget) return;

	// ���ü�/���� ����
	OverheadWidget->SetVisibility(true, true);
	OverheadWidget->SetHiddenInGame(false);
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidget->SetDrawAtDesiredSize(true);
	OverheadWidget->SetOwnerNoSee(false);
	OverheadWidget->SetOnlyOwnerSee(false);

	// �̹� �����Ǿ� ������ �н�
	if (OverheadWidget->GetUserWidgetObject()) return;

	TSubclassOf<UUserWidget> WC = OverheadWidget->GetWidgetClass();
	if (!WC) return;

	// �� OwningPlayer �켱 �õ�(������ ������ ���� PC)
	APlayerController* LocalPC = nullptr;
	if (GEngine)
		LocalPC = GEngine->GetFirstLocalPlayerController(GetWorld());

	UUserWidget* NewUW = nullptr;
	if (LocalPC)
	{
		NewUW = CreateWidget<UUserWidget>(LocalPC, WC); // OwningPlayer ����
	}
	if (!NewUW)
	{
		NewUW = CreateWidget<UUserWidget>(GetWorld(), WC); // WorldContext ����
	}
	if (!NewUW) return;

	OverheadWidget->SetWidget(NewUW);
	OverheadWidget->InitWidget();
	NewUW->SetVisibility(ESlateVisibility::HitTestInvisible);
	UE_LOG(LogTemp, Warning, TEXT("NOT CLIENT"));
}
void ATimeFractureCharacter::RefreshOverheadName()
{
	 if (!OverheadWidget) return;

    // ���� ����Ʈ�� �ν��Ͻ� ����
    EnsureOverheadWidgetLocal();

    UUserWidget* UW = OverheadWidget->GetUserWidgetObject();
    if (!UW) return;

    const FString Name = (GetPlayerState() ? GetPlayerState()->GetPlayerName() : FString());

    // 1) C++ UOverheadWidget�̸� ���� �Լ� ���
    if (UOverheadWidget* OW = Cast<UOverheadWidget>(UW))
    {
        OW->SetDisplayText(Name);          // �Ǵ� OW->ShowPlayerNetRole(this);
        OW->SetVisibility(ESlateVisibility::HitTestInvisible);
        return;
    }

    // 2) BP UserWidget�̾ 'DisplayText'�� ã�Ƽ� ���� ���� (����)
    if (UTextBlock* TB = Cast<UTextBlock>(UW->GetWidgetFromName(TEXT("DisplayText"))))
    {
        TB->SetText(FText::FromString(Name));
        UW->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
	UE_LOG(LogTemp, Warning, TEXT("NOT CLIENT"));
}
void ATimeFractureCharacter::SpawnDefaultWeapon()
{
	ATFGameMode* TFGameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); //���� ���� ��带 ATFGameMode�� ĳ�����Ѵ�.
	UWorld* World = GetWorld();
	if (TFGameMode && World && !bisElimmed && DefaultWeaponClass) {
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true; //�⺻ ����� ���ŵ� �� �ı��ȴ�.
		if (CombatComponent) {
			CombatComponent->EquipWeapon(StartingWeapon); //���� ������Ʈ�� ���⸦ �����Ѵ�.
			
		}

	}
}
void ATimeFractureCharacter::UpdateHUDAmmo()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//�÷��̾� ��Ʈ�ѷ��� ĳ�����Ͽ� �����Ѵ�.
	if (TfPlayerController && CombatComponent && CombatComponent->EquippedWeapon) {
		TfPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo); //HUD�� ź���� ������Ʈ�Ѵ�.
		TfPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo()); //HUD�� ���� ź���� ������Ʈ�Ѵ�.
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

UOverheadWidget* ATimeFractureCharacter::GetOverheadWidget() const
{
	if (OverheadWidget)
		return Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject());
	return nullptr;
}

ECombatState ATimeFractureCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX; //���� ������Ʈ�� �������� ������ �ִ� ���� ���¸� ��ȯ�Ѵ�.
	return CombatComponent->CombatState; //���� ������Ʈ�� ���� ���¸� ��ȯ�Ѵ�.
}

AWeapon* ATimeFractureCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr)
		return nullptr;

	return CombatComponent->EquippedWeapon; //���� ������Ʈ�� ���⸦ ��ȯ�Ѵ�.
}

void ATimeFractureCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if (animInstance && FireWeaponMontage) {
		animInstance->Montage_Play(FireWeaponMontage); //�ִϸ��̼� ��Ÿ�ָ� ����Ѵ�.
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip"); //���� ���¿� ���� ���� �̸��� �����Ѵ�.
		animInstance->Montage_JumpToSection(SectionName); //�ִϸ��̼� ��Ÿ���� �������� �����Ѵ�.
	}
}

void ATimeFractureCharacter::PlayElimMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if (animInstance && ElimMontage) {
		animInstance->Montage_Play(ElimMontage); //�ִϸ��̼� ��Ÿ�ָ� ����Ѵ�.
	}
}

void ATimeFractureCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || CombatComponent->CombatState == ECombatState::ECS_Reloading || 
		CombatComponent->CombatState==ECombatState::ECS_ThrowingGrenade) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if (animInstance && HitReactMontage) {
		animInstance->Montage_Play(HitReactMontage); //�ִϸ��̼� ��Ÿ�ָ� ����Ѵ�.
		FName SectionName("FromFront");
		animInstance->Montage_JumpToSection(SectionName); //�ִϸ��̼� ��Ÿ���� �������� �����Ѵ�.
	}
}

void ATimeFractureCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if (animInstance && ReloadMontage) {
		animInstance->Montage_Play(ReloadMontage); //�ִϸ��̼� ��Ÿ�ָ� ����Ѵ�.
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
		animInstance->Montage_JumpToSection(SectionName); //�ִϸ��̼� ��Ÿ���� �������� �����Ѵ�.
	}
}

void ATimeFractureCharacter::MultiCastHit_Implementation()
{
	PlayHitReactMontage(); //��Ʈ ����Ʈ ��Ÿ�ָ� ����Ѵ�.
}

FVector ATimeFractureCharacter::GetHitTarget() const
{
	if(CombatComponent==nullptr) 	return FVector();
	return CombatComponent->HitTarget; //���� ������Ʈ�� ��Ʈ Ÿ���� ��ȯ�Ѵ�.

}


void ATimeFractureCharacter::PlayThrowGrendadeMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if(animInstance && ThrowGrenadeMontage) {
		animInstance->Montage_Play(ThrowGrenadeMontage); //�ִϸ��̼� ��Ÿ�ָ� ����Ѵ�.
	}
}

void ATimeFractureCharacter::ClientShowBloodScreen_Implementation()
{
	BloodScreen(); // ���� Ŭ�󿡼��� ����
}

void ATimeFractureCharacter::Jump()
{
	if (bDisableGameplay) return;
	Super::Jump();
}

void ATimeFractureCharacter::Destroyed()
{
	Super::Destroyed();
	ATFGameMode* TFGameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); //���� ���� ��带 ATFGameMode�� ĳ�����Ѵ�.
	bool bMatchNotInProgress = TFGameMode && TFGameMode->GetMatchState() !=MatchState::InProgress; //���� ��尡 �����ϰ�, ������ ���� ������ ���� ���
	if(CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress) {
		CombatComponent->EquippedWeapon->Destroy(); //������ ���⸦ �ı��Ѵ�.
	}
}

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalOffset = CameraBoom->SocketOffset;
	// �ٷ� �õ�
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
					// �ڡڡ� �ٽ�: ���������� ���� �÷��̾ OwnerPlayer�� ����
					OverheadWidget->SetOwnerPlayer(LP);
				}
			}
		}
	}
	EnsureOverheadWidgetLocal();
	RefreshOverheadName();
	
	SpawnDefaultWeapon(); //�⺻ ���⸦ �����Ѵ�.
	UpdateHUDAmmo(); //HUD�� ź���� ������Ʈ�Ѵ�.
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority()) //�������� ����Ǵ� ���
	{
		bReplicates = true;
		SetReplicateMovement(true); //ĳ������ �̵��� ��Ʈ��ũ�� ���� �����ǵ��� ����
		OnTakeAnyDamage.AddDynamic(this, &ATimeFractureCharacter::ReceiveDamage); //���ظ� �޾��� �� ȣ��Ǵ� �Լ��� ���ε��Ѵ�.
	}
	if (AttachedGrenade) {
		AttachedGrenade->SetVisibility(false); //����ź �޽��� �����.
	}
}
void ATimeFractureCharacter::SwapButtonPressed()
{
	if(CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		if (HasAuthority()) { // ����
			CombatComponent->SwapWeapons();
		}
		else { // Ŭ���̾�Ʈ
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
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//�÷��̾� ��Ʈ�ѷ��� ĳ�����Ͽ� �����Ѵ�.
	if (TfPlayerController) {
		TfPlayerController->SetHUDHealth(Health, MaxHealth); //�÷��̾� ��Ʈ�ѷ��� HUD�� ü���� �����Ѵ�.
	}
}
void ATimeFractureCharacter::UpdateHUDShield()
{
	TfPlayerController = TfPlayerController == nullptr ? Cast<ATFPlayerController>(Controller) : TfPlayerController;//�÷��̾� ��Ʈ�ѷ��� ĳ�����Ͽ� �����Ѵ�.
	if (TfPlayerController) {
		TfPlayerController->SetHUDShield(Shield, MaxShield); //�÷��̾� ��Ʈ�ѷ��� HUD�� ü���� �����Ѵ�.
	}
}
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
		AddMovementInput(ForwardDir, 1.0f);
	}
	float TargetArmLength = IsAiming() ? AimCameraOffset : normalAimCameraOffset; // ������ �� ������
	float NewArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector TargetSocketOffset = bIsCrouched ? CrouchingCameraOffset : NormalOffset;
	FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = NewSocketOffset;
	AimOffset(DeltaTime); //���� �������� ����Ѵ�.
	HideCameraIfCharacterClose(); //ĳ���Ͱ� ������ ���� �� ī�޶� �����.
	UpdateHUDAmmo();
}

