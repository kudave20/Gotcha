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
class AShooterPlayerController;
class AGotchaGameMode;
class UAnimMontage;
class USphereComponent;
class UCableComponent;
class UMotionWarpingComponent;

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
	
	void Elim(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	
	void PlayFireMontage();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	void PollInit();

	void DestroyWeapons();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<UCombatComponent> Combat;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> AssistArea;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCableComponent> Hook;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

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

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ParryAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> GrappleAction;

	void Move(const FInputActionValue& InputActionValue);
	void MoveButtonReleased();
	void Look(const FInputActionValue& InputActionValue);
	virtual void Jump() override;
	void JumpButtonReleased();
	void CrouchButtonPressed();
	void Dash();
	void Fire();
	void FireButtonReleased();
	void Reload();
	void SwapWeapons();
	void Parry();
	void Grapple();

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

	UFUNCTION()
	void SetCollisionBetweenCharacter(const ECollisionResponse NewResponse);
	UFUNCTION(Server, Reliable)
	void ServerSetCollisionBetweenCharacter(const ECollisionResponse NewResponse);

	UPROPERTY()
	AGotchaGameMode* GotchaGameMode;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	AShooterPlayerController* ShooterPlayerController;

	void UpdateHUDHealth();

	bool bElimmed;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Properties")
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	bool bLeftGame;

	UPROPERTY(Replicated)
	bool bDisableGameplay;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> FireWeaponMontage;
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	float ParryTime = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float HookLength = 3000.f;

	UPROPERTY(Replicated)
	FVector_NetQuantize GrabPoint;

	UPROPERTY(Replicated)
	bool bIsGrappling;
	
	void CheckGrapple();
	void DoGrapple();

	UFUNCTION(Server, Reliable)
	void ServerGrapple();
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	float GrappleForce = 250000.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float GrappleCoolTime = 6.f;

	UPROPERTY(Replicated)
	bool bCanGrapple = true;

	void GrappleFinished();
	
	bool bJumpButtonHeld;

	UFUNCTION(Server, Reliable)
	void ServerHoldJumpButton();
	UFUNCTION(Server, Reliable)
	void ServerReleaseJumpButton();
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	float MantleLength = 150.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float MantleHeight = 60.f;

	void CheckMantle();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDoMantle(const FVector_NetQuantize& FirstPoint, const FVector_NetQuantize& SecondPoint);

	UPROPERTY(Replicated)
	bool bIsMantling;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> MantleMontage;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float MantleMontageLength = 1.1f;

	void MantleFinished();
	
public:	
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE float GetParryTime() const { return ParryTime; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
