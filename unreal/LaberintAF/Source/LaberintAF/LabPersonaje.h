// El jugador (updateHuman y melee de la versión web): WASD, Shift corre (gasta aire), Espacio rueda (invulnerable un instante),
// C se agacha (más lento y casi invisible), el mouse apunta, clic izquierdo pega (tres seguidos hacen combo; mantenido, golpe
// cargado), E usa (cofres, escotilla), Q toma una poción y F prende o apaga el farol. Cámara desde arriba e inclinada.
// El cuerpo es provisorio (formas básicas del motor): se reemplaza por un MetaHuman o un personaje de Fab (ver MIGRACION.md).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LabTipos.h"
#include "LabPersonaje.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class ULabSalud;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class LABERINTAF_API ALabPersonaje : public ACharacter
{
	GENERATED_BODY()

public:
	ALabPersonaje();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> Brazo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camara;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UPointLightComponent> Farol;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<ULabSalud> Salud;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Cuerpo;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Cabeza;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Arma;

	UPROPERTY(BlueprintReadOnly, Category = "Partida") float Aire = 100;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") int32 Oro = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") int32 Pociones = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") bool bTieneLlave = false;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") bool bEscapo = false;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") bool bAgachado = false;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") bool bFarolPrendido = true;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") FName ArmaId = TEXT("daga");
	UPROPERTY(BlueprintReadOnly, Category = "Partida") FString UltimoMensaje;
	UPROPERTY(BlueprintReadOnly, Category = "Partida") float MensajeT = 0;

	UFUNCTION(BlueprintCallable) void Mensaje(const FString& Texto) { UltimoMensaje = Texto; MensajeT = 3.4f; }
	UFUNCTION(BlueprintCallable) float Visibilidad() const;   // humanVis de la web: 1 normal, menos agachado o sin luz
	UFUNCTION(BlueprintCallable) void Equipar(FName Id);
	const FLabArma& ArmaActual() const;
	FString TextoUsar() const;

	virtual void Tick(float Dt) override;
	virtual void SetupPlayerInputComponent(UInputComponent* Entrada) override;

protected:
	virtual void BeginPlay() override;

private:
	// teclas creadas por código (sin assets de Enhanced Input)
	UPROPERTY() TObjectPtr<UInputMappingContext> Contexto;
	UPROPERTY() TObjectPtr<UInputAction> AMover;
	UPROPERTY() TObjectPtr<UInputAction> ACorrer;
	UPROPERTY() TObjectPtr<UInputAction> ARodar;
	UPROPERTY() TObjectPtr<UInputAction> AAgachar;
	UPROPERTY() TObjectPtr<UInputAction> AAtacar;
	UPROPERTY() TObjectPtr<UInputAction> AUsar;
	UPROPERTY() TObjectPtr<UInputAction> APocion;
	UPROPERTY() TObjectPtr<UInputAction> AFarol;

	FVector2D Movimiento = FVector2D::ZeroVector;
	bool bCorriendo = false, bAtaqueApretado = false;
	float Carga = 0, Enfriamiento = 0, ComboT = 0, RodarT = 0, RodarCd = 0, GolpeT = 0;
	int32 Combo = 0;
	FVector RodarDir = FVector::ForwardVector;

	void Mover(const FInputActionValue& V);
	void MoverFin(const FInputActionValue& V) { Movimiento = FVector2D::ZeroVector; }
	void CorrerSi(const FInputActionValue& V) { bCorriendo = true; }
	void CorrerNo(const FInputActionValue& V) { bCorriendo = false; }
	void Rodar(const FInputActionValue& V);
	void Agachar(const FInputActionValue& V);
	void AtaqueSi(const FInputActionValue& V) { bAtaqueApretado = true; }
	void AtaqueNo(const FInputActionValue& V);
	void Usar(const FInputActionValue& V);
	void Pocion(const FInputActionValue& V);
	void CambiarFarol(const FInputActionValue& V);
	void Golpear(bool bCargado);
	void ApuntarAlMouse();
	UFUNCTION() void AlMorir(AActor* Victima, AActor* Asesino);
};
