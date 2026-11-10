
#include "PersonnageNettoyeur.h"
#include "Components/InputComponent.h"
#include "InteractionComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

APersonnageNettoyeur::APersonnageNettoyeur()
{
    PrimaryActorTick.bCanEverTick = false;

    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
    InteractionComponent->SetPhysicsHandle(PhysicsHandle);
}

void APersonnageNettoyeur::BeginPlay()
{
    Super::BeginPlay();
}

void APersonnageNettoyeur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APersonnageNettoyeur::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
void APersonnageNettoyeur::TenterNettoyage()
{
    // La logique du rayon sera ajoutée à l'étape suivante.
}

bool APersonnageNettoyeur::CanJumpInternal_Implementation() const
{
    return (!InteractionComponent || !InteractionComponent->IsHoldingObject())
        && Super::CanJumpInternal_Implementation();
}
