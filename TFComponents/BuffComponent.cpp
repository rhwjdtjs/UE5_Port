// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "GameFramework/CharacterMovementComponent.h" // 추가된 헤더 파일
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
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity; //캐릭터의 점프 속도를 초기화한다.
	}
	MulticastJumpBuff(InitialJumpZVelocity); //멀티캐스트로 점프 버프를 초기화한다.
}
void UBuffComponent::BuffJump(float BuffJump, float JumpBuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, JumpBuffTime); //타이머를 설정한다.
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJump; //캐릭터의 점프 속도를 설정한다.
	}
	MulticastJumpBuff(BuffJump); //멀티캐스트로 점프 버프를 적용한다.
}
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpZVelocity)
{
	if (Character->GetCharacterMovement())
		Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;//캐릭터의 점프 속도를 설정한다.
}
void UBuffComponent::BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float SpeedBuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, SpeedBuffTime); //타이머를 설정한다.
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeedBuff; //캐릭터의 걷는 속도를 설정한다.
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeedBuff; //캐릭터의 웅크린 속도를 설정한다.
		UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
		if (CB) //캐릭터가 조준 상태이면
		{
			UE_LOG(LogTemp, Warning, TEXT("UBuffComponent::BuffSpeed Aim true"));
			CB->SetAimWalkSpeed(BaseSpeedBuff-200.f);
			CB->SetBaseWalkSpeed(BaseSpeedBuff);
		}
	}
	MulticastSpeedBuff(BaseSpeedBuff, CrouchSpeedBuff); //멀티캐스트로 스피드 버프를 적용한다.
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchBaseSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchBaseSpeed;
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed; //캐릭터의 걷는 속도를 설정한다.
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed; //캐릭터의 웅크린 속도를 설정한다.
	UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
	if (CB) //캐릭터가 조준 상태이면
	{
		CB->SetAimWalkSpeed(140.f);
		CB->SetBaseWalkSpeed(InitialBaseSpeed);
	}
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed); //멀티캐스트로 스피드 버프를 적용한다.
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed; //캐릭터의 걷는 속도를 설정한다.
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed; //캐릭터의 웅크린 속도를 설정한다.
	UCBComponent* CB = Cast<UCBComponent>(Character->GetCombatComponent());
	if (CB) //캐릭터가 조준 상태이면
	{
		CB->SetAimWalkSpeed(BaseSpeed-200.f);
		CB->SetBaseWalkSpeed(BaseSpeed);
	}
}
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime; //초당 힐량
	AmountToHeal += HealAmount; //남은 힐량
}

void UBuffComponent::Shield(float ShieldAmount, float ShieldTime)
{
	bShielding = true;
	ShieldingRate = ShieldAmount / ShieldTime; //초당 힐량
	AmountToShield += ShieldAmount; //남은 힐량
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character==nullptr || Character->IsElimmed()) return;
	const float HealThisFrame = HealingRate * DeltaTime; //이번 프레임에 힐량
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth())); //캐릭터의 체력을 증가시키고 최대 체력을 넘지 않도록 클램프한다.
	Character->UpdateHUDHealth(); //HUD의 체력 표시를 업데이트한다.
	AmountToHeal -= HealThisFrame; //남은 힐량 감소

	if(AmountToHeal <=0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShielding || Character == nullptr || Character->IsElimmed()) return;
	const float ShieldThisFrame = ShieldingRate * DeltaTime; //이번 프레임에 힐량
	Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, Character->GetMaxShield())); //캐릭터의 체력을 증가시키고 최대 체력을 넘지 않도록 클램프한다.
	Character->UpdateHUDShield(); //HUD의 체력 표시를 업데이트한다.
	AmountToShield -= ShieldThisFrame; //남은 힐량 감소

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



