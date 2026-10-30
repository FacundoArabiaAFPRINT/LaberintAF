#include "LabLaberinto.h"
#include "LabTipos.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

ALabLaberinto::ALabLaberinto()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	Muros = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Muros"));
	Muros->SetupAttachment(RootComponent);
	Muros->SetCollisionProfileName(TEXT("BlockAll"));
	Piso = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piso"));
	Piso->SetupAttachment(RootComponent);
	Piso->SetCollisionProfileName(TEXT("BlockAll"));

	// Mallas básicas del motor: el proyecto anda sin assets propios. Se reemplazan por Megascans desde el editor.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plano(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (Cubo.Succeeded()) MallaMuro = Cubo.Object;
	if (Plano.Succeeded()) MallaPiso = Plano.Object;
	if (Mat.Succeeded()) { MaterialMuro = Mat.Object; MaterialPiso = Mat.Object; }
}

void ALabLaberinto::Pasillo(int32 CX, int32 CY)
{
	for (int32 Y = 1; Y < CS; ++Y) for (int32 X = 1; X < CS; ++X) Grilla[(CY * CS + Y) * TW + CX * CS + X] = ELabCelda::Pasillo;
}

void ALabLaberinto::Entre(int32 CX, int32 CY, int32 NX, int32 NY)
{
	if (NX != CX) { const int32 X = FMath::Max(CX, NX) * CS; for (int32 Y = 1; Y < CS; ++Y) Grilla[(CY * CS + Y) * TW + X] = ELabCelda::Pasillo; }
	else { const int32 Y = FMath::Max(CY, NY) * CS; for (int32 X = 1; X < CS; ++X) Grilla[Y * TW + CX * CS + X] = ELabCelda::Pasillo; }
}

