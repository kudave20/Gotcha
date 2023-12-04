// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Weapon.h"
#include "Component/CombatComponent.h"

AShooterCharacterBase::AShooterCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh(), FName("Head"));
	Camera->bUsePawnControlRotation = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 600.f;
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AShooterCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	check(ShooterContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShooterContext, 0);
		}
	}

	if (HasAuthority())
	{
		EquipWeapon();
	}
}

void AShooterCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShooterCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::MoveButtonReleased);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Jump);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::CrouchButtonPressed);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Dash);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Fire);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::FireButtonReleased);
	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Reload);
	EnhancedInputComponent->BindAction(SwapAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::SwapWeapons);
}

void AShooterCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void AShooterCharacterBase::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, InputAxisVector.Y);
	AddMovementInput(RightDirection, InputAxisVector.X);
	JumpDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X + FVector::UpVector;
	DashDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X;
}

void AShooterCharacterBase::MoveButtonReleased()
{
	if (Camera)
	{
		JumpDirection = FVector::UpVector;
		DashDirection = Camera->GetForwardVector();
		DashDirection.Z = 0.f;
	}
}

void AShooterCharacterBase::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AShooterCharacterBase::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
		return;
	}
		
	if (JumpCount >= 1 && JumpCount < MaxJumpCount && !CanJump())
	{
		if (JumpDirection == FVector::ZeroVector)
		{
			JumpDirection = FVector::UpVector;
		}
		const FVector JumpForce = JumpDirection * GetCharacterMovement()->JumpZVelocity;
		ServerLaunchCharacter(JumpForce);
		JumpCount++;
	}
	else if (CanJump())
	{
		Super::Jump();
		JumpCount = 1;
	}
}

void AShooterCharacterBase::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AShooterCharacterBase::Dash()
{
	if (Camera && DashCount < MaxDashCount)
	{
		SetCollisionBetweenCharacter(ECR_Ignore);
		
		if (GetVelocity() == FVector::ZeroVector)
		{
			DashDirection = Camera->GetForwardVector();
			DashDirection.Z = 0.f;
		}
		DashDirection.Normalize();
		DashDirection *= DashForce;
		ServerLaunchCharacter(DashDirection);

		FTimerHandle EnableCollisionHandle;
		FTimerDelegate EnabledCollisionDelegate;
		EnabledCollisionDelegate.BindUFunction(this, FName("SetCollisionBetweenCharacter"), ECR_Block);
		GetWorldTimerManager().SetTimer(EnableCollisionHandle, EnabledCollisionDelegate, 0.1f, false);

		if (GetCharacterMovement()->IsFalling())
		{
			FTimerHandle StopHandle;
			GetWorldTimerManager().SetTimer(StopHandle, this, &AShooterCharacterBase::DashFinished, 0.1f);
		}
		
		DashCount++;
		if (!GetWorldTimerManager().IsTimerActive(DashHandle))
		{
			GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterCharacterBase::DashReset, DashCoolTime);
		}
	}
}

void AShooterCharacterBase::ServerLaunchCharacter_Implementation(const FVector_NetQuantize& LaunchForce)
{
	LaunchCharacter(LaunchForce, true, true);
}

void AShooterCharacterBase::DashFinished()
{
	ServerDashFinished();
}

void AShooterCharacterBase::ServerDashFinished_Implementation()
{
	GetCharacterMovement()->StopMovementImmediately();
}

void AShooterCharacterBase::DashReset()
{
	DashCount--;
	if (DashCount > 0)
	{
		GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterCharacterBase::DashReset, DashCoolTime);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString(TEXT("Dash Filled!")));
	}
}

void AShooterCharacterBase::Fire()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AShooterCharacterBase::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AShooterCharacterBase::Reload()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void AShooterCharacterBase::SwapWeapons()
{
	if (Combat)
	{
		Combat->SwapWeapons();
	}
}

void AShooterCharacterBase::EquipWeapon()
{
	if (Combat == nullptr) return;
	
	if (PrimaryGunClass)
	{
		AWeapon* PrimaryGun = GetWorld()->SpawnActor<AWeapon>(PrimaryGunClass);
		if (PrimaryGun)
		{
			Combat->AttachWeaponToRightHand(PrimaryGun);
		}
	}
	if (SecondaryGunClass)
	{
		AWeapon* SecondaryGun = GetWorld()->SpawnActor<AWeapon>(SecondaryGunClass);
		if (SecondaryGun)
		{
			Combat->AttachWeaponToBackpack(SecondaryGun);
		}
	}
}

void AShooterCharacterBase::SetCollisionBetweenCharacter(const ECollisionResponse NewResponse)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, NewResponse);
	ServerSetCollisionBetweenCharacter(NewResponse);
}

void AShooterCharacterBase::ServerSetCollisionBetweenCharacter_Implementation(const ECollisionResponse NewResponse)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, NewResponse);
}
