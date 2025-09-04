// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime; //초당 힐량
	AmountToHeal += HealAmount; //남은 힐량
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
	// ...
}

