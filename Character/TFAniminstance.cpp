// Fill out your copyright notice in the Description page of Project Settings.


#include "TFAniminstance.h"
#include "TimeFractureCharacter.h" //캐릭터의 헤더파일을 포함시킨다
#include "GameFramework/CharacterMovementComponent.h" //캐릭터의 이동 컴포넌트를 포함시킨다
#include "Kismet/KismetMathLibrary.h" //수학 라이브러리를 포함시킨다
#include "UnrealProject_7A/TFComponents/CombatStates.h" //전투 상태를 포함시킨다
#include "UnrealProject_7A/Weapon/Weapon.h" //무기의 헤더파일을 포함시킨다
#include "UnrealProject_7A/TFComponents/WireComponent.h" //와이어 컴포넌트를 포함시킨다
void UTFAniminstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //부모클래스의 초기화함수 호출
	if (!TFCharacter)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
		//trygetpawnowner는 이 애님 인스턴스를 쓰고 있는 스켈레탈 메시의 주인 폰을 받아온다.
		//여기선 내가 만듬 캐릭터 클래스를 캐스팅해서 받아옴
		//애님 인스턴스는 앞으로 이 캐릭터의 상태를 참조하게됨

	}
}

void UTFAniminstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 캐릭터 참조 확인
	if (TFCharacter == nullptr)
	{
		TFCharacter = Cast<ATimeFractureCharacter>(TryGetPawnOwner());
	}
	if (TFCharacter == nullptr) return;

	//캐릭터 포인터를 못찾으면 다시 시도 못찾으면 그냥 리턴 멀티플레이나 복제 타이밍 이슈에서 안전장치
	// 와이어 연결 상태 갱신
	if (TFCharacter && TFCharacter->WireComponent)
	{
		bIsWireAttached = TFCharacter->WireComponent->IsAttached();
	}

	//와이어가 붙어있는지 확인하여 전용포즈를 블렌딩함
	// 이동 관련 정보 계산
	FVector Velocity = TFCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size(); //xy평면에서의 속도 크기 z가 0인것은 점프 낙하때문에 속도가 켜져서 달리는 애니메이션으로 바뀌는걸 막기위해

	bIsInAir = TFCharacter->GetCharacterMovement()->IsFalling(); //낙하중인지
	bIsAccelerating = TFCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f; //플레이어가 실제로 미끄러지는건지 아님 방향키 이동키를 누르고 있는지 구분
	bIsCrouching = TFCharacter->bIsCrouched; //앉기상태

	//전형적인 3인칭 슈터 애님인스턴스 처리
	// 전투 / 조준 상태 갱신
	bWeaponEquipped = TFCharacter->IsWeaponEquipped(); //무기를 들었는지
	bIsAiming = TFCharacter->IsAiming(); //에임중인지
	bEliminated = TFCharacter->IsElimmed(); //사망상태인지


	// 조준 회전값 계산
	FRotator AimRotation = TFCharacter->GetBaseAimRotation(); //캐릭터가 실제 마우스로 조준중인 방향
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TFCharacter->GetVelocity()); //캐릭터가 실제로 이동하고 있는 방향

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//두개의 차이를 구하면 내 몸은 앞으로 달리고 있는데 시선은 왼쪽방향이다 라는 정보가 나옴
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
	YawOffset = DeltaRotation.Yaw;
	//보간장치 RInterpTo를 이용해 부드럽게 변화시키기 (상체 트위스트 애니메이션이 튀지 않도록 하기위해서
	//yaw는 이동중 상체가 어느정도 틀어져야하는지 쓰임

	AO_Yaw = TFCharacter->GETAO_YAW(); //좌우 가로 돌기
	AO_Pitch = TFCharacter->GETAO_PITCH(); //위아래 돌기
	//애님그래프에서 에임오프셋 노드에 들어가는데, 상체의 에임 룩엣 보정용 값이다.
	//즉 크로스헤어를 어디에 두고 있든 상체 팔 총구가 그 방향을 따라가게함

	// 무기 및 IK 보정
	EquippedWeapon = TFCharacter->GetEquippedWeapon(); //총을 양손으로 정확히 쥐게 만들기 하는 뼈 보정부분
	//IK / Inverse Kinematics (역운동학)

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TFCharacter->GetMesh())
	{
		// 왼손 위치 보정 (IK)
		//LefthandSocket이 있는데, 왼손은 여기 부분에 자연스럽게 잡아야함
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		//소켓의 월드 좌표를 가져옴
		FVector OutPos;
		FRotator OutRot;
		TFCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		//캐릭터 오른손 뼈 기준의 로컬 좌표계로 반환
		//왜냐면 총은 오른손에 붙어있고 왼손은 그 총을 받쳐주는 식이라서 왼손의 목표 위치를 오른손 기준 상대 위치로 넘겨줘야 애님 그래프의 양손 IK노드로 자연스럽게 붙일 수 있음.
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));

		// 오른손 회전 보정 (로컬 플레이어 전용)
		if (TFCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//총구 소켓의 월드위치를 구한다.

			FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("muzz"), ERelativeTransformSpace::RTS_World);
			FVector MuzzleLocation = MuzzleTransform.GetLocation();
			FVector AimTarget = TFCharacter->GetHitTarget();//캐릭터가 지금 실제로 조준중인 실제 히트 타켓위치를 가져옴

			FVector LookAtDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
			//총구에서 히트 타겟까지의 방향벡터를 구함
			FRotator BaseRotation = UKismetMathLibrary::MakeRotFromYZ(-LookAtDirection, FVector::UpVector);

			FQuat CorrectionQuat(FVector::YAxisVector, FMath::DegreesToRadians(-90.f));
			FQuat TargetWorldQuat = BaseRotation.Quaternion() * CorrectionQuat;

			const FTransform& ComponentToWorld = TFCharacter->GetMesh()->GetComponentToWorld();
			FQuat TargetComponentQuat = ComponentToWorld.InverseTransformRotation(TargetWorldQuat); //회전계를 메시 로컬 기준으로 다시 변환

			RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetComponentQuat.Rotator(), DeltaTime, 3000.f);
			//보간하여 부드럽게 따라가도록함 (3000.F -> 즉석 반응)
			//그 방향을 기준으로 오른손의 회전을 재설정함
		}
	}


	// FABRIK 사용 여부 결정
	ECombatState CurrentCombatState = TFCharacter->GetCombatState();
	bUseFABRIK = (CurrentCombatState == ECombatState::ECS_Unoccupied) && bWeaponEquipped && !bEliminated;
	//총을 쥐거나 에임자세 잡을때 어색하지 않게 해줌
	//FABRIK 은 FORWARD AND BACKWARD REACHING INVERSE KINEMATICS 의 약자이며 , 언리얼에서 두 손 , 팔꿈치, 어깨등을 자연스럽게 맞추게 하는 IK 솔버중 하나이다.
	//재장전중일땐 왼손 IK를 끔
}
