#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimeFractureCharacter.generated.h"

UCLASS()
class UNREALPROJECT_7A_API ATimeFractureCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATimeFractureCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	virtual void PostInitializeComponents() override; //�Ӽ� �ʱ�ȭ �Լ�
	
protected:
	virtual void BeginPlay() override;
	//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButton();
	//������ �Լ�
private:
	UPROPERTY(VisibleAnywhere, Category="ī�޶�")
	class UCameraComponent* FollowCamera; //�Կ��ϴ� ī�޶�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget; //������� ����
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; //��ġ�� ����	

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //��ġ�� ���Ⱑ �ٲ� �� ȣ��Ǵ� �Լ�
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	class UCBComponent* CombatComponent; //���� ������Ʈ
public:
	void SetOverlappingWeapon(AWeapon* Weapon); //��ġ�� ���⸦ �����ϴ� �Լ�
};
