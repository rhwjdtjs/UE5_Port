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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	virtual void PostInitializeComponents() override; //�Ӽ� �ʱ�ȭ �Լ�
	void Elim(); //�÷��̾� ���� �Լ�
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();
protected:
	virtual void BeginPlay() override;
	void UpdateHUDHealth();
	//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButton();
	void CrouchButton(); //ũ���ġ ��ư �Լ�
	void AimButton(); //���� ��ư �Լ�
	void AimButtonRelease(); //���� ��ư ���� �Լ�
	void AimOffset(float DeltaTime); //���� ������ �Լ�
	void FireButtonPressed();
	void FireButtonReleased(); //�߻� ��ư ���� �Լ�
	void ReloadButtonPressed(); //������ ��ư �Լ�
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController,
		AActor* DamageCursor); //���ظ� �޾��� �� ȣ��Ǵ� �Լ�
	//������ �Լ�
	void PollInit(); //���� ���� �Լ� �ʱ�ȭ
	virtual void OnRep_PlayerState() override; //�÷��̾� ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ�
private:
	float AO_YAW; //���� ȸ�� Yaw ��, �������� Ŭ���̾�Ʈ�� �����Ǵ� ����
	float AO_PITCH; //���� ȸ�� Yaw, Pitch ��
	FRotator BaseAimRotation; //�⺻ ���� ȸ��
	UPROPERTY(VisibleAnywhere, Category = "camera")
	class USpringArmComponent* CameraBoom; //ī�޶� �� ������Ʈ, ī�޶�� ĳ���� ������ �Ÿ� ����
	UPROPERTY(VisibleAnywhere, Category="camera")
	class UCameraComponent* FollowCamera; //�Կ��ϴ� ī�޶�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget; //������� ����
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; //��ġ�� ����	
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector CrouchingCameraOffset; //ũ���ġ ������ �� ī�޶��� ��� ��ġ
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector NormalOffset; //�Ϲ� ������ �� ī�޶��� ��� ��ġ
	UPROPERTY(EditAnywhere, Category = "camera")
	float AimCameraOffset; //���� ������ �� ī�޶��� ��� ��ġ
	UPROPERTY(EditAnywhere, Category = "camera")
	float normalAimCameraOffset; //���� ������ �� ī�޶��� ��� ��ġ
	UPROPERTY(EditAnywhere, Category = "camera")
	float CameraInterpSpeed = 15.f; //ī�޶� ���� �ӵ�
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //��ġ�� ���Ⱑ �ٲ� �� ȣ��Ǵ� �Լ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	class UCBComponent* CombatComponent; //���� ������Ʈ
	UFUNCTION(Server, Reliable)
	void ServerEquipButton(); //�������� ���� ��ư�� ���� �� ȣ��Ǵ� �Լ�

	//
	//�ִϸ��̼� ��Ÿ��
	//
	UPROPERTY(EditAnywhere,Category=Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage; //�ǰ� �ִϸ��̼� ��Ÿ��
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage; //������ �ִϸ��̼� ��Ÿ��

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f; //ī�޶� ĳ���Ϳ� �󸶳� ������ ���� �� ī�޶� ������ �����ϴ� �Ӱ谪
	void HideCameraIfCharacterClose(); //ĳ���Ͱ� ������ ���� �� ī�޶� ����� �Լ�

	//�÷��̾� ü��
	UPROPERTY(EditAnywhere, Category="Player State")
	float MaxHealth = 100.f; //�ִ� ü��
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category = "Player State")
	float Health=100.f; //���� ü��
	bool bisElimmed = false; //�÷��̾ ���ŵǾ����� ����
	FTimerHandle ElimTimer; //�÷��̾� ���� Ÿ�̸� �ڵ�
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f; //�÷��̾� ���� ���� �ð�
	void ElimTimerFinished(); //�÷��̾� ���� Ÿ�̸Ӱ� ������ �� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_Health(); //ü���� �ٲ� �� ȣ��Ǵ� �Լ�
	class ATFPlayerController* TfPlayerController; //�÷��̾� ��Ʈ�ѷ�
	UPROPERTY()
	class ATFPlayerState* TfPlayerState; //�÷��̾� ����
public:
	void SetOverlappingWeapon(AWeapon* Weapon); //��ġ�� ���⸦ �����ϴ� �Լ�
	bool IsWeaponEquipped(); //���Ⱑ �����Ǿ� �ִ��� Ȯ���ϴ� �Լ�
	bool IsAiming();
	FORCEINLINE float GETAO_YAW() const { return AO_YAW; }
	FORCEINLINE float GETAO_PITCH() const { return AO_PITCH; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; } //�ȷο� ī�޶� ��ȯ�ϴ� �Լ�
	FORCEINLINE bool IsElimmed() const { return bisElimmed; } //�÷��̾ ���ŵǾ����� ���θ� ��ȯ�ϴ� �Լ�
	FORCEINLINE float GetHealth() const { return Health; } //���� ü���� ��ȯ�ϴ� �Լ�
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; } //���� ü���� ��ȯ�ϴ� �Լ�
	ECombatState GetCombatState() const; //���� ���¸� ��ȯ�ϴ� �Լ� 
	AWeapon* GetEquippedWeapon();
	void PlayFireMontage(bool bAiming); //���� �߻� ��� ��� �Լ�
	void PlayElimMontage(); //�ǰ� �ִϸ��̼� ��Ÿ�� ��� �Լ�
	void PlayHitReactMontage(); //��Ʈ ����Ʈ ��Ÿ�� ��� �Լ�
	void PlayReloadMontage(); //������ �ִϸ��̼� ��Ÿ�� ��� �Լ�
	UFUNCTION(NetMulticast, UnReliable)
	void MultiCastHit(); //��Ʈ �̺�Ʈ�� ��Ƽĳ��Ʈ�� ȣ���ϴ� �Լ�
	
	FVector GetHitTarget() const; //��Ʈ Ÿ���� ��ȯ�ϴ� �Լ�
};
