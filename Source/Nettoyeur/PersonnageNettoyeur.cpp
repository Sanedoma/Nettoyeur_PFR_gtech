#include "PersonnageNettoyeur.h"

#include "InteractionComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

APersonnageNettoyeur::APersonnageNettoyeur()
{
    PrimaryActorTick.bCanEverTick = false;

    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
    InteractionComponent->SetPhysicsHandle(PhysicsHandle);
}

bool APersonnageNettoyeur::CanJumpInternal_Implementation() const
{
    return (!InteractionComponent || !InteractionComponent->IsHoldingObject())
        && Super::CanJumpInternal_Implementation();
}
