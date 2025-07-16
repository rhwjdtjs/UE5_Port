

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //복제를 위한 헤더파일을 포함시킨다.
#include "GameFramework/CharacterMovementComponent.h" //캐릭터 이동 컴포넌트를 포함시킨다.
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UnrealProject_7a/PlayerController/TFPlayerController.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
//	//baseWalkSpeed = 600.f; //기본 걷는 속도를 설정한다.
//	AimingWalkSpeed = 300.f; //조준 상태의 걷는 속도를 설정한다.
}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip == nullptr) return;

	EquippedWeapon = WeaponEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	EquippedWeapon->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandSocket")
	);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //장착된 무기를 복제한다.
	DOREPLIFETIME(UCBComponent, bisAiming);// 조준 여부를 복제한다.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //장착된 무기를 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
}


void UCBComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.

	if (bisAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), 
			DeltaTime, EquippedWeapon->GetZoomInterpSpeed()); //조준 상태일 때 FOV를 보간한다.
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed); //조준 상태가 아닐 때 FOV를 보간한다.
	}
	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV); //캐릭터의 카메라 FOV를 현재 FOV로 설정한다.
	}
}

void UCBComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCBComponent::FireTimerFinished, EquippedWeapon->FireRate); //발사 타이머를 시작한다.
}
void UCBComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.
	bCanFire = true; //발사 가능 여부를 true로 설정한다.
	if (bFireButtonPressed && EquippedWeapon->bAutoMatickFire) {
		Fire(); //발사 버튼이 눌렸으면 발사한다.
	}
}

void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //캐릭터의 최대 걷는 속도를 기본 속도로 설정한다.
		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView; //기본 FOV를 설정한다.
			CurrentFOV = DefaultFOV; //현재 FOV를 기본 FOV로 설정한다.
		}

	}
}

void UCBComponent::SetAiming(bool bAiming)
{
	bisAiming = bAiming; //조준 여부를 설정한다.
	ServerSetAiming(bAiming); //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}

void UCBComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		EquippedWeapon->AttachToComponent(
			Character->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("RightHandSocket")
		);
	}
}

void UCBComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; //발사 버튼이 눌렸는지 여부를 설정한다.
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCBComponent::Fire()
{
	if (bCanFire)
	{
		ServerFire(HitTarget); // 서버에 올바른 타겟 전달
		if (EquippedWeapon) {
			bCanFire = false; //발사 가능 여부를 false로 설정한다.
			CrosshairShootingFactor = 0.85f; //발사 버튼이 눌렸을 때 크로스헤어 사격 계수를 증가시킨다.
		}
		StartFireTimer(); //발사 타이머를 시작한다.
	}
}

void UCBComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	MulticastFire(TraceHitTargert); //서버에서 발사 멀티캐스트 함수를 호출한다.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}


void UCBComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	if (EquippedWeapon == nullptr) return;
	if (Character) {
		Character->PlayFireMontage(bisAiming); //캐릭터의 발사 모션을 재생한다.
		EquippedWeapon->Fire(TraceHitTargert); //무기를 발사한다.
	}
}

void UCBComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	
	FVector2D ViewportSize; // 화면 크기를 저장할 변수
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); // 화면 크기를 가져온다.
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 화면 중앙의 좌표를 계산한다.
	FVector CrosshairWorldPostion; // 화면 중앙의 월드 위치를 저장할 변수
	FVector CrosshairWorldDirection; // 화면 중앙의 월드 방향을 저장할 변수
	bool bScreenToWorld =UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPostion, CrosshairWorldDirection); // 화면 중앙의 위치와 방향을 계산한다.
	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPostion; // 시작 위치는 화면 중앙의 월드 위치로 설정한다.

		if (Character) {
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size(); // 캐릭터와 시작 위치 사이의 거리를 계산한다.
			Start += CrosshairWorldDirection * (DistanceToCharacter + TargetDistance); // 시작 위치를 캐릭터와의 거리만큼 앞으로 이동시킨다.
		}
		FVector End = Start + (CrosshairWorldDirection * 80000.f); // 끝 위치는 시작 위치에서 월드 방향으로 10000 단위 떨어진 위치
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character); // 캐릭터를 무시하는 쿼리 파라미터를 설정한다.
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility, QueryParams);// 라인 트레이스를 수행한다.
		if (TraceHitResult.GetActor() == Character)
		{
			UE_LOG(LogTemp, Warning, TEXT("자기 자신 맞음"));
		}
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()) {
			HUDPackage.CrosshairColor = FLinearColor::Red; // 히트된 액터가 인터페이스를 구현하고 있으면 크로스헤어 색상을 빨간색으로 설정한다.
		}
		else {
			HUDPackage.CrosshairColor = FLinearColor::White; // 히트된 액터가 인터페이스를 구현하고 있지 않으면 크로스헤어 색상을 흰색으로 설정한다.
		}
	}
}

void UCBComponent::SetHUDCrossharis(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->GetController()) : Controller; // 플레이어 컨트롤러를 가져온다.
	if (Controller) {
		TFHUD = TFHUD == nullptr ? Cast<ATFHUD>(Controller->GetHUD()) : TFHUD; // HUD를 가져온다.
		if (TFHUD) {

			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter; // 무기가 있으면 중앙 크로스헤어를 설정한다.
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft; // 무기가 있으면 왼쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight; // 무기가 있으면 오른쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop; // 무기가 있으면 위쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom; // 무기가 있으면 아래쪽 크로스헤어를 설정한다.
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr; // 무기가 있으면 중앙 크로스헤어를 설정한다.
				HUDPackage.CrosshairsLeft = nullptr; // 무기가 있으면 왼쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsRight = nullptr; // 무기가 있으면 오른쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsTop = nullptr; // 무기가 있으면 위쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsBottom = nullptr; // 무기가 있으면 아래쪽 크로스헤어를 설정한다.
			}
			//크로스헤어 스프레드 계산
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed); // 걷는 속도 범위를 설정한다.
			FVector2D VelocityMultiplierRange(0.f, 1.f); // 속도 곱셈 범위를 설정한다.
			FVector Velocity = Character->GetVelocity(); // 캐릭터의 속도를 가져온다.
			Velocity.Z = 0.f; // Z축의 속도를 0으로 만든다.

			CrosshairVelocityFactor=FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange,
				Velocity.Size()); // 걷는 속도에 따라 크로스헤어 스프레드를 계산한다.
			if (bisAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorValue1, DeltaTime, AimFactorValue2); // 조준 상태일 때 크로스헤어 스프레드를 보간한다.
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, AimFactorValue2); // 조준 상태가 아닐 때 크로스헤어 스프레드를 보간한다.
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f); // 사격 상태일 때 크로스헤어 스프레드를 보간한다.
			HUDPackage.CrosshairSpread = 0.5f+ CrosshairVelocityFactor - CrosshairAimFactor+CrosshairShootingFactor; // 크로스헤어 스프레드를 설정한다.
			TFHUD->SetHUDPackage(HUDPackage); // HUD 패키지를 설정한다.
		}
	}
}

void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) // 로컬 플레이어 컨트롤러인 경우에만 실행한다.
	{
		FHitResult HitResult; // 트레이스 결과를 저장할 변수
		TraceUnderCrosshairs(HitResult); // 매 프레임마다 화면 중앙 아래의 물체를 추적한다.
		HitTarget = HitResult.ImpactPoint; // 히트 타겟을 트레이스 결과의 충돌 지점으로 설정한다.
		SetHUDCrossharis(DeltaTime); // 매 프레임마다 HUD의 크로스헤어를 설정한다.
		InterpFOV(DeltaTime); // FOV를 보간한다.
	}

}

