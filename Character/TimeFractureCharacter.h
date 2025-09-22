#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnrealProject_7A/Interfaces/InteractWithCrosshairInterface.h"
#include "UnrealProject_7A/TFComponents/CombatStates.h"
#include "TimeFractureCharacter.generated.h"

UCLASS()
class UNREALPROJECT_7A_API ATimeFractureCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	ATimeFractureCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
	virtual void PostInitializeComponents() override; //속성 초기화 함수
	void Elim(); //플레이어 제거 함수
	void UpdateHUDHealth();
	void UpdateHUDShield();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();
	UPROPERTY(Replicated)
	bool bDisableGameplay = false; //게임플레이 비활성화 여부
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope); //스나이퍼 스코프 위젯 표시 함수
	bool bisElimmed = false; //플레이어가 제거되었는지 여부
	void PlayThrowGrendadeMontage(); //수류탄 투척 애니메이션 몽타주 재생 함수
	void HandleChatKey();
	void HandleChatCancel();
	void HandleChatSubmit();
	UFUNCTION(BlueprintImplementableEvent)
	void BloodScreen(); //0914 피격이펙트
	UFUNCTION(Client, Reliable)
	void ClientShowBloodScreen();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Wire", meta = (AllowPrivateAccess = "true"))
	class UWireComponent* WireComponent;
protected:
	virtual void Jump() override;
	virtual void Destroyed() override; //캐릭터가 파괴될 때 호출되는 함수
	virtual void BeginPlay() override;
	void SwapButtonPressed(); //무기 교체 버튼 함수
	virtual void PossessedBy(AController* NewController) override;	
	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed(); //서버에서 무기 교체 버튼 함수
	//
	void Dive();
	UFUNCTION(Server, Reliable)
	void ServerDivePressed();   

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDive();       
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButton();
	void CrouchButton(); //크라우치 버튼 함수
	void AimButton(); //조준 버튼 함수
	void AimButtonRelease(); //조준 버튼 해제 함수
	void AimOffset(float DeltaTime); //조준 오프셋 함수
	void FireButtonPressed();
	void FireButtonReleased(); //발사 버튼 해제 함수
	void ReloadButtonPressed(); //재장전 버튼 함수
	void GrenadeButtonPressed(); //수류탄 버튼 함수
	void WireButtonPressed(); //와이어 버튼 함수
	void WireButtonReleased(); //와이어 버튼 해제 함수
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController,
		AActor* DamageCursor); //피해를 받았을 때 호출되는 함수
	//움직임 함수
	void PollInit(); //허드와 같은 함수 초기화
	virtual void OnRep_PlayerState() override; //플레이어 상태가 변경되었을 때 호출되는 함수

