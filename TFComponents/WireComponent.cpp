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
#include "TimerManager.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "NiagaraComponent.h"
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
void UWireComponent::ClientWallFail_Implementation()
{
	if (bIsAttached && Character)
	{
		FVector ToTarget = WireTarget - Character->GetActorLocation();
		float Dist = ToTarget.Size();
		FHitResult WallHit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);
		bool bBlocked = GetWorld()->LineTraceSingleByChannel(
			WallHit,
			Character->GetActorLocation(),
			WireTarget,
			ECC_Visibility,
			Params
		);
		if (bBlocked && WallHit.Distance < 100.f)
		{
			// 벽에 너무 가까이 → 강제 해제
			ReleaseWire();
			return;
		}
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
void UWireComponent::MulticastWireSuccess_Implementation()
{
	bCanFireWire = false;

	// 쿨타임 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UWireComponent::ResetWireCooldown,
			WireCooldown,
			false
		);
	}

	// 로컬 UI 처리
	if (Character && Character->IsLocallyControlled())
	{
		if (!WireCooldownWidget && WireCooldownWidgetClass)
		{
			WireCooldownWidget = CreateWidget<UUserWidget>(
				Character->GetWorld(),
				WireCooldownWidgetClass
			);
			if (WireCooldownWidget)
			{
				WireCooldownWidget->AddToViewport();
				WireCooldownText = Cast<UTextBlock>(
					WireCooldownWidget->GetWidgetFromName(TEXT("WireCoolTime"))
				);
			}
		}

		if (WireCooldownWidget)
		{
			WireCooldownWidget->SetVisibility(ESlateVisibility::Visible);
		}

		RemainingCooldown = WireCooldown;
		UpdateWireCooldownUI();
		Character->GetWorldTimerManager().SetTimer(
			CooldownUITimerHandle,
			this,
			&UWireComponent::TickWireCooldownUI,
			1.0f,
			true
		);
	}
}
void UWireComponent::ClientWireFail_Implementation()
{
	if (Character && Character->IsLocallyControlled())
	{
		if (!WireCooldownWidget && WireCooldownWidgetClass)
		{
			WireCooldownWidget = CreateWidget<UUserWidget>(
				Character->GetWorld(),
				WireCooldownWidgetClass
			);
			if (WireCooldownWidget)
			{
				WireCooldownWidget->AddToViewport();
				WireCooldownText = Cast<UTextBlock>(
					WireCooldownWidget->GetWidgetFromName(TEXT("WireCoolTime"))
				);
			}
		}

		if (WireCooldownWidget && WireCooldownText)
		{
			WireCooldownWidget->SetVisibility(ESlateVisibility::Visible);
			WireCooldownText->SetText(FText::FromString(TEXT("There is no target for the wire.")));

			// 1초 뒤 자동 숨김
			FTimerHandle TempHandle;
			Character->GetWorldTimerManager().SetTimer(
				TempHandle,
				[this]()
				{
					if (WireCooldownText)
					{
						WireCooldownText->SetText(FText::GetEmpty());
					}
					if (WireCooldownWidget)
					{
						WireCooldownWidget->SetVisibility(ESlateVisibility::Hidden);
					}
				},
				1.0f, false
			);
		}
	}
}
void UWireComponent::MulticastStartZipperSound_Implementation()
{
	if (Character && ZipperLoopSound)
	{
		ZipperAudioComponent = UGameplayStatics::SpawnSoundAttached(
			ZipperLoopSound,
			Character->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true   // bStopWhenAttachedToDestroyed = true
		);
		if (ZipperAudioComponent)
		{
			ZipperAudioComponent->Play();
		}
	}
}
void UWireComponent::MulticastPlayWireSound_Implementation()
{
	if (WireFireSound && Character)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WireFireSound,
			Character->GetActorLocation()
		);
	}
}
void UWireComponent::UpdateWireCooldownUI()
{
	if (!WireCooldownText) return;

	// 남은 시간 계산
	float Remaining = GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimerHandle);

	if (Remaining > 0.f)
	{
		FString CooldownString = FString::Printf(TEXT("Wire(Q)-CoolTimeLeft - %.1f"), Remaining);
		WireCooldownText->SetText(FText::FromString(CooldownString));

		// 일정 주기로 다시 호출 (예: 0.1초마다)
		GetWorld()->GetTimerManager().SetTimer(
			CooldownUITimerHandle,
			this,
			&UWireComponent::UpdateWireCooldownUI,
			0.1f,
			false
		);
	}
	else
	{
		WireCooldownText->SetText(FText::GetEmpty()); // 텍스트 숨김
	}
}
void UWireComponent::TickWireCooldownUI()
{
	if (WireCooldownText)
	{
		RemainingCooldown -= 1.f;
		if (RemainingCooldown > 0.f)
		{
			FString CooldownMsg = FString::Printf(TEXT("Wire(Q)-CoolTimeLeft - %.0f"), RemainingCooldown);
			WireCooldownText->SetText(FText::FromString(CooldownMsg));
		}
		else
		{
			WireCooldownText->SetText(FText::GetEmpty());
			if (WireCooldownWidget)
			{
				WireCooldownWidget->SetVisibility(ESlateVisibility::Hidden);
			}
			// 타이머 정지
			Character->GetWorldTimerManager().ClearTimer(CooldownUITimerHandle);
		}
	}
}
void UWireComponent::ResetWireCooldown()
{
	bCanFireWire = true;
}
void UWireComponent::OnRep_CanFireWire()
{
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
		FColor::Black,
		false,     
		1.f,     
		0,         
		1.0f       
	);
}
void UWireComponent::MulticastPlayWireEffects_Implementation(const FVector& Start, const FVector& Target)
{
	if (Character && WireTravelEffect)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		FTransform SocketTransformLL = Mesh->GetSocketTransform(FName("LeftUpLeg"));
		FRotator SocketRotLL = SocketTransformLL.Rotator();
		FTransform SocketTransformRL = Mesh->GetSocketTransform(FName("RightUpLeg"));
		FRotator SocketRotRL = SocketTransformRL.Rotator();
		FTransform SocketTransformLA = Mesh->GetSocketTransform(FName("LeftArm"));
		FRotator SocketRotLA = SocketTransformLA.Rotator();
		FTransform SocketTransformRA = Mesh->GetSocketTransform(FName("RightArm"));
		FRotator SocketRotRA = SocketTransformRA.Rotator();
		// 왼쪽
		ActiveTravelEffectLeft = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WireTravelEffect,
			Mesh,
			FName("LeftUpLeg"), // 중심부 소켓에 붙이고
			FVector(50.f, -100.f, 0.f), // 소켓 기준 옆으로 밀기
			SocketRotLL,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		ActiveTravelEffectRight = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WireTravelEffect,
			Mesh,
			FName("RightUpLeg"),
			FVector(50.f, 100.f, 0.f),
			SocketRotRL,
			EAttachLocation::KeepRelativeOffset,
			true
		);
		ActiveTravelEffectLeftFront = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WireTravelEffect,
			Mesh,
			FName("LeftArm"),   // 필요하면 "UpperArm_L" 같은 실제 소켓명 확인
			FVector(100.f, -100.f, 0.f),
			SocketRotLA,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		ActiveTravelEffectRightFront = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WireTravelEffect,
			Mesh,
			FName("RightArm"),  // 필요하면 "UpperArm_R"
			FVector(100.f, 100.f, 0.f),
			SocketRotRA,
			EAttachLocation::KeepRelativeOffset,
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
		if (ZipperAudioComponent && ZipperAudioComponent->IsPlaying())
		{
			ZipperAudioComponent->Stop();
			ZipperAudioComponent = nullptr;
		}
		if (ActiveTravelEffectLeft)
		{
			ActiveTravelEffectLeft->Deactivate();
			ActiveTravelEffectLeft = nullptr;
		}
		if (ActiveTravelEffectRight)
		{
			ActiveTravelEffectRight->Deactivate();
			ActiveTravelEffectRight = nullptr;
		}
		if (ActiveTravelEffectLeftFront)
		{
			ActiveTravelEffectLeftFront->Deactivate();
			ActiveTravelEffectLeftFront = nullptr;
		}
		if (ActiveTravelEffectRightFront) {
			ActiveTravelEffectRightFront->Deactivate();
			ActiveTravelEffectRightFront = nullptr;
		}
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling); //걷기모드로 변경
	}
}
void UWireComponent::ServerFireWire_Implementation()
{
	if (!bCanFireWire) return;
	if (!Character) return;
	if (Character->IsElimmed() || Character->bIsDodging ||
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
	if (!bHit) {
		if (Character->IsLocallyControlled()) // 서버 본인
		{
			ClientWireFail_Implementation();
		}
		else
		{
			ClientWireFail(); // 다른 클라
		}
		return;
	}
	WireTarget = Hit.ImpactPoint;
	bIsAttached = true;

	if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		Move->SetMovementMode(MOVE_Flying);
	MulticastPlayWireEffects(Start, WireTarget);
	MulticastDrawWire(Start, WireTarget);
	MulticastPlayWireSound();
	MulticastStartZipperSound();
	MulticastWireSuccess();
	
}
void UWireComponent::FireWire()
{
	if (!bCanFireWire) return;
	if (Character && !Character->HasAuthority())
	{
		ServerFireWire();
	}
	else {
		ServerFireWire();
	}
	/*
	if (Character && Character->IsLocallyControlled())
	{
		if (!WireCooldownWidget && WireCooldownWidgetClass)
		{
			WireCooldownWidget = CreateWidget<UUserWidget>(
				Character->GetWorld(),
				WireCooldownWidgetClass
			);
			if (WireCooldownWidget)
			{
				WireCooldownWidget->AddToViewport();
				WireCooldownText = Cast<UTextBlock>(
					WireCooldownWidget->GetWidgetFromName(TEXT("WireCoolTime"))
				);
			}
		}

		if (WireCooldownWidget)
		{
			WireCooldownWidget->SetVisibility(ESlateVisibility::Visible);
		}

		RemainingCooldown = WireCooldown;
		UpdateWireCooldownUI();
		Character->GetWorldTimerManager().SetTimer(
			CooldownUITimerHandle,
			this,
			&UWireComponent::TickWireCooldownUI,
			1.0f,
			true
		);
	}

	bCanFireWire = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&UWireComponent::ResetWireCooldown,
			WireCooldown,
			false
		);
	}
	*/
}

void UWireComponent::ReleaseWire()
{
	if (Character && !Character->HasAuthority())
	{
		if (ZipperAudioComponent && ZipperAudioComponent->IsPlaying())
		{
			ZipperAudioComponent->Stop();
			ZipperAudioComponent = nullptr;
		}
		if (ActiveTravelEffectLeft)
		{
			ActiveTravelEffectLeft->Deactivate();
			ActiveTravelEffectLeft = nullptr;
		}
		if (ActiveTravelEffectRight)
		{
			ActiveTravelEffectRight->Deactivate();
			ActiveTravelEffectRight = nullptr;
		}
		if (ActiveTravelEffectLeftFront)
		{
			ActiveTravelEffectLeftFront->Deactivate();
			ActiveTravelEffectLeftFront = nullptr;
		}
		if (ActiveTravelEffectRightFront) {
			ActiveTravelEffectRightFront->Deactivate();
			ActiveTravelEffectRightFront = nullptr;
		}
		ServerReleaseWire();
		return;
	}
	ServerReleaseWire();
}


void UWireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character->HasAuthority())
	{
		if (bIsAttached && Character)
		{
			FVector ToTarget = WireTarget - Character->GetActorLocation();
			float Dist = ToTarget.Size();
			FHitResult WallHit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Character);
			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				WallHit,
				Character->GetActorLocation(),
				WireTarget,
				ECC_Visibility,
				Params
			);
			if (bBlocked && WallHit.Distance < 100.f)
			{
				// 벽에 너무 가까이 → 강제 해제
				ReleaseWire();
				return;
			}
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
	else {
		ClientWallFail();
	}

}

