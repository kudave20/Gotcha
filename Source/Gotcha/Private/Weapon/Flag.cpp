// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"
#include "Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetupAttachment(FlagMesh);
	
	PickupArea = CreateDefaultSubobject<USphereComponent>(TEXT("PickupArea"));
	PickupArea->SetupAttachment(FlagMesh);
	PickupArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupArea->SetCollisionObjectType(ECC_WorldDynamic);
	PickupArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bCanFire = false;
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PickupArea->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnSphereOverlap);
	}
}

void AFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(OtherActor);
	if (ShooterCharacter)
	{
		ShooterCharacter->HoldFlag(this);
	}
}