private:


	UFUNCTION(Server, Reliable)
	void ServerSetActorRotation(const FRotator& NewRotation);
	float AO_YAW; //조준 회전 Yaw 값, 서버에서 클라이언트로 복제되는 변수
	float AO_PITCH; //조준 회전 Yaw, Pitch 값
	FRotator BaseAimRotation; //기본 조준 회전
	UPROPERTY(VisibleAnywhere, Category = "camera")
	class USpringArmComponent* CameraBoom; //카메라 붐 컴포넌트, 카메라와 캐릭터 사이의 거리 조절
	UPROPERTY(VisibleAnywhere, Category="camera")
	class UCameraComponent* FollowCamera; //촬영하는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget; //오버헤드 위젯
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; //겹치는 무기	
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector CrouchingCameraOffset; //크라우치 상태일 때 카메라의 상대 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector NormalOffset; //일반 상태일 때 카메라의 상대 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	float AimCameraOffset; //조준 상태일 때 카메라의 상대 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	float normalAimCameraOffset; //조준 상태일 때 카메라의 상대 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	float CameraInterpSpeed = 15.f; //카메라 보간 속도
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //겹치는 무기가 바뀔 때 호출되는 함수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	class UCBComponent* CombatComponent; //전투 컴포넌트
	UPROPERTY(VisibleAnyWhere)
	class UBuffComponent* BuffComponent; //버프 컴포넌트
	UFUNCTION(Server, Reliable)
	void ServerEquipButton(); //서버에서 장착 버튼을 누를 때 호출되는 함수
	//수류탄
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* AttachedGrenade; //장착된 수류탄 메쉬 컴포넌트
	//
	//애니메이션 몽타주
	//
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ThrowGrenadeMontage; //수류탄 투척 애니메이션 몽타주
	UPROPERTY(EditAnywhere,Category=Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage; //피격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage; //재장전 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* DiveMontage;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f; //카메라가 캐릭터와 얼마나 가까이 있을 때 카메라를 숨길지 결정하는 임계값
	void HideCameraIfCharacterClose(); //캐릭터가 가까이 있을 때 카메라를 숨기는 함수

	//플레이어 체력
	UPROPERTY(EditAnywhere, Category="Player State")
	float MaxHealth = 100.f; //최대 체력
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category = "Player State")
	float Health=100.f; //현재 체력
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxShield = 100.f; //최대 체력
	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player State")
	float Shield = 0.f; //현재 체력
	UFUNCTION()
	void OnRep_Shield(float LastShield); //실드가 바뀔 때 호출되는 함수
	UPROPERTY(Replicated)
	FRotator MoveRotation; //캐릭터의 이동 회전
	UPROPERTY(EditAnywhere)
	float RollDistance = 200.f;
	FTimerHandle ElimTimer; //플레이어 제거 타이머 핸들
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f; //플레이어 제거 지연 시간
	void ElimTimerFinished(); //플레이어 제거 타이머가 끝났을 때 호출되는 함수
	UFUNCTION()
	void OnRep_Health(float LastHealth); //체력이 바뀔 때 호출되는 함수
	class ATFPlayerController* TfPlayerController; //플레이어 컨트롤러
	UPROPERTY()
	class ATFPlayerState* TfPlayerState; //플레이어 상태

	//기본 무기 설정
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass; //기본 무기 클래스
	
	UPROPERTY(EditAnywhere)
	class USoundCue* HitCharacterSound;
	UPROPERTY(EditAnywhere)
	USoundCue* HitConfirmSound;
	//0914 피격 ui
	
public:
	bool bIsDodging;
	void EnsureOverheadWidgetLocal();
	void RefreshOverheadName();
	void SpawnDefaultWeapon(); //기본 무기를 생성하는 함수
	void UpdateHUDAmmo(); //HUD의 탄약을 업데이트하는 함수
	void SetOverlappingWeapon(AWeapon* Weapon); //겹치는 무기를 설정하는 함수
	bool IsWeaponEquipped(); //무기가 장착되어 있는지 확인하는 함수
	bool IsAiming();
	FORCEINLINE float GETAO_YAW() const { return AO_YAW; }
	FORCEINLINE float GETAO_PITCH() const { return AO_PITCH; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; } //팔로우 카메라를 반환하는 함수
	FORCEINLINE bool IsElimmed() const { return bisElimmed; } //플레이어가 제거되었는지 여부를 반환하는 함수
	FORCEINLINE float GetHealth() const { return Health; } //현재 체력을 반환하는 함수
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; } //현재 체력을 반환하는 함수
	FORCEINLINE UCBComponent* GetCombatComponent() const { return CombatComponent; } //전투 컴포넌트를 반환하는 함수
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; } //재장전 애니메이션 몽타주를 반환하는 함수
	FORCEINLINE USkeletalMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; } //장착된 수류탄 메쉬 컴포넌트를 반환하는 함수
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; } //버프 컴포넌트를 반환하는 함수
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; } //체력을 설정하는 함수
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; } //실드를 설정하는 함수
	FORCEINLINE float GetShield() const { return Shield; } //실드를 설정하는 함수
	FORCEINLINE float GetMaxShield() const { return MaxShield; } //최대 실드를 반환하는 함수
	class UOverheadWidget* GetOverheadWidget() const;
	ECombatState GetCombatState() const; //전투 상태를 반환하는 함수 
	AWeapon* GetEquippedWeapon();
	void PlayFireMontage(bool bAiming); //무기 발사 모션 재생 함수
	void PlayElimMontage(); //피격 애니메이션 몽타주 재생 함수
	void PlayHitReactMontage(); //히트 리액트 몽타주 재생 함수
	void PlayReloadMontage(); //재장전 애니메이션 몽타주 재생 함수
	UFUNCTION(NetMulticast, UnReliable)
	void MultiCastHit(); //히트 이벤트를 멀티캐스트로 호출하는 함수
	
	FVector GetHitTarget() const; //히트 타겟을 반환하는 함수
	
};
