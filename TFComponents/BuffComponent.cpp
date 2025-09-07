// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "GameFramework/CharacterMovementComponent.h" // �߰��� ��� ����
#include "UnrealProject_7A/tfcomponents/CBComponent.h"
// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}
void UBuffComponent::SetInitialJump(float JumpZVelocity)
{
	InitialJumpZVelocity = JumpZVelocity;
}
void UBuffComponent::ResetJump()
{
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity; //ĳ������ ���� �ӵ��� �ʱ�ȭ�Ѵ�.
	}
	MulticastJumpBuff(InitialJumpZVelocity); //��Ƽĳ��Ʈ�� ���� ������ �ʱ�ȭ�Ѵ�.
}
void UBuffComponent::BuffJump(float BuffJump, float JumpBuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, JumpBuffTime); //Ÿ�̸Ӹ� �����Ѵ�.
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJump; //ĳ������ ���� �ӵ��� �����Ѵ�.
	}
	MulticastJumpBuff(BuffJump); //��Ƽĳ��Ʈ�� ���� ������ �����Ѵ�.
}
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpZVelocity)
{
	if (Character->GetCharacterMovement())
		Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;//ĳ������ ���� �ӵ��� �����Ѵ�.
}
void UBuffComponent::BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float SpeedBuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, SpeedBuffTime); //Ÿ�̸Ӹ� �����Ѵ�.
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeedBuff; //ĳ������ �ȴ� �ӵ��� �����Ѵ�.
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeedBuff; //ĳ������ ��ũ�� �ӵ��� �����Ѵ�.
		UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
		if (CB) //ĳ���Ͱ� ���� �����̸�
		{
			UE_LOG(LogTemp, Warning, TEXT("UBuffComponent::BuffSpeed Aim true"));
			CB->SetAimWalkSpeed(BaseSpeedBuff-200.f);
			CB->SetBaseWalkSpeed(BaseSpeedBuff);
		}
	}
	MulticastSpeedBuff(BaseSpeedBuff, CrouchSpeedBuff); //��Ƽĳ��Ʈ�� ���ǵ� ������ �����Ѵ�.
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchBaseSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchBaseSpeed;
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed; //ĳ������ �ȴ� �ӵ��� �����Ѵ�.
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed; //ĳ������ ��ũ�� �ӵ��� �����Ѵ�.
	UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
	if (CB) //ĳ���Ͱ� ���� �����̸�
	{
		CB->SetAimWalkSpeed(140.f);
		CB->SetBaseWalkSpeed(InitialBaseSpeed);
	}
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed); //��Ƽĳ��Ʈ�� ���ǵ� ������ �����Ѵ�.
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed; //ĳ������ �ȴ� �ӵ��� �����Ѵ�.
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed; //ĳ������ ��ũ�� �ӵ��� �����Ѵ�.
	UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
	if (CB) //ĳ���Ͱ� ���� �����̸�
	{
		CB->SetAimWalkSpeed(BaseSpeed-200.f);
		CB->SetBaseWalkSpeed(BaseSpeed);
	}
}
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime; //�ʴ� ����
	AmountToHeal += HealAmount; //���� ����
}

void UBuffComponent::Shield(float ShieldAmount, float ShieldTime)
{
	bShielding = true;
	ShieldingRate = ShieldAmount / ShieldTime; //�ʴ� ����
	AmountToShield += ShieldAmount; //���� ����
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character==nullptr || Character->IsElimmed()) return;
	const float HealThisFrame = HealingRate * DeltaTime; //�̹� �����ӿ� ����
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth())); //ĳ������ ü���� ������Ű�� �ִ� ü���� ���� �ʵ��� Ŭ�����Ѵ�.
	Character->UpdateHUDHealth(); //HUD�� ü�� ǥ�ø� ������Ʈ�Ѵ�.
	AmountToHeal -= HealThisFrame; //���� ���� ����

	if(AmountToHeal <=0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShielding || Character == nullptr || Character->IsElimmed()) return;
	const float ShieldThisFrame = ShieldingRate * DeltaTime; //�̹� �����ӿ� ����
	Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, Character->GetMaxShield())); //ĳ������ ü���� ������Ű�� �ִ� ü���� ���� �ʵ��� Ŭ�����Ѵ�.
	Character->UpdateHUDShield(); //HUD�� ü�� ǥ�ø� ������Ʈ�Ѵ�.
	AmountToShield -= ShieldThisFrame; //���� ���� ����

	if (AmountToShield <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bShielding = false;
		AmountToShield = 0.f;
	}
}

// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}



// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
	// ...
}



