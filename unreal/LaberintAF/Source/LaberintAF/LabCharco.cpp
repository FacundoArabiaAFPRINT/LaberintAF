#include "LabCharco.h"
#include "LabPersonaje.h"
#include "LabSalud.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"

ALabCharco::ALabCharco()
{
	PrimaryActorTick.bCanEverTick = true;
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	RootComponent = Malla;
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cil(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cil.Succeeded()) Malla->SetStaticMesh(Cil.Object);
	Luz = CreateDefaultSubobject<UPointLightComponent>(TEXT("Luz"));
	Luz->SetupAttachment(Malla);
	Luz->SetIntensity(0); Luz->SetCastShadows(false);
}

ALabCharco* ALabCharco::Crear(UWorld* Mundo, ELabCharco Tipo, const FVector& Lugar, float RadioCm, float Duracion, AActor* Duenio)
{
	if (!Mundo) return nullptr;
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALabCharco* C = Mundo->SpawnActor<ALabCharco>(ALabCharco::StaticClass(), FTransform(FVector(Lugar.X, Lugar.Y, 1)), P);
	if (!C) return nullptr;
	C->Tipo = Tipo; C->Radio = RadioCm; C->Duracion = Duracion; C->Duenio = Duenio;
	C->SetActorScale3D(FVector(RadioCm / 50.f, RadioCm / 50.f, Tipo == ELabCharco::Cepo ? .08f : .02f));
	const FLinearColor Col = Tipo == ELabCharco::Veneno ? FLinearColor(.25f, .55f, .1f) : Tipo == ELabCharco::Fuego ? FLinearColor(1, .35f, .05f) : Tipo == ELabCharco::Tela ? FLinearColor(.85f, .85f, .8f) : FLinearColor(.35f, .35f, .4f);
	if (UMaterialInterface* M = C->Malla->GetMaterial(0))
	{
		UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(M, C);
		D->SetVectorParameterValue(TEXT("Color"), Col);
		C->Malla->SetMaterial(0, D);
	}
	if (Tipo == ELabCharco::Fuego) { C->Luz->SetIntensity(3000); C->Luz->SetLightColor(Col); C->Luz->SetAttenuationRadius(RadioCm * 3); }
	return C;
}

void ALabCharco::Tick(float Dt)
{
	Super::Tick(Dt);
	Duracion -= Dt;
	if (Duracion <= 0) { Destroy(); return; }
	for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
	{
		ALabPersonaje* P = *It;
		if (!P->Salud || !P->Salud->Vivo() || P->bEscapo) continue;
		if (FVector::Dist2D(P->GetActorLocation(), GetActorLocation()) > Radio) continue;
		switch (Tipo)
		{
		case ELabCharco::Veneno: P->Salud->Veneno = FMath::Max(P->Salud->Veneno, 1.2f); break;
		case ELabCharco::Fuego: P->Salud->Fuego = FMath::Max(P->Salud->Fuego, 1.5f); P->Salud->RecibirGolpe(12 * Dt, Duenio.Get(), ELabGolpe::Otro, FLabEfectos()); break;
		case ELabCharco::Tela: P->Salud->Lento = FMath::Max(P->Salud->Lento, .4f); break;
		case ELabCharco::Cepo:
			if (bArmado && P->Salud->Invulnerable <= 0)
			{
				bArmado = false; Duracion = 4;
				FLabEfectos E; E.Enreda = 2.2f; E.Sangra = 3;
				P->Salud->RecibirGolpe(10, Duenio.Get(), ELabGolpe::Filo, E);
				P->Mensaje(TEXT("¡Pisaste un cepo de Roberto!"));
			}
			break;
		}
	}
}
