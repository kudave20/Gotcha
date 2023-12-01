// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AShooterCharacterBase;
class UCombatComponent;

/**
 * 
 */
UCLASS()
class GOTCHA_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCombatComponent> Combat;
	
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> ShooterContext;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY()
	AShooterCharacterBase* ControlledCharacter;

	void Move(const FInputActionValue& InputActionValue);
	void MoveReleased();
	void Look(const FInputActionValue& InputActionValue);
	void Jump();
	void Crouch();
	void Dash();

	UFUNCTION(Server, Reliable)
	void ServerLaunchCharacter(const FVector LaunchForce);

	FVector JumpDirection;
	int32 JumpCount = 0;

	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MaxJumpCount = 2;

	FVector DashDirection;
	int32 DashCount = 0;
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MaxDashCount = 3;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DashCoolTime = 3.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DashForce = 10000.f;

	UFUNCTION()
	void SetCollisionBetweenCharacter(const ECollisionResponse NewResponse);
	
	void DashFinished();

	UFUNCTION(Server, Reliable)
	void ServerDashFinished();

	FTimerHandle DashHandle;
	
	void DashReset();
};
