// Fill out your copyright notice in the Description page of Project Settings.


#include "WireComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "CombatStates.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Character/TFAniminstance.h"
// Sets default values for this component's properties
UWireComponent::UWireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}
void UWireComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ATimeFractureCharacter>(GetOwner());

}
void UWireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWireComponent, bIsAttached);
	DOREPLIFETIME(UWireComponent, WireTarget);
}
void UWireComponent::OnRep_WireState()
{
	/*
	if (!Character) Character = Cast<ATimeFractureCharacter>(GetOwner());
	if (!Character) return;

	//  �����: ���� Ŭ������/��������, ���� ���� Ȯ��
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_WireState] %s  bIsAttached=%s"),
		Character->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		bIsAttached ? TEXT("True") : TEXT("False"));

	// (����) ���⼭ AnimInstance�� �ٷ� �ݿ��ص� ������,
	// AnimInstance�� �� ������ WireComponent�� �����Ƿ� ���� ����.
	// ���ܵΰ� �ʹٸ� �����ϰ�:
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		if (UTFAniminstance* Anim = Cast<UTFAniminstance>(Mesh->GetAnimInstance()))
			Anim->bIsWireAttached = bIsAttached;

	// �̵����� ������ (Ŭ�󿡼� �ٲٸ� ��鸲 ����)
	if (Character->HasAuthority())
		if (auto* Move = Character->GetCharacterMovement())
			Move->SetMovementMode(bIsAttached ? MOVE_Flying : MOVE_Falling);
	*/
	if (!Character) Character = Cast<ATimeFractureCharacter>(GetOwner());
	if (!Character) return;

	// �����: ��� �ҷȴ��� �α�
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_WireState] %s bIsAttached=%s"),
		Character->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		bIsAttached ? TEXT("True") : TEXT("False"));

	// Ŭ�� ����: AnimInstance�� bool �ݿ�
	if (!Character->HasAuthority()) //  Ŭ�󿡼��� ����
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UTFAniminstance* Anim = Cast<UTFAniminstance>(Mesh->GetAnimInstance()))
			{
				Anim->bIsWireAttached = bIsAttached;
			}
		}
	}
}
void UWireComponent::ServerReleaseWire_Implementation()
{
	bIsAttached = false;

	if (Character && Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling); //�ȱ���� ����
	}
}
void UWireComponent::ServerFireWire_Implementation()
{
	if (!Character) return;
	if (Character->IsElimmed() || Character->bIsDodging ||
		Character->GetCombatState() == ECombatState::ECS_Reloading ||
		Character->GetCombatState() == ECombatState::ECS_ThrowingGrenade)
		return;


	FVector EyesLoc; FRotator EyesRot;
	Character->GetActorEyesViewPoint(EyesLoc, EyesRot); // ���������� ��ȿ

	const FVector Start = EyesLoc;
	const FVector End = Start + (EyesRot.Vector() * MaxWireDistance);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WireTrace), false, Character);
	Params.AddIgnoredActor(Character);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
	if (!bHit) return;                     // ����̸� ���� (���ϸ� ������� �ٲ㵵 ��)

	WireTarget = Hit.ImpactPoint;
	bIsAttached = true;

	if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		Move->SetMovementMode(MOVE_Flying);
         
}
void UWireComponent::FireWire()
{
	if (Character && !Character->HasAuthority())
	{
		ServerFireWire();
		return;
	}
	ServerFireWire();
}

void UWireComponent::ReleaseWire()
{
	if (Character && !Character->HasAuthority())
	{
		ServerReleaseWire();
		return;
	}
	ServerReleaseWire();
}


void UWireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	/*
	if (bIsAttached && Character && !Character->IsElimmed()) {
		FVector Direction = (WireTarget - Character->GetActorLocation()).GetSafeNormal();
		FVector NewVelocity = Direction * PullSpeed;
		Character->GetCharacterMovement()->Velocity = NewVelocity;

		if(FVector::Dist(Character->GetActorLocation(), WireTarget) < 150.f) {
			ReleaseWire();
		}
	}
	*/
	if (bIsAttached && Character)
	{
		FVector ToTarget = WireTarget - Character->GetActorLocation();
		float Dist = ToTarget.Size();

		if (Dist > 120.f) // ��ǥ���� ���� �ָ� ��� ����
		{
			FVector Dir = ToTarget.GetSafeNormal();
			Character->GetCharacterMovement()->Velocity = Dir * PullSpeed;
		}
		else
		{
			// ��ǥ ��ó�� �ڵ� ����
			ReleaseWire();
		}
	}
}

