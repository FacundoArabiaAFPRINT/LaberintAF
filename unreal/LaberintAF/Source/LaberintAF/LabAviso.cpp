#include "LabAviso.h"
#include "LabPersonaje.h"
#include "LabSalud.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"

ALabAviso::ALabAviso()
{
	PrimaryActorTick.bCanEverTick = true;
	Malla = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Malla"));
	RootComponent = Malla;
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Malla->SetCastShadow(false);
	// material provisorio del motor (opaco); para el aspecto final se pone un material de calcomanía rojo translúcido
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (Mat.Succeeded()) Material = Mat.Object;
}

ALabAviso* ALabAviso::Crear(UWorld* Mundo, ELabFormaAviso Forma, const FVector& Lugar, float Yaw, float Radio, float Largo, float Ancho, float ArcoRad,
	float Duracion, FLinearColor Color, AActor* Duenio, TFunction<void(ALabAviso*)> AlTerminar)
{
	if (!Mundo) return nullptr;
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALabAviso* A = Mundo->SpawnActor<ALabAviso>(ALabAviso::StaticClass(), FTransform(FRotator(0, Yaw, 0), Lugar + FVector(0, 0, 2)), P);
	if (!A) return nullptr;
	A->Forma = Forma; A->Radio = Radio; A->Largo = Largo; A->Ancho = Ancho; A->Arco = ArcoRad;
	A->Duracion = FMath::Max(.01f, Duracion); A->Color = Color; A->Duenio = Duenio; A->AlTerminar = MoveTemp(AlTerminar);
	if (A->Material && Forma != ELabFormaAviso::Nada)
	{
		A->MatFondo = UMaterialInstanceDynamic::Create(A->Material, A);
		A->MatFondo->SetVectorParameterValue(TEXT("Color"), Color * .35f);
		A->MatLleno = UMaterialInstanceDynamic::Create(A->Material, A);
		A->MatLleno->SetVectorParameterValue(TEXT("Color"), Color);
	}
	A->Dibujar(0);
	return A;
}

void ALabAviso::Despues(UWorld* Mundo, float Segundos, AActor* Duenio, TFunction<void(ALabAviso*)> Hacer)
{
	Crear(Mundo, ELabFormaAviso::Nada, FVector::ZeroVector, 0, 0, 0, 0, 0, Segundos, FLinearColor::Black, Duenio, MoveTemp(Hacer));
}

void ALabAviso::Forma2D(float K, TArray<FVector>& V, TArray<int32>& Tri) const
{
	V.Reset(); Tri.Reset();
	if (Forma == ELabFormaAviso::Linea)
	{
		const float L = Largo * K, A = Ancho / 2;
		V = {FVector(0, -A, 0), FVector(L, -A, 0), FVector(L, A, 0), FVector(0, A, 0)};
		Tri = {0, 2, 1, 0, 3, 2};
		return;
	}
	const int32 N = 32;
	const float R = Radio * (Forma == ELabFormaAviso::Cono ? K : K), A0 = Forma == ELabFormaAviso::Cono ? -Arco / 2 : 0, AA = Forma == ELabFormaAviso::Cono ? Arco : 2 * PI;
	V.Add(FVector::ZeroVector);
	for (int32 i = 0; i <= N; ++i) { const float a = A0 + AA * i / N; V.Add(FVector(FMath::Cos(a) * R, FMath::Sin(a) * R, 0)); }
	for (int32 i = 1; i <= N; ++i) { Tri.Add(0); Tri.Add(i + 1); Tri.Add(i); }
}

void ALabAviso::Dibujar(float Progreso)
{
	if (Forma == ELabFormaAviso::Nada) return;
	TArray<FVector> V; TArray<int32> Tri; TArray<FVector> Nor; TArray<FVector2D> UV; TArray<FLinearColor> Col; TArray<FProcMeshTangent> Tan;
	Forma2D(1, V, Tri); Nor.Init(FVector::UpVector, V.Num());
	Malla->CreateMeshSection_LinearColor(0, V, Tri, Nor, UV, Col, Tan, false);
	Forma2D(FMath::Clamp(Progreso, .02f, 1.f), V, Tri);
	for (FVector& P : V) P.Z += 1;
	Nor.Init(FVector::UpVector, V.Num());
	Malla->CreateMeshSection_LinearColor(1, V, Tri, Nor, UV, Col, Tan, false);
	if (MatFondo) Malla->SetMaterial(0, MatFondo);
	if (MatLleno) Malla->SetMaterial(1, MatLleno);
}

void ALabAviso::Tick(float Dt)
{
	Super::Tick(Dt);
	if (Seguir.IsValid()) SetActorLocation(Seguir->GetActorLocation() * FVector(1, 1, 0) + FVector(0, 0, GetActorLocation().Z));
	if (Rastrear.IsValid() && Duracion - T > FijarAntes)
	{
		const FVector D = Rastrear->GetActorLocation() - GetActorLocation();
		SetActorRotation(FRotator(0, D.Rotation().Yaw, 0));
		if (Duenio.IsValid()) Duenio->SetActorRotation(FRotator(0, D.Rotation().Yaw, 0));
	}
	T += Dt;
	Dibujar(T / Duracion);
	if (T >= Duracion)
	{
		// si el que lo tiró murió en el medio, no pega
		bool DuenioVivo = true;
		if (Duenio.IsValid()) if (ULabSalud* S = Duenio->FindComponentByClass<ULabSalud>()) DuenioVivo = S->Vivo();
		if (DuenioVivo && !Duenio.IsStale() && AlTerminar) AlTerminar(this);
		Destroy();
	}
}

bool ALabAviso::Toca(const FVector& P, float R) const
{
	const FVector L = GetActorTransform().InverseTransformPosition(P);
	switch (Forma)
	{
	case ELabFormaAviso::Circulo: return FVector2D(L.X, L.Y).Size() < Radio + R * .6f;
	case ELabFormaAviso::Linea: return L.X > -R && L.X < Largo + R && FMath::Abs(L.Y) < Ancho / 2 + R * .6f;
	case ELabFormaAviso::Cono:
	{
		const float D = FVector2D(L.X, L.Y).Size();
		return D < Radio + R && (D < R + 25 || FMath::Abs(FMath::Atan2(L.Y, L.X)) < Arco / 2);
	}
	default: return false;
	}
}

TArray<ALabPersonaje*> ALabAviso::Alcanzados() const
{
	TArray<ALabPersonaje*> Out;
	for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
	{
		ALabPersonaje* P = *It;
		if (!P->Salud || !P->Salud->Vivo() || P->Salud->Invulnerable > 0 || P->bEscapo) continue;
		if (Toca(P->GetActorLocation(), 45)) Out.Add(P);
	}
	return Out;
}
