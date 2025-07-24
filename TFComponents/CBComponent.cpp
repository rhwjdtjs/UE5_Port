

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //������ ���� ��������� ���Խ�Ų��.
#include "GameFramework/CharacterMovementComponent.h" //ĳ���� �̵� ������Ʈ�� ���Խ�Ų��.
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UnrealProject_7a/PlayerController/TFPlayerController.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "CombatStates.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
//	//baseWalkSpeed = 600.f; //�⺻ �ȴ� �ӵ��� �����Ѵ�.
//	AimingWalkSpeed = 300.f; //���� ������ �ȴ� �ӵ��� �����Ѵ�.
}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip == nullptr) return;
	if (EquippedWeapon) {
		EquippedWeapon->DropWeapon(); //�̹� ������ ���Ⱑ ������ ����Ѵ�.
	}
	EquippedWeapon = WeaponEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	EquippedWeapon->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandSocket")
	);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo(); //������ ������ HUD ź���� �����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //������ ������ ���� ź���� �����Ѵ�.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //������ ���⸦ �����Ѵ�.
	DOREPLIFETIME(UCBComponent, bisAiming);// ���� ���θ� �����Ѵ�.
	DOREPLIFETIME(UCBComponent, CombatState); //���� ���¸� �����Ѵ�.
	DOREPLIFETIME_CONDITION(UCBComponent, CarriedAmmo, COND_OwnerOnly); // �����ڸ� ź���� �����Ѵ�.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //������ ���⸦ �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
}

void UCBComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) {
			Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
		}
		break;
	case ECombatState::ECS_Reloading:
		UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_CombatState - Reloading started"));
			HandleReload(); //���ε带 ó���Ѵ�.
		
		break;
	case ECombatState::ECS_MAX:
		break;
	}
}
void UCBComponent::Reload() {
	if (CarriedAmmo > 0 && CombatState !=ECombatState::ECS_Reloading && EquippedWeapon && !EquippedWeapon->IsEmpty()) {
		ServerReload(); //������ ���ε� ��û�� ������.
	}
}
void UCBComponent::HandleReload()
{
	if (Character) {
		Character->PlayReloadMontage(); //ĳ������ ���ε� ��Ÿ�ָ� ����Ѵ�.
	}
}
int32 UCBComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0; //������ ���Ⱑ ������ 0�� ��ȯ�Ѵ�.
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo(); //źâ�� ���� ������ ����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // ������ ������ Ÿ���� ���� ź�� �ʿ� �ִ��� Ȯ��
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // ������ ���� Ÿ���� ���� ���� ź�� ���� ������
		int32 Least = FMath::Min(RoomInMag, AmountCarried); // źâ�� ���� ������ ���� ź�� �� �� ���� ���� ����
		return FMath::Clamp(RoomInMag, 0, Least); // 0�� Least ������ ������ RoomInMag�� �����Ͽ� ��ȯ (�ִ� ���� ������ ź�� ��)
	}
	return 0;
}
void UCBComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //ĳ���Ͱ� ������ �Լ��� �����Ѵ�.
	int32 ReloadAmount = AmountToReload(); //���ε��� ���� ����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // ������ ������ Ÿ���� ���� ź�� �ʿ� �ִ��� Ȯ��
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount; // ���� ź�࿡�� ���ε��� ���� �����Ѵ�.
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //���� ���� ź���� ������Ʈ�Ѵ�.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
	EquippedWeapon->AddAmmo(-ReloadAmount); //������ ���⿡ ���ε��� ���� �߰��Ѵ�.
}
void UCBComponent::ServerReload_Implementation()
{
	
	
	CombatState = ECombatState::ECS_Reloading;//���� ���¸� ���ε� ���·� �����Ѵ�
	HandleReload();//���ε带 ó���Ѵ�.
}
void UCBComponent::FinishReload()
{
	if (Character == nullptr) return;
	
	if (Character->HasAuthority()) {
		// �������� ���� ����
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues(); //ź�� ���� ������Ʈ�Ѵ�.
	}
	else {
		// Ŭ���̾�Ʈ������ ���� RPC ȣ��
		ServerFinishReload();
	}
	if (bFireButtonPressed) {
		Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
	}
}

