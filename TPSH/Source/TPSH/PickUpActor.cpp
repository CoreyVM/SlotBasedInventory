// Fill out your copyright notice in the Description page of Project Settings.
//Corey Veitch-McAllister 2019
#include "PickUpActor.h"
#include "TPSHCharacter.h"
#include "Engine.h"

// Sets default values
APickUpActor::APickUpActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	buttonPressed = false;
	this->SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	this->RootComponent = (SceneComponent);

	this->ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	this->ItemMesh->AttachTo(this->RootComponent);

	//Create Collider
	this->boxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	this->boxCollider->SetGenerateOverlapEvents(true);
	this->boxCollider->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
    this->boxCollider->AttachToComponent(this->RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	this->pickedUp = false;
	this->canDiscard = false;
	this->canCombine = false;
	this->combineChecked = false;
	this->ZoomInMin = 60;
	this->ZoomInMax = 75;
	this->canUse = true;
	this->itemDesc = "Nothing";
}

 void APickUpActor::BeginPlay() {
	Super::BeginPlay();
}


void APickUpActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
void APickUpActor::Show(bool visible) {
	ECollisionEnabled::Type collision = visible ? ECollisionEnabled::QueryAndPhysics : 
		ECollisionEnabled::NoCollision;
	this->ItemMesh->SetVisibility(visible); //USe this-> initially
	this->ItemMesh->SetCollisionEnabled(collision);
	this->SetActorTickEnabled(visible);
	this->boxCollider->SetCollisionEnabled(collision);
}


void APickUpActor::OnInteract() {
	FString pickup = FString::Printf(TEXT("Picked Up: %s"), *Name);
	ATPSHCharacter* player = Cast<ATPSHCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (player)
		player->AddToInventory(this);
}

void APickUpActor::AddItemToPlayerInventory(APickUpActor* item) {
	ATPSHCharacter* player = Cast<ATPSHCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	this->AttachToActor(player, FAttachmentTransformRules::KeepRelativeTransform); //Attaches the actor to the player 
	Show(false);
}

void APickUpActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		//OnInteract();
	}
}
