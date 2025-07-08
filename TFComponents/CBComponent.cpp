

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
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
//	//baseWalkSpeed = 600.f; //�⺻ �ȴ� �ӵ��� �����Ѵ�.
//	AimingWalkSpeed = 300.f; //���� ������ �ȴ� �ӵ��� �����Ѵ�.
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
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //������ ���⸦ �����Ѵ�.
	DOREPLIFETIME(UCBComponent, bisAiming);// ���� ���θ� �����Ѵ�.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //������ ���⸦ �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
}


void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //ĳ������ �ִ� �ȴ� �ӵ��� �⺻ �ӵ��� �����Ѵ�.
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


	}
}

void UCBComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; //�߻� ��ư�� ���ȴ��� ���θ� �����Ѵ�.
	if (bFireButtonPressed) {
		FHitResult HitResult; // Ʈ���̽� ����� ������ ����
		TraceUnderCrosshairs(HitResult); // �� �����Ӹ��� ȭ�� �߾� �Ʒ��� ��ü�� �����Ѵ�.
		ServerFire(HitResult.ImpactPoint); //�������� �߻� ��ư�� ���ȴ��� ���θ� �����Ѵ�.
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
	if (Character) {
		Character->PlayFireMontage(bisAiming); //ĳ������ �߻� ����� ����Ѵ�.
		EquippedWeapon->Fire(TraceHitTargert); //���⸦ �߻��Ѵ�.
	}
}

void UCBComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize; // ȭ�� ũ�⸦ ������ ����
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); // ȭ�� ũ�⸦ �����´�.
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // ȭ�� �߾��� ��ǥ�� ����Ѵ�.
	FVector CrosshairWorldPostion; // ȭ�� �߾��� ���� ��ġ�� ������ ����
	FVector CrosshairWorldDirection; // ȭ�� �߾��� ���� ������ ������ ����
	bool bScreenToWorld =UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPostion, CrosshairWorldDirection); // ȭ�� �߾��� ��ġ�� ������ ����Ѵ�.
	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPostion; // ���� ��ġ�� ȭ�� �߾��� ���� ��ġ
		FVector End = Start + (CrosshairWorldDirection * 80000.f); // �� ��ġ�� ���� ��ġ���� ���� �������� 10000 ���� ������ ��ġ

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility); // ���� Ʈ���̽��� ����Ͽ� ȭ�� �߾� �Ʒ��� ��ü�� �����Ѵ�.
	}
}

void UCBComponent::SetHUDCrossharis(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->GetController()) : Controller; // �÷��̾� ��Ʈ�ѷ��� �����´�.
	if (Controller) {
		TFHUD = TFHUD == nullptr ? Cast<ATFHUD>(Controller->GetHUD()) : TFHUD; // HUD�� �����´�.
		if (TFHUD) {
			FHUDPakage HUDPackage; // HUD ��Ű���� �����Ѵ�.
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

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor; // ũ�ν���� �������带 �����Ѵ�.
			TFHUD->SetHUDPackage(HUDPackage); // HUD ��Ű���� �����Ѵ�.
		}
	}
}

void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrossharis(DeltaTime); // �� �����Ӹ��� HUD�� ũ�ν��� �����Ѵ�.
	if (Character && Character->IsLocallyControlled()) // ���� �÷��̾� ��Ʈ�ѷ��� ��쿡�� �����Ѵ�.
	{
		FHitResult HitResult; // Ʈ���̽� ����� ������ ����
		TraceUnderCrosshairs(HitResult); // �� �����Ӹ��� ȭ�� �߾� �Ʒ��� ��ü�� �����Ѵ�.
		HitTarget = HitResult.ImpactPoint; // ��Ʈ Ÿ���� Ʈ���̽� ����� �浹 �������� �����Ѵ�.
	}

}

