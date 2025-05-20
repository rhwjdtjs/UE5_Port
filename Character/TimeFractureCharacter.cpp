// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), FName("Head")); //소켓네임을 이용해서 followcamera를 cameraarm에 부착
	FollowCamera->bUsePawnControlRotation = true; //카메라회전하면 몸도회전

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //루트컴포넌트에 부착
}
void ATimeFractureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATimeFractureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATimeFractureCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATimeFractureCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATimeFractureCharacter::LookUp);
	//프로젝트 세팅에 저장된 키의 이름을 바인드한다. this ->이 함수의 있는 함수를 불러옴
}
void ATimeFractureCharacter::MoveForward(float Value)//"컨트롤러가 바라보는 방향을 기준으로 캐릭터의 전방 벡터를 구하는 것"
{
	if (Controller != nullptr && Value != 0.f) //플레이어가 컨트롤러를 갖고 있나 && 움직이고있나
	{
		//컨트롤러의 전방으로 보낸다.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //컨트롤러의 회전을 담당한다.
		//YawRotation: 컨트롤러의 Yaw 값(좌우 회전만 고려)을 가지고 FRotator 생성
		//피치(X축 회전), 롤(Z축 회전)은 무시함 → 캐릭터가 지면과 수평으로만 회전하게 하려는 의도
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); //전방벡터를 가져와서 초기화 , 
		//RotationMatrix(YawRotation): 이 로테이터를 기반으로 회전 행렬을 생성
		//이 회전 행렬은 캐릭터가 바라보는 방향 정보 등을 수학적으로 담고 있음
		// .GetUnitAxis(EAxis::X): 회전 행렬에서 X축(전방 방향) 의 단위 벡터를 가져옴
		//즉, 현재 바라보는 방향 기준으로 전방이 어디인지 알려주는 벡터
		//로테이터로 회전 매트릭스를 만들 수 있고 회전 매트릭스에 정보가 담겨있음, frotator에서 frotationmatrix를 만들고 이를 단위축이라 부르고 fvector를 반환한다.
		//지면과 평행한 방향을 나타냄
		AddMovementInput(Direction, Value); //방향과 값을 취하여 캐릭터가 해당방향으로 이동하게 한다. , 속도와 가속도가 필요하다.
	}
	
}
void ATimeFractureCharacter::MoveRight(float Value)
{
	//컨트롤러의 전방으로 보낸다.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //컨트롤러의 회전을 담당한다.
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //전방벡터를 가져와서 초기화 , 
	//로테이터로 회전 매트릭스를 만들 수 있고 회전 매트릭스에 정보가 담겨있음, frotator에서 frotationmatrix를 만들고 이를 단위축이라 부르고 fvector를 반환한다.
	//지면과 평행한 방향을 나타냄
	AddMovementInput(Direction, Value); //방향과 값을 취하여 캐릭터가 해당방향으로 이동하게 한다. , 속도와 가속도가 필요하다.
}
void ATimeFractureCharacter::Turn(float Value)
{
	AddControllerYawInput(Value); 
}
void ATimeFractureCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATimeFractureCharacter::BeginPlay()
{
	Super::BeginPlay();
}
void ATimeFractureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

