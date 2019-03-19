
#include "player.h"
#include "softTone.h"
#include "tmr.h"

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

//------------------------------------------------------
// void InicializaEfecto (TipoEfecto *p_efecto)
//
// Descripcion: inicializa los parametros del objeto efecto.
// Ejemplo de uso:
//
// ...
//
// TipoEfecto efecto_demo;
//
// if ( InicializaEfecto (&efecto_demo, "DISPARO", frecuenciasDisparo, tiemposDisparo, 16) < 0 ) {
// 		printf("\n[ERROR!!!][InicializaEfecto]\n");
// 		fflush(stdout);
// 		}
//
// ...
//
//------------------------------------------------------
void InicializaEfecto (TipoEfecto *p_efecto, char *nombre, int *array_frecuencias, int *array_duraciones, int num_notas)
{
	p_efecto->nombre = *nombre;
	p_efecto->frecuencias = *array_frecuencias;
	p_efecto->duraciones = *array_duraciones;
	p_efecto->num_notas = num_notas;
	//return p_efecto->num_notas;
}

// Procedimiento de inicializacion del objeto especifico
// Nota: parte inicialización común a InicializaPlayDisparo y InicializaPlayImpacto
void InicializaPlayer (TipoPlayer *p_player)
{
	p_player->posicion_nota_actual = 0;
	p_player->frecuencia_nota_actual = p_player.p_diapro.frecuancias[0];
	p_player->duracion_nota_actual = p_player.p_diapro.duraciones[0];
	//softToneCreate (PLAYER_PWM_PIN); la mando al config de piTankGo
	softToneWrite (PLAYER_PWM_PIN, p_player.frecuencia_nota_actual);
	tmr_startms(tmr_t* tmr_t, this->user_data.duracion_nota_actual);
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaStartDisparo (fsm_t* this)
{
	int result = 0;
	if (flags_player && FLAG_START_DISPARO)
	{
		result = 1;
	}
	return result;
}

int CompruebaStartImpacto (fsm_t* this)
{
	int result = 0;
	if (flags_player && FLAG_START_IMPACTO)
	{
		result = 1;
	}
	return result;
}

int CompruebaNuevaNota (fsm_t* this)
{
	int result = 0;
	if (!(flags_player && FLAG_PLAYER_END))
	{
		result = 1;
	}
	return result;
}

int CompruebaNotaTimeout (fsm_t* this)
{
	int result = 0;
	if (flags_player && FLAG_NOTA_TIMEOUT)
	{
		result = 1;
	}
	return result;
}

int CompruebaFinalEfecto (fsm_t* this)
{
	int result = 0;
	if (flags_player && FLAG_PLAYER_END)
	{
		result = 1;
	}
	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void InicializaPlayDisparo (fsm_t* this)
{
	this->user_data.p_efecto = &this->user_data.efecto_disparo;
	InicializaPlayer (&this.user_data);
	flags_player = flags_player&!FLAG_START_DISPARO;
}

void InicializaPlayImpacto (fsm_t* this)
{
	this->user_data.p_efecto = &this->user_data.efecto_impacto;
	InicializaPlayer (&this.user_data);
	flags_player = flags_player&!FLAG_START_IMPACTO;
}

void ComienzaNuevaNota (fsm_t* this)
{
	softToneWrite (PLAYER_PWM_PIN, this->user_data.frecuencia_nota_actual);
	tmr_startms(tmr_t* tmr_t, this->user_data.duracion_nota_actual);
}

void ActualizaPlayer (fsm_t* this)
{
	if (this->user_data.posicion_nota_actual + 1 < this->user_data.p_efecto.num_notas)
	{
		this->user_data.posicion_nota_actual += 1;
		this->user_data.frecuencia_nota_actual = this->user_data.p_efecto.frecuancias[this->user_data.posicion_nota_actual];
		this->user_data.duracion_nota_actual = this->user_data.p_efecto.duraciones[this->user_data.posicion_nota_actual];
	}
	else
	{
		flags_player|FLAG_PLAYER_END = 1;
	}

	flags_player = flags_player&!FLAG_NOTA_TIMEOUT;
}

void FinalEfecto (fsm_t* this)
{
	softToneStop (PLAYER_PWM_PIN);
	flags_player = flags_player&!FLAG_PLAYER_END;
}

//------------------------------------------------------
// PROCEDIMIENTOS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void timer_player_duracion_nota_actual_isr (union sigval value)
{
	flags_player|FLAG_NOTA_TIMEOUT=1;
	tmr_stop (tmr_t* tmr_t);
}
