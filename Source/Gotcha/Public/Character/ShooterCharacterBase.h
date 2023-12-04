// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacterBase.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UCameraComponent;
class AWeapon;
class UCombatComponent;

UCLASS()
class GOTCHA_API AShooterCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> Camera;
	
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

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> SwapAction;

	void Move(const FInputActionValue& InputActionValue);
	void MoveButtonReleased();
	void Look(const FInputActionValue& InputActionValue);
	virtual void Jump() override;
	void CrouchButtonPressed();
	void Dash();
	void Fire();
	void FireButtonReleased();
	void Reload();
	void SwapWeapons();

	UPROPERTY(EditAnywhere, Category = "Properties")
	TSubclassOf<AWeapon> PrimaryGunClass;

	UPROPERTY(EditAnywhere, Category = "Properties")
	TSubclassOf<AWeapon> SecondaryGunClass;

	void EquipWeapon();

	UFUNCTION(Server, Reliable)
	void ServerLaunchCharacter(const FVector_NetQuantize& LaunchForce);

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

	FTimerHandle DashHandle;

	void DashFinished();

	UFUNCTION(Server, Reliable)
	void ServerDashFinished();
	
	void DashReset();
	
	void SetCollisionBetweenCharacter(const ECollisionResponse NewResponse);
	
	UFUNCTION(Server, Reliable)
	void ServerSetCollisionBetweenCharacter(const ECollisionResponse NewResponse);

	UPROPERTY(EditAnywhere, Category = "Properties")
	float MaxHealth = 100.f;

	UPROPERTY(Replicated)
	float Health = 100.f;
	
public:	
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }
};
