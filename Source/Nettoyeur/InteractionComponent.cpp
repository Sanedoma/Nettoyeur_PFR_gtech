#include "InteractionComponent.h"

#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "InteractionProfileComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::SetPhysicsHandle(UPhysicsHandleComponent* InPhysicsHandle)
{
    PhysicsHandle = InPhysicsHandle;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateFocus();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateFocus();
    UpdateHold(DeltaTime);
}

bool UInteractionComponent::GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
    const AActor* OwnerActor = GetOwner();
    OutLocation = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
    OutRotation = OwnerActor ? OwnerActor->GetActorRotation() : FRotator::ZeroRotator;

    const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
    const APlayerController* PlayerController = OwnerPawn
        ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    if (!PlayerController)
    {
        return false;
    }

    PlayerController->GetPlayerViewPoint(OutLocation, OutRotation);
    return true;
}

void UInteractionComponent::UpdateFocus()
{
    UWorld* World = GetWorld();
    FVector ViewLocation;
    FRotator ViewRotation;

    // Sans PlayerController (personnage pas encore possédé) on ne vise rien.
    if (!World || !GetViewPoint(ViewLocation, ViewRotation))
    {
        SetCurrentFocus(nullptr);
        return;
    }

    const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InteractionFocus), false);
    QueryParams.AddIgnoredActor(GetOwner());

    // L'UNIQUE trace de visée du projet : tous les systèmes lisent son résultat.
    FHitResult Hit;
    const bool bHit = World->SweepSingleByChannel(
        Hit,
        ViewLocation,
        TraceEnd,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(10.f),
        QueryParams);

    if (bAfficherDebug)
    {
        // Durée 0 = une image : appelé à chaque Tick, le trait suit la visée.
        DrawDebugLine(World, ViewLocation, bHit ? Hit.Location : TraceEnd,
            bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.5f);
        if (bHit)
        {
            DrawDebugSphere(World, Hit.Location, 10.f, 8, FColor::Green, false, 0.f);
        }
    }

    SetCurrentFocus(bHit ? Hit.GetActor() : nullptr);
}

void UInteractionComponent::SetCurrentFocus(AActor* NewFocus)
{
    if (NewFocus == CurrentFocus)
    {
        return;
    }

    // On ne prévient les abonnés que lorsque la cible change vraiment,
    // pas à chaque image.
    CurrentFocus = NewFocus;
    OnFocusChanged.Broadcast(NewFocus);
}

AActor* UInteractionComponent::GetCurrentFocus() const
{
    return CurrentFocus;
}

bool UInteractionComponent::CanFocusPerform(EInteractionType Interaction) const
{
    if (!CurrentFocus)
    {
        return false;
    }

    const UInteractionProfileComponent* Profile =
        CurrentFocus->FindComponentByClass<UInteractionProfileComponent>();
    return Profile && Profile->CanPerform(Interaction);
}

UPrimitiveComponent* UInteractionComponent::GetFocusPrimitive() const
{
    return CurrentFocus
        ? Cast<UPrimitiveComponent>(CurrentFocus->GetRootComponent())
        : nullptr;
}

void UInteractionComponent::Pull()
{
    if (IsHoldingObject() || !PhysicsHandle || !CanFocusPerform(EInteractionType::Pull))
    {
        return;
    }

    UPrimitiveComponent* Primitive = GetFocusPrimitive();
    if (!Primitive || !Primitive->IsSimulatingPhysics())
    {
        return;
    }

    PhysicsHandle->GrabComponentAtLocationWithRotation(
        Primitive,
        NAME_None,
        Primitive->GetComponentLocation(),
        Primitive->GetComponentRotation());

    bIsGrabbing = false;

    // Pull conserve la hauteur et l'orientation initiales de l'objet :
    // contrairement à Grab, l'objet ne suit pas le Pitch de la caméra.
    HeldTargetLocation = Primitive->GetComponentLocation();
    HeldTargetRotation = Primitive->GetComponentRotation();

    if (IsHoldingObject())
    {
        OnHoldChanged.Broadcast(true);
    }
}

