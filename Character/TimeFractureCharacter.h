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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
	virtual void PostInitializeComponents() override; //속성 초기화 함수
	
protected:
	virtual void BeginPlay() override;
	//
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButton();
	//움직임 함수
private:
	UPROPERTY(VisibleAnywhere, Category="카메라")
	class UCameraComponent* FollowCamera; //촬영하는 카메라
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget; //오버헤드 위젯
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; //겹치는 무기	

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //겹치는 무기가 바뀔 때 호출되는 함수
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	class UCBComponent* CombatComponent; //전투 컴포넌트
	UFUNCTION(Server, Reliable)
	void ServerEquipButton(); //서버에서 장착 버튼을 누를 때 호출되는 함수
public:
	void SetOverlappingWeapon(AWeapon* Weapon); //겹치는 무기를 설정하는 함수
	bool IsWeaponEquipped(); //무기가 장착되어 있는지 확인하는 함수
};
