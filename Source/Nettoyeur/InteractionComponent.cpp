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

    // Secours (dette n° 12) : si personne n'a fourni de Physics Handle (composant ajouté
    // à la main sur un autre acteur), on prend celui de l'acteur propriétaire.
    if (!PhysicsHandle && GetOwner())
    {
        PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
    }
    if (!PhysicsHandle)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("InteractionComponent : aucun Physics Handle sur %s, Pull / Grab / Launch sont desactives."),
            *GetNameSafe(GetOwner()));
    }

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
        CurrentFocusComponent = nullptr;
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

    // Dette n° 8 : on garde aussi le composant précis touché, pour saisir le bon mesh
    // quand la racine de l'acteur n'est pas l'objet physique.
    CurrentFocusComponent = bHit ? Hit.GetComponent() : nullptr;
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

bool UInteractionComponent::ActorAllows(const AActor* Actor, EInteractionType Interaction)
{
    if (!Actor)
    {
        return false;
    }

    const UInteractionProfileComponent* Profile =
        Actor->FindComponentByClass<UInteractionProfileComponent>();
    return Profile && Profile->CanPerform(Interaction);
}

bool UInteractionComponent::CanFocusPerform(EInteractionType Interaction) const
{
    return ActorAllows(CurrentFocus, Interaction);
}

UPrimitiveComponent* UInteractionComponent::GetFocusPrimitive() const
{
    if (!CurrentFocus)
    {
        return nullptr;
    }

    // 1. Le composant réellement touché, s'il appartient bien à la cible et simule la physique.
    if (IsValid(CurrentFocusComponent) && CurrentFocusComponent->GetOwner() == CurrentFocus
        && CurrentFocusComponent->IsSimulatingPhysics())
    {
        return CurrentFocusComponent;
    }

    // 2. Sinon la racine, comme avant T002 (Pull / Grab vérifient ensuite qu'elle simule).
    return Cast<UPrimitiveComponent>(CurrentFocus->GetRootComponent());
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

    // Distance entre le point tenu (l'origine du composant) et le bas de l'objet
    // (dette n° 11) : juste si l'origine du mesh n'est pas au centre (pivot à la base…).
    const FBoxSphereBounds& HeldBounds = HeldComponent->Bounds;
    const double OriginToBottom =
        HeldComponent->GetComponentLocation().Z - (HeldBounds.Origin.Z - HeldBounds.BoxExtent.Z);

    // Dette n° 9 : un Line Trace s'arrête au premier objet bloquant. Si c'est un objet
    // physique (une autre caisse), on l'ignore et on relance, jusqu'à trouver le sol
    // (bloquant et non simulé) ou ne plus rien toucher.
    constexpr int32 MaxGroundTraces = 8;
    bool bFoundGround = false;
    for (int32 TraceIndex = 0; TraceIndex < MaxGroundTraces; ++TraceIndex)
    {
        FHitResult GroundHit;
        if (!World->LineTraceSingleByChannel(
            GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            break;
        }

        UPrimitiveComponent* HitPrimitive = GroundHit.GetComponent();
        if (HitPrimitive && HitPrimitive->IsSimulatingPhysics())
        {
            QueryParams.AddIgnoredComponent(HitPrimitive);
            continue;
        }

        DesiredLocation.Z = GroundHit.ImpactPoint.Z + OriginToBottom + 2.f;
        bFoundGround = true;
        break;
    }

    // Au bord d'un vide, on lâche l'objet : il tombe avec la gravité au lieu
    // de rester suspendu à une hauteur mémorisée.
    if (!bFoundGround)
    {
        ReleaseGrab();
        return;
    }

    // Un seul lissage pour X, Y et Z (dette n° 10 : Z était lissé une deuxième fois).
    HeldTargetLocation = FMath::VInterpTo(
        HeldTargetLocation, DesiredLocation, DeltaTime, HoldInterpSpeed);

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

void UInteractionComponent::Launch()
{
    // Rien n'est tenu, ou l'objet est tiré en Pull : on ne fait rien.
    if (!IsHoldingObject() || !bIsGrabbing)
    {
        return;
    }

    // Le profil lu est celui de l'objet TENU (pas de la cible visée, qui peut différer).
    UPrimitiveComponent* HeldComponent = PhysicsHandle->GrabbedComponent;
    if (!HeldComponent || !ActorAllows(HeldComponent->GetOwner(), EInteractionType::Launch))
    {
        // Pas de Launch dans le profil : l'objet reste tenu (choix documenté T002).
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);
    const FVector LaunchDirection = ViewRotation.Vector();

    // Relâcher par le chemin normal : OnHoldChanged(false) part, le prompt réapparaît.
    ReleaseGrab();

    // Vitesse de départ = impulsion / masse, plafonnée : un objet lourd part moins vite,
    // un objet très léger ne dépasse jamais MaxLaunchSpeed.
    const float Mass = HeldComponent->GetMass();
    const float LaunchSpeed = Mass > KINDA_SMALL_NUMBER
        ? FMath::Min(LaunchImpulse / Mass, MaxLaunchSpeed)
        : MaxLaunchSpeed;

    // Détection de collision continue : à grande vitesse, l'objet ne peut pas « sauter »
    // par-dessus un mur fin entre deux pas de simulation.
    HeldComponent->SetUseCCD(true);

    // bVelChange = true : la valeur passée est directement un changement de vitesse (cm/s),
    // la masse a déjà été prise en compte au-dessus.
    HeldComponent->AddImpulse(LaunchDirection * LaunchSpeed, NAME_None, true);
}

bool UInteractionComponent::IsHoldingObject() const
{
    return PhysicsHandle && PhysicsHandle->GrabbedComponent != nullptr;
}
