// Fill out your copyright notice in the Description page of Project Settings.


#include "WireComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "CombatStates.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/Character/TFAniminstance.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
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
void UWireComponent::MulticastDrawWire_Implementation(const FVector& Start, const FVector& End)
{
	FVector FaceStart = Start;
	if (Character && Character->GetMesh())
	{
		FaceStart = Character->GetActorLocation()
			+ Character->GetActorForwardVector() * 50.f
			+ FVector(0.f, 0.f, 70.f);
	}
	DrawDebugLine(
		GetWorld(),
		FaceStart,
		End,
		FColor::Green,
		false,     
		0.05f,     
		0,         
		2.0f       
	);
}
void UWireComponent::MulticastPlayWireEffects_Implementation(const FVector& Start, const FVector& Target)
{
	if (WireShootEffect)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WireShootEffect,                                  
			Character->GetMesh(),                             
			FName("muzz"),                                    
			FVector::ZeroVector,                              
			FRotator::ZeroRotator,                           
			EAttachLocation::SnapToTargetIncludingScale,      
			true                                              
		);
	}

	if (WireImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			WireImpactEffect,
			Target,
			FRotator::ZeroRotator
		);
	}
}
void UWireComponent::OnRep_WireState()
{
	if (!Character) Character = Cast<ATimeFractureCharacter>(GetOwner());
	if (!Character) return;
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_WireState] %s bIsAttached=%s"),
		Character->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		bIsAttached ? TEXT("True") : TEXT("False"));

	if (!Character->HasAuthority()) //  클라에서만 실행
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
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling); //걷기모드로 변경
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
	Character->GetActorEyesViewPoint(EyesLoc, EyesRot); // 서버에서도 유효

	const FVector Start = EyesLoc;
	const FVector End = Start + (EyesRot.Vector() * MaxWireDistance);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WireTrace), false, Character);
	Params.AddIgnoredActor(Character);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
	if (!bHit) return;                    

	WireTarget = Hit.ImpactPoint;
	bIsAttached = true;

	if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		Move->SetMovementMode(MOVE_Flying);
	MulticastPlayWireEffects(Start, WireTarget);
	MulticastDrawWire(Start, WireTarget);
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
	if (bIsAttached && Character)
	{
		FVector ToTarget = WireTarget - Character->GetActorLocation();
		float Dist = ToTarget.Size();

		if (Dist > 120.f) // 목표까지 아직 멀면 계속 끌기
		{
			FVector Dir = ToTarget.GetSafeNormal();
			Character->GetCharacterMovement()->Velocity = Dir * PullSpeed;
		}
		else
		{
			// 목표 근처면 자동 해제
			ReleaseWire();
		}
	}
}

