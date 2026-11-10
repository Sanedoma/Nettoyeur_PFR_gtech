#include "InteractionComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "InteractionProfileComponent.h"
#include "InteractionTypes.h"
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

FVector UInteractionComponent::GetAimDirection() const
{
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    const APlayerController* PlayerController = OwnerPawn
        ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

    if (PlayerController)
    {
        FVector ViewLocation;
        FRotator ViewRotation;
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
        return ViewRotation.Vector();
    }

    return GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
}

void UInteractionComponent::UpdateFocus()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    APlayerController* PlayerController = OwnerPawn
        ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    UWorld* World = GetWorld();

    if (!PlayerController || !World)
    {
        CurrentFocus = nullptr;
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PullFocus), false);
    QueryParams.AddIgnoredActor(GetOwner());

    FHitResult Hit;
    const bool bHit = World->SweepSingleByChannel(
        Hit,
        ViewLocation,
        TraceEnd,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(10.f),
        QueryParams);

    CurrentFocus = bHit ? Hit.GetActor() : nullptr;
}

UPrimitiveComponent* UInteractionComponent::GetFocusPrimitive() const
{
    return CurrentFocus
        ? Cast<UPrimitiveComponent>(CurrentFocus->GetRootComponent())
        : nullptr;
}

void UInteractionComponent::Pull()
{
    if (IsHoldingObject() || !PhysicsHandle || !CurrentFocus)
    {
        return;
    }

    const UInteractionProfileComponent* Profile =
        CurrentFocus->FindComponentByClass<UInteractionProfileComponent>();
    if (!Profile || !Profile->CanPerform(EInteractionType::Pull))
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
}

void UInteractionComponent::Grab()
{
    if (IsHoldingObject() || !PhysicsHandle || !CurrentFocus)
    {
        return;
    }

    const UInteractionProfileComponent* Profile =
        CurrentFocus->FindComponentByClass<UInteractionProfileComponent>();
    if (!Profile || !Profile->CanPerform(EInteractionType::Grab))
    {
        return;
    }

    UPrimitiveComponent* Primitive = GetFocusPrimitive();
    if (!Primitive || !Primitive->IsSimulatingPhysics())
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    APawn* OwnerPawn = Cast<APawn>(OwnerActor);
    APlayerController* PlayerController = OwnerPawn
        ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

    FVector ViewLocation = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
    FRotator ViewRotation = OwnerActor ? OwnerActor->GetActorRotation() : FRotator::ZeroRotator;
    if (PlayerController)
    {
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }

    const FVector DesiredLocation = ViewLocation + ViewRotation.Vector() * HoldDistance;
    HeldTargetRotation = Primitive->GetComponentRotation();
    HeldTargetLocation = DesiredLocation;
    bIsGrabbing = true;

    PhysicsHandle->GrabComponentAtLocationWithRotation(
        Primitive,
        NAME_None,
        Primitive->GetComponentLocation(),
        HeldTargetRotation);
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

    // Grab suit la position complète de la caméra, mais conserve la rotation
    // exacte de l'objet enregistrée au moment de la saisie.
    if (bIsGrabbing)
    {
        const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
        const APlayerController* PlayerController = OwnerPawn
            ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

        FVector ViewLocation = OwnerActor->GetActorLocation();
        FRotator ViewRotation = OwnerActor->GetActorRotation();
        if (PlayerController)
        {
            PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
        }

        const FVector DesiredLocation = ViewLocation + ViewRotation.Vector() * HoldDistance;
        HeldTargetLocation = FMath::VInterpTo(
            HeldTargetLocation, DesiredLocation, DeltaTime, HoldInterpSpeed);

        PhysicsHandle->SetTargetLocationAndRotation(HeldTargetLocation, HeldTargetRotation);
        return;
    }

    // Pull ne suit que le Yaw : le Pitch de la caméra ne soulève jamais l'objet.
    FRotator ViewRotation = OwnerActor->GetActorRotation();
    if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
    {
        if (const APlayerController* PlayerController =
            Cast<APlayerController>(OwnerPawn->GetController()))
        {
            FVector ViewLocation;
            PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
        }
    }

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
    if (PhysicsHandle && PhysicsHandle->GrabbedComponent)
    {
        PhysicsHandle->ReleaseComponent();
    }

    bIsGrabbing = false;
}

bool UInteractionComponent::IsHoldingObject() const
{
    return PhysicsHandle && PhysicsHandle->GrabbedComponent != nullptr;
}
