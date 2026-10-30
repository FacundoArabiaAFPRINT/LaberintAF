#include "LabProyectil.h"
#include "LabPersonaje.h"
#include "LabSalud.h"
#include "LabCharco.h"
#include "LabModoJuego.h"
#include "LabLaberinto.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"

ALabProyectil::ALabProyectil()
{
	PrimaryActorTick.bCanEverTick = true;
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	RootComponent = Malla;
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esfera(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Esfera.Succeeded()) Malla->SetStaticMesh(Esfera.Object);
	Malla->SetWorldScale3D(FVector(.18f));
	Luz = CreateDefaultSubobject<UPointLightComponent>(TEXT("Luz"));
	Luz->SetupAttachment(Malla);
	Luz->SetIntensity(800); Luz->SetAttenuationRadius(250); Luz->SetCastShadows(false);
}

ALabProyectil* ALabProyectil::Disparar(AActor* Duenio, float Yaw, float VelocidadCm, float Vida, float Danio, const FLabEfectos& Efectos, const FString& Flags, FLinearColor Color, AActor* Objetivo)
{
	if (!Duenio) return nullptr;
	const FVector Dir = FRotator(0, Yaw, 0).Vector();
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALabProyectil* A = Duenio->GetWorld()->SpawnActor<ALabProyectil>(ALabProyectil::StaticClass(), FTransform(Dir.Rotation(), Duenio->GetActorLocation() + Dir * 70), P);
	if (!A) return nullptr;
	A->Vel = Dir * VelocidadCm; A->Vida = Vida; A->Danio = Danio; A->Efectos = Efectos; A->Flags = Flags; A->Duenio = Duenio; A->Objetivo = Objetivo;
	if (UMaterialInterface* M = A->Malla->GetMaterial(0))
	{
		UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(M, A);
		D->SetVectorParameterValue(TEXT("Color"), Color);
		A->Malla->SetMaterial(0, D);
	}
	A->Luz->SetLightColor(Color);
	return A;
}

void ALabProyectil::Tick(float Dt)
{
	Super::Tick(Dt);
	Vida -= Dt;
	if (Tiene(TEXT("Persigue")) && Objetivo.IsValid())
	{
		const FVector Quiero = (Objetivo->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const float V = Vel.Size() + Lab::CmS(30) * Dt;
		Vel = FMath::VInterpNormalRotationTo(Vel.GetSafeNormal(), Quiero, Dt, 170.f) * V;
	}
	const FVector Nuevo = GetActorLocation() + Vel * Dt;
	ALabLaberinto* L = ALabModoJuego::Laberinto(this);
	if (L && L->EsSolido(Nuevo) && !Tiene(TEXT("Persigue"))) { Terminar(); return; }
	SetActorLocation(Nuevo);
	for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
	{
		ALabPersonaje* P = *It;
		if (P == Duenio.Get() || !P->Salud || !P->Salud->Vivo() || P->bEscapo) continue;
		if (FVector::Dist2D(P->GetActorLocation(), Nuevo) > 55) continue;
		if (P->Salud->RecibirGolpe(Danio, Duenio.Get(), ELabGolpe::Distancia, Efectos) && Tiene(TEXT("Gancho")) && Duenio.IsValid())
		{
			// el gancho te arrastra hasta el Carnicero
			const FVector Hacia = Duenio->GetActorLocation() + (P->GetActorLocation() - Duenio->GetActorLocation()).GetSafeNormal2D() * 110;
			P->SetActorLocation(FVector(Hacia.X, Hacia.Y, P->GetActorLocation().Z), true);
			P->Salud->Aturdido = FMath::Max(P->Salud->Aturdido, .5f);
		}
		Terminar();
		return;
	}
	if (Vida <= 0) Terminar();
}

void ALabProyectil::Terminar()
{
	const FVector Aca = GetActorLocation() - Vel.GetSafeNormal() * 25;
	if (Tiene(TEXT("Charco"))) ALabCharco::Crear(GetWorld(), ELabCharco::Veneno, Aca, Lab::Cm(12), 4, Duenio.Get());
	if (Tiene(TEXT("Tela"))) ALabCharco::Crear(GetWorld(), ELabCharco::Tela, Aca, Lab::Cm(11), 7, Duenio.Get());
	Destroy();
}
