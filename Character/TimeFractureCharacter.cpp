// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeFractureCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
ATimeFractureCharacter::ATimeFractureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), FName("Head")); //���ϳ����� �̿��ؼ� followcamera�� cameraarm�� ����
	FollowCamera->bUsePawnControlRotation = true; //ī�޶�ȸ���ϸ� ����ȸ��

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent); //��Ʈ������Ʈ�� ����
}
void ATimeFractureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATimeFractureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATimeFractureCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATimeFractureCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATimeFractureCharacter::LookUp);
	//������Ʈ ���ÿ� ����� Ű�� �̸��� ���ε��Ѵ�. this ->�� �Լ��� �ִ� �Լ��� �ҷ���
}
void ATimeFractureCharacter::MoveForward(float Value)//"��Ʈ�ѷ��� �ٶ󺸴� ������ �������� ĳ������ ���� ���͸� ���ϴ� ��"
{
	if (Controller != nullptr && Value != 0.f) //�÷��̾ ��Ʈ�ѷ��� ���� �ֳ� && �����̰��ֳ�
	{
		//��Ʈ�ѷ��� �������� ������.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //��Ʈ�ѷ��� ȸ���� ����Ѵ�.
		//YawRotation: ��Ʈ�ѷ��� Yaw ��(�¿� ȸ���� ���)�� ������ FRotator ����
		//��ġ(X�� ȸ��), ��(Z�� ȸ��)�� ������ �� ĳ���Ͱ� ����� �������θ� ȸ���ϰ� �Ϸ��� �ǵ�
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); //���溤�͸� �����ͼ� �ʱ�ȭ , 
		//RotationMatrix(YawRotation): �� �������͸� ������� ȸ�� ����� ����
		//�� ȸ�� ����� ĳ���Ͱ� �ٶ󺸴� ���� ���� ���� ���������� ��� ����
		// .GetUnitAxis(EAxis::X): ȸ�� ��Ŀ��� X��(���� ����) �� ���� ���͸� ������
		//��, ���� �ٶ󺸴� ���� �������� ������ ������� �˷��ִ� ����
		//�������ͷ� ȸ�� ��Ʈ������ ���� �� �ְ� ȸ�� ��Ʈ������ ������ �������, frotator���� frotationmatrix�� ����� �̸� �������̶� �θ��� fvector�� ��ȯ�Ѵ�.
		//����� ������ ������ ��Ÿ��
		AddMovementInput(Direction, Value); //����� ���� ���Ͽ� ĳ���Ͱ� �ش�������� �̵��ϰ� �Ѵ�. , �ӵ��� ���ӵ��� �ʿ��ϴ�.
	}
	
}
void ATimeFractureCharacter::MoveRight(float Value)
{
	//��Ʈ�ѷ��� �������� ������.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //��Ʈ�ѷ��� ȸ���� ����Ѵ�.
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); //���溤�͸� �����ͼ� �ʱ�ȭ , 
	//�������ͷ� ȸ�� ��Ʈ������ ���� �� �ְ� ȸ�� ��Ʈ������ ������ �������, frotator���� frotationmatrix�� ����� �̸� �������̶� �θ��� fvector�� ��ȯ�Ѵ�.
	//����� ������ ������ ��Ÿ��
	AddMovementInput(Direction, Value); //����� ���� ���Ͽ� ĳ���Ͱ� �ش�������� �̵��ϰ� �Ѵ�. , �ӵ��� ���ӵ��� �ʿ��ϴ�.
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

