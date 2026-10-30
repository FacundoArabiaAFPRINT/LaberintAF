// La partida (newMatch, update y endMatch de la versión web): arma el mundo con código (luz de luna, cielo, niebla volumétrica,
// posproceso), genera el laberinto, reparte bestias, élites, jefes y cofres, crea a tu cazador dormido hasta que termina el
// farmeo, cuenta los 15 minutos y maneja la escotilla de la salida y el final.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LabModoJuego.generated.h"

class ALabLaberinto;
class ALabPersonaje;
class ALabEnemigo;
class ALabCofre;

UCLASS()
class LABERINTAF_API ALabModoJuego : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALabModoJuego();

	static ALabModoJuego* Get(const UObject* Contexto);
	static ALabLaberinto* Laberinto(const UObject* Contexto);

	UPROPERTY(EditAnywhere, Category = "Partida") float Duracion = 900;    // 480 en la partida rápida
	UPROPERTY(EditAnywhere, Category = "Partida") float Farmeo = 120;      // sin cazadores al principio
	UPROPERTY(EditAnywhere, Category = "Partida") int32 CantidadBestias = 66;
	UPROPERTY(EditAnywhere, Category = "Partida") int32 NivelCazador = 0;  // 0 = al azar (1 a 10)

	UPROPERTY(BlueprintReadOnly) float Tiempo = 0;
	UPROPERTY(BlueprintReadOnly) bool bCazadoresAfuera = false;
	UPROPERTY(BlueprintReadOnly) bool bTerminada = false;
	UPROPERTY(BlueprintReadOnly) bool bGano = false;
	UPROPERTY(BlueprintReadOnly) FString FinTitulo;
	UPROPERTY(BlueprintReadOnly) FString FinTexto;
	UPROPERTY(BlueprintReadOnly) FString AnuncioTitulo;
	UPROPERTY(BlueprintReadOnly) FString AnuncioTexto;
	UPROPERTY(BlueprintReadOnly) float AnuncioT = 0;
	UPROPERTY(BlueprintReadOnly) float Escotilla = 0;        // segundos aguantados (hacen falta 10)
	UPROPERTY(BlueprintReadOnly) float EscotillaAbiertaT = 0;

	UPROPERTY() TObjectPtr<ALabLaberinto> Mapa;
	UPROPERTY() TArray<TObjectPtr<ALabEnemigo>> Cazadores;

	ALabPersonaje* Jugador() const;
	const ALabEnemigo* CazadorConLlave() const;
	FVector LugarSalida() const;
	bool EscotillaAbierta() const { return EscotillaAbiertaT > 0; }
	float TiempoRestante() const { return FMath::Max(0.f, Duracion - Tiempo); }
	float FaltaParaCazadores() const { return FMath::Max(0.f, Farmeo - Tiempo); }
	ALabEnemigo* JefeCerca() const;

	ALabEnemigo* CrearBestia(const FString& Id, const FVector& Lugar, bool bElite);
	void AlMorirEnemigo(ALabEnemigo* Muerto, AActor* Asesino);
	void UsarEscotilla(ALabPersonaje* P);
	void Anunciar(const FString& Titulo, const FString& Texto) { AnuncioTitulo = Titulo; AnuncioTexto = Texto; AnuncioT = 4.5f; }
	void Terminar(bool bEscapo, AActor* Asesino);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void StartPlay() override;
	virtual void Tick(float Dt) override;

private:
	bool bPoblada = false;
	bool bAbriendo = false;
	void Asegurar();
	void Ambiente();
	void Poblar();
	void SoltarCazadores();
	ALabCofre* CrearCofre(const FVector& Lugar, uint8 Clase, ALabEnemigo* Jefe = nullptr);
};
