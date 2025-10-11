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
// ============================================================
// [초기화] BeginPlay()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 소유 캐릭터(ATimeFractureCharacter) 캐스팅.
// ============================================================
void UWireComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ATimeFractureCharacter>(GetOwner());

}
// ============================================================
// [복제 항목 등록] GetLifetimeReplicatedProps()
// ------------------------------------------------------------
// 기능 요약 : 
//   - bIsAttached, WireTarget 네트워크 복제 등록.
// ============================================================
void UWireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWireComponent, bIsAttached);
	DOREPLIFETIME(UWireComponent, WireTarget);
}
// ============================================================
// [클라이언트 벽 충돌 처리] ClientWallFail()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 와이어 상태에서 벽과의 거리 감지 후 너무 가깝거나 너무 멀면 해제.
// ============================================================
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
// ============================================================
// [와이어 성공 멀티캐스트] MulticastWireSuccess()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 와이어 성공 시 쿨다운 시작 및 HUD 표시.
// ============================================================
void UWireComponent::MulticastWireSuccess_Implementation()
{
	bCanFireWire = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle, this, &UWireComponent::ResetWireCooldown, WireCooldown, false);
	}

	if (Character && Character->IsLocallyControlled())
	{
		if (!WireCooldownWidget && WireCooldownWidgetClass)
		{
			if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
			{
				WireCooldownWidget = CreateWidget<UUserWidget>(PC, WireCooldownWidgetClass);
				if (WireCooldownWidget)
				{
					WireCooldownWidget->AddToViewport();
					WireCooldownText = Cast<UTextBlock>(WireCooldownWidget->GetWidgetFromName(TEXT("WireCoolTime")));
				}
			}
		}

		if (WireCooldownWidget)
		{
			WireCooldownWidget->SetVisibility(ESlateVisibility::Visible);
		}

		RemainingCooldown = WireCooldown;
		if (WireCooldownText)
		{
			WireCooldownText->SetText(FText::FromString(
				FString::Printf(TEXT("Wire(Q)-CoolTimeLeft - %.0f"), RemainingCooldown)));
		}

		Character->GetWorldTimerManager().ClearTimer(CooldownUITimerHandle);
		Character->GetWorldTimerManager().SetTimer(
			CooldownUITimerHandle, this, &UWireComponent::TickWireCooldownUI, 1.0f, true);
	}
}
// ============================================================
// [와이어 실패 클라이언트 처리] ClientWireFail()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 타겟 미존재 시 "No Target" 메시지를 HUD에 표시하고 1초 후 자동 숨김.
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
// ============================================================
// [지퍼 사운드 재생] MulticastStartZipperSound()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 와이어 이동 중 루프 사운드를 캐릭터에 부착하여 재생.
// ============================================================
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
// ============================================================
// [와이어 발사 사운드 재생] MulticastPlayWireSound()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 와이어 발사 순간 사운드 재생.
// ============================================================
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
// ============================================================
// [쿨타임 UI 갱신] UpdateWireCooldownUI()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 남은 쿨타임 시간에 따라 HUD 텍스트를 실시간 갱신.
// ============================================================
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
// ============================================================
// [쿨타임 UI 타이머 틱] TickWireCooldownUI()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 매초 남은 쿨타임을 줄이고, 0초 시 UI 숨김 처리.
// ============================================================
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
// ============================================================
// [쿨타임 초기화] ResetWireCooldown()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 쿨다운 종료 후 bCanFireWire 플래그를 true로 복귀.
// ============================================================
void UWireComponent::ResetWireCooldown()
{
	bCanFireWire = true;
}
void UWireComponent::OnRep_CanFireWire()
{
}
// ============================================================
// [와이어 시각화] MulticastDrawWire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 캐릭터 앞에서 타겟까지 디버그 라인으로 와이어 표시.
// ============================================================
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
// ============================================================
// [와이어 비주얼 이펙트] MulticastPlayWireEffects()
// ------------------------------------------------------------
// 기능 요약 : 
//   - Niagara 시스템을 사용하여 팔다리에서 와이어 이펙트 생성.
//   - Impact 지점에서 파티클 폭발 효과 재생.
// ============================================================
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
// ============================================================
// [와이어 상태 복제 응답] OnRep_WireState()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 애님인스턴스 변수(bIsWireAttached) 동기화.
//   - 해제 시 이펙트 정리.
// ============================================================
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
	if (!bIsAttached)
	{
		MulticastStopWireEffects(); // 로컬에도 안전하게 정리
	}
}
// ============================================================
// [이펙트 중단] MulticastStopWireEffects()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 오디오 및 Niagara 이펙트 모두 중단 및 포인터 정리.
// ============================================================
void UWireComponent::MulticastStopWireEffects_Implementation()
{
	if (ZipperAudioComponent && ZipperAudioComponent->IsPlaying())
	{
		ZipperAudioComponent->Stop();
		ZipperAudioComponent = nullptr;
	}
	if (ActiveTravelEffectLeft) { ActiveTravelEffectLeft->Deactivate(); ActiveTravelEffectLeft = nullptr; }
	if (ActiveTravelEffectRight) { ActiveTravelEffectRight->Deactivate(); ActiveTravelEffectRight = nullptr; }
	if (ActiveTravelEffectLeftFront) { ActiveTravelEffectLeftFront->Deactivate(); ActiveTravelEffectLeftFront = nullptr; }
	if (ActiveTravelEffectRightFront) { ActiveTravelEffectRightFront->Deactivate(); ActiveTravelEffectRightFront = nullptr; }
}
// ============================================================
// [서버 해제 처리] ServerReleaseWire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 와이어 상태를 해제하고 이동 모드를 낙하로 변경.
// ============================================================
void UWireComponent::ServerReleaseWire_Implementation()
{
	bIsAttached = false;
	MulticastStopWireEffects(); 
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
// ============================================================
// [서버 와이어 발사] ServerFireWire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 시야 방향으로 라인트레이스 후 타겟에 닿으면 와이어 연결.
//   - 실패 시 클라이언트 피드백 표시.
// ============================================================
void UWireComponent::ServerFireWire_Implementation()
{
	if (Character->bIsCrouched) return;
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
// ============================================================
// [와이어 발사 요청] FireWire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트 → 서버 RPC 호출로 와이어 발사 요청.
// ============================================================
void UWireComponent::FireWire()
{
	if (!bCanFireWire) return;
	if (Character->bIsCrouched) return;
	if (Character && !Character->HasAuthority())
	{
		ServerFireWire();
	}
	else {
		ServerFireWire();
	}
}
// ============================================================
// [와이어 해제 요청] ReleaseWire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 로컬 및 서버 권한 여부에 따라 RPC로 해제 처리.
// ============================================================
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

// ============================================================
// [Tick 처리] TickComponent()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 매 프레임마다 와이어 당김 이동, 벽 감지 및 자동 해제 수행.
// 알고리즘 설명 : 
//   1. 서버 : 실제 물리 이동 처리.
//   2. 클라이언트 : 로컬 보정 및 자동 해제 요청.
// ============================================================
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
		// 클라 보정이 필요하다면 'RPC 호출'이 아니라 '로컬'로 처리
		if (Character && Character->IsLocallyControlled() && bIsAttached)
		{
			// 벽 체크 & 너무 가까우면 로컬에서 먼저 풀기 요청
			FHitResult WallHit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Character);
			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				WallHit, Character->GetActorLocation(), WireTarget, ECC_Visibility, Params);

			float Dist = (WireTarget - Character->GetActorLocation()).Size();
			if ((bBlocked && WallHit.Distance < 100.f) || Dist <= 120.f)
			{
				ReleaseWire(); // 이건 서버 RPC를 타게 됨
			}
		}
	}

}