void UCBComponent::ServerFinishReload_Implementation()
{
	CombatState = ECombatState::ECS_Unoccupied;
}
	
void UCBComponent::OnRep_CarriedAmmo()
{
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
}



void UCBComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingCarriedAmmo); //������ �⺻ ź���� �����Ѵ�.
}

void UCBComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.

	if (bisAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), 
			DeltaTime, EquippedWeapon->GetZoomInterpSpeed()); //���� ������ �� FOV�� �����Ѵ�.
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed); //���� ���°� �ƴ� �� FOV�� �����Ѵ�.
	}
	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV); //ĳ������ ī�޶� FOV�� ���� FOV�� �����Ѵ�.
	}
}

void UCBComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCBComponent::FireTimerFinished, EquippedWeapon->FireRate); //�߻� Ÿ�̸Ӹ� �����Ѵ�.
}
void UCBComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.
	bCanFire = true; //�߻� ���� ���θ� true�� �����Ѵ�.
	if (bFireButtonPressed && EquippedWeapon->bAutoMatickFire) {
		Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
	}
}

bool UCBComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false; //������ ���Ⱑ ������ �߻��� �� ����.
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied; //�߻��� �� �ִ� �������� Ȯ���Ѵ�.
}



void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //ĳ������ �ִ� �ȴ� �ӵ��� �⺻ �ӵ��� �����Ѵ�.
		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView; //�⺻ FOV�� �����Ѵ�.
			CurrentFOV = DefaultFOV; //���� FOV�� �⺻ FOV�� �����Ѵ�.
		}
	}
	if (Character->HasAuthority()) {
		InitializeCarriedAmmo(); //�������� ĳ������ ���� ź���� �ʱ�ȭ�Ѵ�.
	}
}



void UCBComponent::SetAiming(bool bAiming)
{
	bisAiming = bAiming; //���� ���θ� �����Ѵ�.
	ServerSetAiming(bAiming); //�������� ���� ���θ� �����Ѵ�.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //���� ���ο� ���� ĳ������ �ִ� �ȴ� �ӵ��� �����Ѵ�.
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
	bFireButtonPressed = bPressed; //�߻� ��ư�� ���ȴ��� ���θ� �����Ѵ�.
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCBComponent::Fire()
{
	if (CanFire())
	{
		ServerFire(HitTarget); // ������ �ùٸ� Ÿ�� ����
		if (EquippedWeapon) {
			bCanFire = false; //�߻� ���� ���θ� false�� �����Ѵ�.
			CrosshairShootingFactor = 0.85f; //�߻� ��ư�� ������ �� ũ�ν���� ��� ����� ������Ų��.
		}
		StartFireTimer(); //�߻� Ÿ�̸Ӹ� �����Ѵ�.
	}
}

void UCBComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	MulticastFire(TraceHitTargert); //�������� �߻� ��Ƽĳ��Ʈ �Լ��� ȣ���Ѵ�.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //�������� ���� ���θ� �����Ѵ�.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //���� ���ο� ���� ĳ������ �ִ� �ȴ� �ӵ��� �����Ѵ�.
	}
}


void UCBComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState==ECombatState::ECS_Unoccupied) {
		Character->PlayFireMontage(bisAiming); //ĳ������ �߻� ����� ����Ѵ�.
		EquippedWeapon->Fire(TraceHitTargert); //���⸦ �߻��Ѵ�.
	}
}

void UCBComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld && Character && Character->GetFollowCamera()) {
		FVector CameraLocation = Character->GetFollowCamera()->GetComponentLocation();
		// ĸ�� ��������ŭ �� �ָ� ���� (�ڱ� �ݸ��� �ۿ��� ����)
		float CapsuleRadius = Character->GetCapsuleComponent() ? Character->GetCapsuleComponent()->GetScaledCapsuleRadius() : 34.f;
		FVector Start = CameraLocation + CrosshairWorldDirection * (CapsuleRadius + 20.f);
		FVector End = Start + (CrosshairWorldDirection * 80000.f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		if (Character->GetCapsuleComponent())
			QueryParams.AddIgnoredComponent(Character->GetCapsuleComponent());

		TArray<FHitResult> HitResults;
		GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_Visibility, QueryParams);

		bool bFoundValidHit = false;
		for (const FHitResult& Hit : HitResults) {
			if (Hit.GetActor() && Hit.GetActor() != Character) {
				TraceHitResult = Hit;
				bFoundValidHit = true;
				break;
			}
		}
		// ����� ���� ImpactPoint�� End�� ����
		if (!bFoundValidHit) {
			TraceHitResult = FHitResult();
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() == Character) {
			UE_LOG(LogTemp, Warning, TEXT("�ڱ� �ڽ� ����"));
		}
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()) {
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else {
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCBComponent::SetHUDCrossharis(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->GetController()) : Controller; // �÷��̾� ��Ʈ�ѷ��� �����´�.
	if (Controller) {
		TFHUD = TFHUD == nullptr ? Cast<ATFHUD>(Controller->GetHUD()) : TFHUD; // HUD�� �����´�.
		if (TFHUD) {

			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter; // ���Ⱑ ������ �߾� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight; // ���Ⱑ ������ ������ ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom; // ���Ⱑ ������ �Ʒ��� ũ�ν��� �����Ѵ�.
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr; // ���Ⱑ ������ �߾� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsLeft = nullptr; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsRight = nullptr; // ���Ⱑ ������ ������ ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsTop = nullptr; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsBottom = nullptr; // ���Ⱑ ������ �Ʒ��� ũ�ν��� �����Ѵ�.
			}
			//ũ�ν���� �������� ���
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed); // �ȴ� �ӵ� ������ �����Ѵ�.
			FVector2D VelocityMultiplierRange(0.f, 1.f); // �ӵ� ���� ������ �����Ѵ�.
			FVector Velocity = Character->GetVelocity(); // ĳ������ �ӵ��� �����´�.
			Velocity.Z = 0.f; // Z���� �ӵ��� 0���� �����.

			CrosshairVelocityFactor=FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange,
				Velocity.Size()); // �ȴ� �ӵ��� ���� ũ�ν���� �������带 ����Ѵ�.
			if (bisAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorValue1, DeltaTime, AimFactorValue2); // ���� ������ �� ũ�ν���� �������带 �����Ѵ�.
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, AimFactorValue2); // ���� ���°� �ƴ� �� ũ�ν���� �������带 �����Ѵ�.
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f); // ��� ������ �� ũ�ν���� �������带 �����Ѵ�.
			HUDPackage.CrosshairSpread = 0.5f+ CrosshairVelocityFactor - CrosshairAimFactor+CrosshairShootingFactor; // ũ�ν���� �������带 �����Ѵ�.
			TFHUD->SetHUDPackage(HUDPackage); // HUD ��Ű���� �����Ѵ�.
		}
	}
}

void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) // ���� �÷��̾� ��Ʈ�ѷ��� ��쿡�� �����Ѵ�.
	{
		FHitResult HitResult; // Ʈ���̽� ����� ������ ����
		TraceUnderCrosshairs(HitResult); // �� �����Ӹ��� ȭ�� �߾� �Ʒ��� ��ü�� �����Ѵ�.
		HitTarget = HitResult.ImpactPoint; // ��Ʈ Ÿ���� Ʈ���̽� ����� �浹 �������� �����Ѵ�.
		SetHUDCrossharis(DeltaTime); // �� �����Ӹ��� HUD�� ũ�ν��� �����Ѵ�.
		InterpFOV(DeltaTime); // FOV�� �����Ѵ�.
	}

}