void UInteractionComponent::Grab()
{
    if (IsHoldingObject() || !PhysicsHandle || !CanFocusPerform(EInteractionType::Grab))
    {
        return;
    }

    UPrimitiveComponent* Primitive = GetFocusPrimitive();
    if (!Primitive || !Primitive->IsSimulatingPhysics())
    {
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    const FVector DesiredLocation = ViewLocation + ViewRotation.Vector() * HoldDistance;
    HeldTargetRotation = Primitive->GetComponentRotation();
    HeldTargetLocation = DesiredLocation;
    bIsGrabbing = true;

    PhysicsHandle->GrabComponentAtLocationWithRotation(
        Primitive,
        NAME_None,
        Primitive->GetComponentLocation(),
        HeldTargetRotation);

    if (IsHoldingObject())
    {
        OnHoldChanged.Broadcast(true);
    }
}

void UInteractionComponent::UpdateHold(float DeltaTime)
{
    if (!PhysicsHandle || !PhysicsHandle->GrabbedComponent)
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    UPrimitiveComponent* HeldComponent = PhysicsHandle->GrabbedComponent;
    if (!OwnerActor || !HeldComponent)
    {
        ReleaseGrab();
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    // Grab suit la position complète de la caméra, mais conserve la rotation
    // exacte de l'objet enregistrée au moment de la saisie.
    if (bIsGrabbing)
    {
        const FVector DesiredLocation = ViewLocation + ViewRotation.Vector() * HoldDistance;
        HeldTargetLocation = FMath::VInterpTo(
            HeldTargetLocation, DesiredLocation, DeltaTime, HoldInterpSpeed);

        PhysicsHandle->SetTargetLocationAndRotation(HeldTargetLocation, HeldTargetRotation);
        return;
    }

    // Pull ne suit que le Yaw : le Pitch de la caméra ne soulève jamais l'objet.
    const FVector HorizontalForward = FRotator(0.f, ViewRotation.Yaw, 0.f).Vector();
    const FVector OwnerLocation = OwnerActor->GetActorLocation();
    FVector DesiredLocation = OwnerLocation + HorizontalForward * HoldDistance;

    // Cherche le sol SOUS la position voulue à chaque frame : la hauteur peut
    // donc changer quand le joueur monte ou descend une plateforme.
    UWorld* World = GetWorld();
    if (!World)
    {
        ReleaseGrab();
        return;
    }

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PullGround), false);
    QueryParams.AddIgnoredActor(OwnerActor);
    QueryParams.AddIgnoredActor(HeldComponent->GetOwner());

    const FVector TraceStart = DesiredLocation + FVector(0.f, 0.f, 500.f);
    const FVector TraceEnd = DesiredLocation - FVector(0.f, 0.f, 1500.f);
    TArray<FHitResult> GroundHits;
    World->LineTraceMultiByChannel(
        GroundHits, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

    bool bFoundGround = false;
    for (const FHitResult& GroundHit : GroundHits)
    {
        const UPrimitiveComponent* HitPrimitive = GroundHit.GetComponent();
        if (GroundHit.bBlockingHit && HitPrimitive &&
            !HitPrimitive->IsSimulatingPhysics())
        {
            const float HalfHeight = HeldComponent->Bounds.BoxExtent.Z;
            DesiredLocation.Z = GroundHit.ImpactPoint.Z + HalfHeight + 2.f;
            bFoundGround = true;
            break;
        }
    }

    // Au bord d'un vide, on lâche l'objet : il tombe avec la gravité au lieu
    // de rester suspendu à une hauteur mémorisée.
    if (!bFoundGround)
    {
        ReleaseGrab();
        return;
    }

    HeldTargetLocation = FMath::VInterpTo(
        HeldTargetLocation, DesiredLocation, DeltaTime, HoldInterpSpeed);
    HeldTargetLocation.Z = FMath::FInterpTo(
        HeldTargetLocation.Z, DesiredLocation.Z, DeltaTime, HoldInterpSpeed);

    // L'orientation reste celle de la saisie ; seul le déplacement au sol change.
    PhysicsHandle->SetTargetLocationAndRotation(HeldTargetLocation, HeldTargetRotation);
}

void UInteractionComponent::ReleaseGrab()
{
    // ReleaseGrab est aussi appelé quand on relâche la touche sans rien tenir :
    // on ne prévient les abonnés que si un objet était vraiment tenu.
    const bool bWasHolding = IsHoldingObject();
    if (bWasHolding)
    {
        PhysicsHandle->ReleaseComponent();
    }

    bIsGrabbing = false;

    if (bWasHolding)
    {
        OnHoldChanged.Broadcast(false);
    }
}

bool UInteractionComponent::IsHoldingObject() const
{
    return PhysicsHandle && PhysicsHandle->GrabbedComponent != nullptr;
}
