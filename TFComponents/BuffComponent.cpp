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
	HealingRate = HealAmount / HealingTime; //�ʴ� ����
	AmountToHeal += HealAmount; //���� ����
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