void ALabLaberinto::Generar()
{
	Rng.Initialize(Semilla ? Semilla : FMath::Rand());
	Grilla.Init(ELabCelda::Muro, TW * TH);
	Salas.Reset();
	TArray<uint8> Vis; Vis.Init(0, CW * CH);
	const int32 C0 = MID - 1;
	auto EnMedio = [C0](int32 X, int32 Y) { return X >= C0 && X <= C0 + 2 && Y >= C0 && Y <= C0 + 2; };
	for (int32 Y = 0; Y < CH; ++Y) for (int32 X = 0; X < CW; ++X) if (EnMedio(X, Y)) Vis[Y * CW + X] = 1;

	// laberinto perfecto (backtracker)
	TArray<FIntPoint> Pila; Pila.Add({0, 0}); Vis[0] = 1; Pasillo(0, 0);
	const FIntPoint D[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	while (Pila.Num())
	{
		const FIntPoint C = Pila.Last();
		TArray<FIntPoint> Vecinos;
		for (const FIntPoint& d : D)
		{
			const FIntPoint N = C + d;
			if (N.X >= 0 && N.Y >= 0 && N.X < CW && N.Y < CH && !Vis[N.Y * CW + N.X]) Vecinos.Add(N);
		}
		if (!Vecinos.Num()) { Pila.Pop(); continue; }
		const FIntPoint N = Vecinos[Rng.RandRange(0, Vecinos.Num() - 1)];
		Entre(C.X, C.Y, N.X, N.Y); Vis[N.Y * CW + N.X] = 1; Pasillo(N.X, N.Y); Pila.Add(N);
	}
	// atajos (10 %)
	for (int32 CY = 0; CY < CH; ++CY) for (int32 CX = 0; CX < CW; ++CX)
	{
		if (EnMedio(CX, CY)) continue;
		if (CX < CW - 1 && !EnMedio(CX + 1, CY) && Rng.FRand() < .1f) Entre(CX, CY, CX + 1, CY);
		if (CY < CH - 1 && !EnMedio(CX, CY + 1) && Rng.FRand() < .1f) Entre(CX, CY, CX, CY + 1);
	}
	// salas
	TArray<uint8> Usado; Usado.Init(0, CW * CH);
	auto Entra = [&](int32 CX, int32 CY, int32 W, int32 H)
	{
		if (CX < 0 || CY < 0 || CX + W > CW || CY + H > CH) return false;
		for (int32 Y = CY - 1; Y <= CY + H; ++Y) for (int32 X = CX - 1; X <= CX + W; ++X)
			if (X >= 0 && Y >= 0 && X < CW && Y < CH && Usado[Y * CW + X]) return false;
		return true;
	};
	auto Tallar = [&](int32 CX, int32 CY, int32 W, int32 H, const TCHAR* Tipo)
	{
		for (int32 Y = CY; Y < CY + H; ++Y) for (int32 X = CX; X < CX + W; ++X) Usado[Y * CW + X] = 1;
		for (int32 Y = CY * CS + 1; Y <= (CY + H) * CS - 1; ++Y) for (int32 X = CX * CS + 1; X <= (CX + W) * CS - 1; ++X) Grilla[Y * TW + X] = ELabCelda::Sala;
		FLabSala S; S.Tipo = Tipo; S.CX = CX; S.CY = CY; S.W = W; S.H = H;
		S.Min = Centro(CX * CS + 1, CY * CS + 1) - FVector(Lab::Tile / 2, Lab::Tile / 2, 0);
		S.Max = Centro((CX + W) * CS - 1, (CY + H) * CS - 1) + FVector(Lab::Tile / 2, Lab::Tile / 2, 0);
		S.Centro = (S.Min + S.Max) / 2;
		Salas.Add(S);
	};
	Tallar(C0, C0, 3, 3, TEXT("exit"));
	Entre(MID, C0, MID, C0 - 1); Entre(MID, C0 + 2, MID, C0 + 3); Entre(C0, MID, C0 - 1, MID); Entre(C0 + 2, MID, C0 + 3, MID);
	const int32 Ct = MID * CS + 2;
	for (int32 Y = -1; Y <= 1; ++Y) for (int32 X = -1; X <= 1; ++X) Grilla[(Ct + Y) * TW + Ct + X] = ELabCelda::Salida;
	for (const FIntPoint& Q : {FIntPoint(-4, -4), FIntPoint(4, -4), FIntPoint(-4, 4), FIntPoint(4, 4)}) Grilla[(Ct + Q.Y) * TW + Ct + Q.X] = ELabCelda::Muro;
	Salida = Centro(Ct, Ct);
	auto Probar = [&](int32 W, int32 H, const TCHAR* Tipo, float MinC)
	{
		for (int32 k = 0; k < 300; ++k)
		{
			const int32 CX = Rng.RandRange(0, CW - W), CY = Rng.RandRange(0, CH - H);
			if (FMath::Sqrt(FMath::Square(CX + W / 2.f - CW / 2.f) + FMath::Square(CY + H / 2.f - CH / 2.f)) < MinC + 1.5f) continue;
			if (Entra(CX, CY, W, H)) { Tallar(CX, CY, W, H, Tipo); return; }
		}
	};
	Probar(3, 3, TEXT("boss"), 4); Probar(3, 3, TEXT("boss"), 4);
	Probar(2, 2, TEXT("vault"), 3); Probar(2, 2, TEXT("vault"), 3);
	Probar(2, 2, TEXT("merchant"), 3); Probar(2, 2, TEXT("merchant"), 3);
	Probar(2, 2, TEXT("altar"), 3); Probar(2, 2, TEXT("altar"), 3);
	for (int32 i = 0; i < 3; ++i) Probar(2, 2, TEXT("treasure"), 3);
	for (int32 i = 0; i < 6; ++i) Probar(2 + Rng.RandRange(0, 1), 2 + Rng.RandRange(0, 1), TEXT("hall"), 3);
	// columnas en los salones
	for (const FLabSala& S : Salas)
	{
		if (S.Tipo != TEXT("hall")) continue;
		for (int32 i = 1; i < S.W; ++i) for (int32 j = 1; j < S.H; ++j) if (Rng.FRand() < .75f) Grilla[((S.CY + j) * CS) * TW + (S.CX + i) * CS] = ELabCelda::Muro;
	}
	// cuevas
	for (int32 k = 0; k < 8; ++k)
	{
		int32 CX, CY;
		do { CX = Rng.RandRange(0, CW - 1); CY = Rng.RandRange(0, CH - 1); } while (FMath::Abs(CX - MID) < 3 && FMath::Abs(CY - MID) < 3);
		const int32 TX = CX * CS + 2, TY = CY * CS + 2; const float Rad = 2 + Rng.FRand() * 1.8f;
		for (int32 Y = -5; Y <= 5; ++Y) for (int32 X = -5; X <= 5; ++X)
		{
			const int32 PX = TX + X, PY = TY + Y;
			if (PX < 1 || PY < 1 || PX >= TW - 1 || PY >= TH - 1) continue;
			if (FMath::Sqrt(float(X * X + Y * Y)) < Rad + Rng.FRand() * .9f && Grilla[PY * TW + PX] == ELabCelda::Muro) Grilla[PY * TW + PX] = ELabCelda::Pasillo;
		}
	}
}

void ALabLaberinto::Construir()
{
	Muros->ClearInstances();
	Muros->SetStaticMesh(MallaMuro);
	if (MaterialMuro) Muros->SetMaterial(0, MaterialMuro);
	const float K = Lab::Tile / 100.f;   // la malla básica mide 1 m
	for (int32 Y = 0; Y < TH; ++Y) for (int32 X = 0; X < TW; ++X)
	{
		if (Grilla[Y * TW + X] != ELabCelda::Muro) continue;
		// solo los muros que tocan algo caminable (el resto queda tapado y no hace falta dibujarlo)
		bool Borde = false;
		for (int32 dy = -1; dy <= 1 && !Borde; ++dy) for (int32 dx = -1; dx <= 1; ++dx) if (En(X + dx, Y + dy) != ELabCelda::Muro) { Borde = true; break; }
		if (!Borde) continue;
		const FVector C = Centro(X, Y) + FVector(0, 0, Lab::AltoMuro / 2);
		Muros->AddInstance(FTransform(FRotator::ZeroRotator, C, FVector(K, K, Lab::AltoMuro / 100.f)), true);
	}
	Piso->SetStaticMesh(MallaPiso);
	if (MaterialPiso) Piso->SetMaterial(0, MaterialPiso);
	Piso->SetWorldLocation(Centro(TW / 2, TH / 2));
	Piso->SetWorldScale3D(FVector(TW * K, TH * K, 1));
}

FVector ALabLaberinto::Centro(int32 X, int32 Y) const
{
	return GetActorLocation() + FVector((X + .5f) * Lab::Tile, (Y + .5f) * Lab::Tile, 0);
}

FIntPoint ALabLaberinto::Casillero(const FVector& P) const
{
	const FVector L = P - GetActorLocation();
	return FIntPoint(FMath::FloorToInt(L.X / Lab::Tile), FMath::FloorToInt(L.Y / Lab::Tile));
}

bool ALabLaberinto::EsSolido(const FVector& P) const
{
	const FIntPoint C = Casillero(P);
	return En(C.X, C.Y) == ELabCelda::Muro;
}

bool ALabLaberinto::Visible(const FVector& A, const FVector& B) const
{
	const float D = FVector::Dist2D(A, B);
	const int32 N = FMath::CeilToInt(D / Lab::Cm(4));
	for (int32 i = 1; i < N; ++i) if (EsSolido(FMath::Lerp(A, B, float(i) / N))) return false;
	return true;
}

const FLabSala* ALabLaberinto::SalaEn(const FVector& P) const
{
	for (const FLabSala& S : Salas) if (S.Contiene(P)) return &S;
	return nullptr;
}

FVector ALabLaberinto::PuntoLibre(TFunctionRef<bool(const FVector&)> Filtro, bool* Encontrado)
{
	for (int32 k = 0; k < 800; ++k)
	{
		const int32 X = Rng.RandRange(1, TW - 2), Y = Rng.RandRange(1, TH - 2);
		const ELabCelda T = Grilla[Y * TW + X];
		if (T != ELabCelda::Pasillo && T != ELabCelda::Sala) continue;
		const FVector P = Centro(X, Y);
		if (Filtro(P)) { if (Encontrado) *Encontrado = true; return P; }
	}
	if (Encontrado) *Encontrado = false;
	return Centro(1, 1);
}

FVector ALabLaberinto::PuntoLibreCerca(const FVector& Desde, float RadioCm)
{
	for (int32 k = 0; k < 30; ++k)
	{
		const float A = Rng.FRand() * 2 * PI, D = Rng.FRand() * RadioCm;
		const FVector P = Desde + FVector(FMath::Cos(A) * D, FMath::Sin(A) * D, 0);
		if (!EsSolido(P) && Visible(Desde, P)) return P;
	}
	return Desde;
}

TArray<FVector> ALabLaberinto::Camino(const FVector& Desde, const FVector& Hasta) const
{
	TArray<FVector> Out;
	const FIntPoint A = Casillero(Desde), B = Casillero(Hasta);
	if (En(A.X, A.Y) == ELabCelda::Muro || En(B.X, B.Y) == ELabCelda::Muro) return Out;
	TArray<int32> Previo; Previo.Init(-1, TW * TH);
	TArray<int32> Cola; Cola.Reserve(TW * TH);
	const int32 S = A.Y * TW + A.X, G = B.Y * TW + B.X;
	Cola.Add(S); Previo[S] = S;
	for (int32 h = 0; h < Cola.Num(); ++h)
	{
		const int32 C = Cola[h];
		if (C == G) break;
		const int32 X = C % TW, Y = C / TW;
		const int32 Vec[4] = {C + 1, C - 1, C + TW, C - TW};
		const bool Ok[4] = {X < TW - 1, X > 0, Y < TH - 1, Y > 0};
		for (int32 i = 0; i < 4; ++i) if (Ok[i] && Previo[Vec[i]] < 0 && Grilla[Vec[i]] != ELabCelda::Muro) { Previo[Vec[i]] = C; Cola.Add(Vec[i]); }
	}
	if (Previo[G] < 0) return Out;
	for (int32 C = G; C != S; C = Previo[C]) Out.Insert(Centro(C % TW, C / TW), 0);
	return Out;
}

TArray<FIntPoint> ALabLaberinto::CallejonesSinSalida() const
{
	TArray<FIntPoint> Out;
	for (int32 CY = 0; CY < CH; ++CY) for (int32 CX = 0; CX < CW; ++CX)
	{
		const int32 TX = CX * CS + 1, TY = CY * CS + 1;
		if (En(TX, TY) != ELabCelda::Pasillo || (FMath::Abs(CX - MID) < 2 && FMath::Abs(CY - MID) < 2)) continue;
		auto Col = [&](int32 X) { for (int32 i = 0; i < 3; ++i) if (En(X, TY + i) != ELabCelda::Muro) return false; return true; };
		auto Fila = [&](int32 Y) { for (int32 i = 0; i < 3; ++i) if (En(TX + i, Y) != ELabCelda::Muro) return false; return true; };
		const int32 M = int32(Col(CX * CS)) + int32(Col((CX + 1) * CS)) + int32(Fila(CY * CS)) + int32(Fila((CY + 1) * CS));
		if (M == 3) Out.Add(FIntPoint(CX * CS + 2, CY * CS + 2));
	}
	return Out;
}
