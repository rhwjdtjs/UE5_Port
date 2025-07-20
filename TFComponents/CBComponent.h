#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "CBComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UCBComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCBComponent();
	friend class ATimeFractureCharacter;
	void EquipWeapon(class AWeapon* WeaponEquip); //무기를 장착하는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
private:
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon; //장착된 무기
	class ATimeFractureCharacter* Character; //캐릭터
	class ATFPlayerController* Controller; //플레이어 컨트롤러
	class ATFHUD* TFHUD; //HUD
	UPROPERTY(Replicated)
	bool bisAiming; //조준 여부
	UPROPERTY(EditAnywhere, Category = "Combat")
	float baseWalkSpeed; //기본 걷는 속도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimingWalkSpeed; //조준 상태의 걷는 속도

	bool bFireButtonPressed; //발사 버튼이 눌렸는지 여부
	//허브 & 크로스헤어
	FHUDPakage HUDPackage; //HUD 패키지 구조체
	float CrosshairVelocityFactor; //크로스헤어 속도 계수
	FVector  HitTarget; //히트 타겟
	float CrosshairAimFactor; //크로스헤어 조준 계수
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue1 = 0.58f; //조준 계수 값 1
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue2 = 30.f; //조준 계수 값 1
	float CrosshairShootingFactor; //크로스헤어 사격 계수
	//FOV 에이밍
	//조준중이 아닐때 FOV
	float DefaultFOV; //기본 FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV=30.f; //줌된 FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed=20.f; //줌 인터폴레이션 속도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TargetDistance;
	float CurrentFOV; //현재 FOV
	void InterpFOV(float DeltaTime); //FOV를 보간하는 함수
	//자동화기
	FTimerHandle FireTimer; //발사 타이머 핸들
	void StartFireTimer(); //발사 타이머를 시작하는 함수
	void FireTimerFinished(); //발사 타이머가 끝났을 때 호출되는 함수
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bCanFire = true; //발사 가능 여부

	bool CanFire(); //발사 가능 여부를 확인하는 함수
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming); //조준 상태를 설정하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //서버에서 조준 상태를 설정하는 함수
	UFUNCTION()
	void OnRep_EquippedWeapon(); //장착된 무기가 바뀔 때 호출되는 함수
	void FireButtonPressed(bool bPressed); //발사 버튼이 눌렸을 때 호출되는 함수
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTargert); //서버에서 발사 버튼이 눌렸는지 여부를 설정하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTargert); //발사 멀티캐스트 함수

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); //조준선 아래의 물체를 추적하는 함수
	void SetHUDCrossharis(float DeltaTime); //HUD의 크로스헤어를 설정하는 함수
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
