#include "LabHUD.h"
#include "LabModoJuego.h"
#include "LabPersonaje.h"
#include "LabEnemigo.h"
#include "LabSalud.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

void ALabHUD::Barra(float X, float Y, float W, float H, float F, FLinearColor Col)
{
	DrawRect(FLinearColor::Black, X - 3, Y - 3, W + 6, H + 6);
	DrawRect(FLinearColor(.08f, .04f, .03f), X, Y, W, H);
	DrawRect(Col, X, Y, W * FMath::Clamp(F, 0.f, 1.f), H);
}

void ALabHUD::Texto(const FString& T, float X, float Y, FLinearColor Col, float Escala, bool bCentrado)
{
	UFont* F = GEngine->GetLargeFont();
	float W = 0, H = 0;
	GetTextSize(T, W, H, F, Escala);
	const float PX = bCentrado ? X - W / 2 : X;
	DrawText(T, FLinearColor::Black, PX + 2, Y + 2, F, Escala);
	DrawText(T, Col, PX, Y, F, Escala);
}

void ALabHUD::DrawHUD()
{
	Super::DrawHUD();
	ALabModoJuego* M = ALabModoJuego::Get(this);
	ALabPersonaje* J = M ? M->Jugador() : nullptr;
	if (!M || !J || !Canvas) return;
	const float W = Canvas->SizeX, H = Canvas->SizeY, K = FMath::Max(.7f, H / 900.f);
	const FLinearColor Tinta(.91f, .86f, .75f), Brasa(.82f, .54f, .27f), Sangre(.66f, .16f, .12f);

	const int32 T = FMath::CeilToInt(M->TiempoRestante());
	Texto(FString::Printf(TEXT("%d:%02d"), T / 60, T % 60), W / 2, 10 * K, Tinta, 1.6f * K, true);
	Barra(20 * K, 20 * K, 220 * K, 12 * K, J->Salud->Vida / J->Salud->VidaMax, Sangre);
	Barra(20 * K, 42 * K, 220 * K, 8 * K, J->Aire / 100.f, FLinearColor(.54f, .52f, .45f));
	Texto(FString::Printf(TEXT("Oro %d · Pociones %d (Q) · %s"), J->Oro, J->Pociones, *J->ArmaActual().Nombre), 20 * K, 58 * K, Tinta, 1.f * K);
	FString Caza;
	if (!M->bCazadoresAfuera) Caza = FString::Printf(TEXT("FARMEO · tu cazador sale en %d s"), FMath::CeilToInt(M->FaltaParaCazadores()));
	else if (J->bTieneLlave) Caza = TEXT("Tenés tu llave: la salida está en el centro.");
	else Caza = TEXT("Matá a tu cazador para conseguir tu llave.");
	Texto(Caza, 20 * K, 80 * K, Brasa, 1.f * K);
	if (J->bAgachado) Texto(TEXT("AGACHADO"), 20 * K, 102 * K, FLinearColor(.56f, .88f, .48f), .9f * K);
	if (J->Salud->Sangrado > 0) Texto(TEXT("SANGRANDO"), 140 * K, 102 * K, Sangre, .9f * K);

	if (ALabEnemigo* B = M->JefeCerca())
	{
		Texto(B->Nombre.ToUpper() + (B->bFuria ? TEXT(" · FURIOSO") : TEXT("")), W / 2, 92 * K, FLinearColor(1, .69f, .25f), 1.2f * K, true);
		Barra(W / 2 - 210 * K, 122 * K, 420 * K, 12 * K, B->Salud->Vida / B->Salud->VidaMax, FLinearColor(.88f, .25f, .19f));
	}
	if (M->AnuncioT > 0)
	{
		Texto(M->AnuncioTitulo, W / 2, H * .18f, FLinearColor(1, .82f, .23f), 2.2f * K, true);
		Texto(M->AnuncioTexto, W / 2, H * .18f + 50 * K, Tinta, 1.f * K, true);
	}
	const FString Usar = J->TextoUsar();
	if (!Usar.IsEmpty()) Texto(Usar, W / 2, H - 150 * K, Tinta, 1.1f * K, true);
	if (J->MensajeT > 0) Texto(J->UltimoMensaje, W / 2, H - 110 * K, Brasa, 1.1f * K, true);
	if (M->Escotilla > 0 && !M->EscotillaAbierta()) Barra(W / 2 - 120 * K, H - 70 * K, 240 * K, 10 * K, M->Escotilla / 10.f, Brasa);

	if (M->bTerminada)
	{
		DrawRect(FLinearColor(0, 0, 0, .7f), 0, 0, W, H);
		Texto(M->FinTitulo, W / 2, H * .38f, M->bGano ? FLinearColor(1, .82f, .23f) : Sangre, 3.f * K, true);
		Texto(M->FinTexto, W / 2, H * .38f + 80 * K, Tinta, 1.1f * K, true);
		Texto(TEXT("Otra partida en unos segundos…"), W / 2, H * .38f + 120 * K, FLinearColor(.6f, .58f, .5f), .9f * K, true);
	}
}
