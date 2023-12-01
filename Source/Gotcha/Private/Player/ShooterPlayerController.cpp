// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterPlayerController::AShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(ShooterContext);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(ShooterContext, 0);
	}
}

void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AShooterPlayerController::MoveReleased);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Jump);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Crouch);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Dash);
}

void AShooterPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter)
	{
		ControlledCharacter->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledCharacter->AddMovementInput(RightDirection, InputAxisVector.X);
		JumpDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X + FVector::UpVector;
		DashDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X;
	}
}

void AShooterPlayerController::MoveReleased()
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter && ControlledCharacter->GetCamera())
	{
		JumpDirection = FVector::UpVector;
		DashDirection = ControlledCharacter->GetCamera()->GetForwardVector();
		DashDirection.Z = 0.f;
	}
}

void AShooterPlayerController::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	
	AddPitchInput(LookAxisVector.Y);
	AddYawInput(LookAxisVector.X);
}

void AShooterPlayerController::Jump()
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter)
	{
		if (ControlledCharacter->bIsCrouched)
		{
			ControlledCharacter->UnCrouch();
			return;
		}
		
		if (JumpCount >= 1 && JumpCount < MaxJumpCount && !ControlledCharacter->CanJump() && ControlledCharacter->GetCharacterMovement())
		{
			if (JumpDirection == FVector::ZeroVector)
			{
				JumpDirection = FVector::UpVector;
			}
			const FVector JumpForce = JumpDirection * ControlledCharacter->GetCharacterMovement()->JumpZVelocity;
			ServerLaunchCharacter(JumpForce);
			JumpCount++;
		}
		else if (ControlledCharacter->CanJump())
		{
			ControlledCharacter->Jump();
			JumpCount = 1;
		}
	}
}

void AShooterPlayerController::Crouch()
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter)
	{
		if (ControlledCharacter->bIsCrouched)
		{
			ControlledCharacter->UnCrouch();
		}
		else
		{
			ControlledCharacter->Crouch();
		}
	}
}

void AShooterPlayerController::Dash()
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter && ControlledCharacter->GetCamera() && ControlledCharacter->GetCharacterMovement() && DashCount < MaxDashCount)
	{
		SetCollisionBetweenCharacter(ECR_Ignore);
		
		if (ControlledCharacter->GetVelocity() == FVector::ZeroVector)
		{
			DashDirection = ControlledCharacter->GetCamera()->GetForwardVector();
			DashDirection.Z = 0.f;
		}
		DashDirection.Normalize();
		DashDirection *= DashForce;
		ServerLaunchCharacter(DashDirection);

		FTimerHandle EnableCollisionHandle;
		FTimerDelegate EnabledCollisionDelegate;
		EnabledCollisionDelegate.BindUFunction(this, FName("SetCollisionBetweenCharacter"), ECR_Block);
		GetWorldTimerManager().SetTimer(EnableCollisionHandle, EnabledCollisionDelegate, 0.1f, false);

		if (ControlledCharacter->GetCharacterMovement()->IsFalling())
		{
			FTimerHandle StopHandle;
			GetWorldTimerManager().SetTimer(StopHandle, this, &AShooterPlayerController::DashFinished, 0.1f);
		}
		
		DashCount++;
		if (!GetWorldTimerManager().IsTimerActive(DashHandle))
		{
			GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterPlayerController::DashReset, DashCoolTime);
		}
	}
}

void AShooterPlayerController::SetCollisionBetweenCharacter(const ECollisionResponse NewResponse)
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter)
	{
		ControlledCharacter->SetCollisionBetweenCharacter(NewResponse);
	}
}

void AShooterPlayerController::DashFinished()
{
	ServerDashFinished();
}

void AShooterPlayerController::ServerDashFinished_Implementation()
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter && ControlledCharacter->GetCharacterMovement())
	{
		ControlledCharacter->GetCharacterMovement()->StopMovementImmediately();
	}
}

void AShooterPlayerController::DashReset()
{
	DashCount--;
	if (DashCount > 0)
	{
		GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterPlayerController::DashReset, DashCoolTime);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString(TEXT("Dash Filled!")));
	}
}

void AShooterPlayerController::ServerLaunchCharacter_Implementation(const FVector LaunchForce)
{
	ControlledCharacter = ControlledCharacter == nullptr ? GetPawn<AShooterCharacterBase>() : ControlledCharacter;
	if (ControlledCharacter)
	{
		ControlledCharacter->LaunchCharacter(LaunchForce, true, true);
	}
}
