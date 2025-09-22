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

	//  디버그: 내가 클라인지/서버인지, 값이 뭔지 확인
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_WireState] %s  bIsAttached=%s"),
		Character->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		bIsAttached ? TEXT("True") : TEXT("False"));

	// (선택) 여기서 AnimInstance에 바로 반영해도 되지만,
	// AnimInstance가 매 프레임 WireComponent를 읽으므로 생략 가능.
	// 남겨두고 싶다면 안전하게:
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		if (UTFAniminstance* Anim = Cast<UTFAniminstance>(Mesh->GetAnimInstance()))
			Anim->bIsWireAttached = bIsAttached;

	// 이동모드는 서버만 (클라에서 바꾸면 흔들림 생김)
	if (Character->HasAuthority())
		if (auto* Move = Character->GetCharacterMovement())
			Move->SetMovementMode(bIsAttached ? MOVE_Flying : MOVE_Falling);
	*/
	if (!Character) Character = Cast<ATimeFractureCharacter>(GetOwner());
	if (!Character) return;

	// 디버그: 어디서 불렸는지 로그
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_WireState] %s bIsAttached=%s"),
		Character->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		bIsAttached ? TEXT("True") : TEXT("False"));

	// 클라 전용: AnimInstance에 bool 반영
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
	if (!bHit) return;                     // 허공이면 종료 (원하면 허용으로 바꿔도 됨)

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

