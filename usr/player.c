
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
//      printf("\n[ERROR!!!][InicializaEfecto]\n");
//      fflush(stdout);
//      }
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
    p_player->frecuencia_nota_actual = p_player->p_efecto->frecuancias[0];
    p_player->duracion_nota_actual = p_player->p_efecto->duraciones[0];

    softToneWrite (PLAYER_PWM_PIN, p_player->frecuencia_nota_actual);//primera nota a sonar
    //se le pasa el timer asociado y se le pasa el tiempo en milisegundos que dura la nota
    tmr_startms(timer_efecto, p_player->duracion_nota_actual);
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaStartDisparo (fsm_t* this)
{
    int result = 0;
    if (flags_player & FLAG_START_DISPARO)
    {
        result = 1;
    }
    return result;
}

/*Puede "expulsar" el efecto de Disparo. Si se ha producido el impacto
hay que evitar que esta comprobación vuelva a mostrarse ya que provocaría 
un iniciaPlayImpact() de forma constante. Es decir, solo tiene que entrar una vez.
*/
int CompruebaStartImpacto (fsm_t* this)
{
    int result = 0;
    //desde otro punto se le tiene que mandar el flag de que ha habido impacto
    if (flags_player & FLAG_START_IMPACTO)
    {
        result = 1;
    }

    return result;
}

int CompruebaNuevaNota (fsm_t* this)
{
    int result = 0;
    if (!(flags_player & FLAG_PLAYER_END))
    {
        result = 1;
    }
    return result;
}

int CompruebaNotaTimeout (fsm_t* this)
{
    int result = 0;
    if (flags_player & FLAG_NOTA_TIMEOUT)//se modifica en la interrupcion de timer
    {
        result = 1;
    }
    return result;
}

int CompruebaFinalEfecto (fsm_t* this)
{
    int result = 0;
    if (flags_player & FLAG_PLAYER_END)
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
    //se asigna el puntero del player al efecto disparo. Así al llamar a InicializaPlayer se apunta a los datos del efecto de disparo
    this->user_data.p_efecto = &(this->user_data.efecto_disparo);
    InicializaPlayer (&(this.user_data));//se le pasa toda la estrucctura player
    flags_player &= ~FLAG_START_DISPARO;//se deshabilita el flag de disparo
}

void InicializaPlayImpacto (fsm_t* this)
{
    this->user_data->p_efecto = &(this->user_data.efecto_impacto);
    InicializaPlayer (&(this.user_data));
    flahs_player &= ~FLAG_START_IMPACTO;//se deshabilita el flag para que no vuelva a entrar por el mismo flag
}

void ComienzaNuevaNota (fsm_t* this)
{
    softToneWrite (PLAYER_PWM_PIN, this->user_data->frecuencia_nota_actual);
    tmr_startms(timer_efecto, this->user_data->duracion_nota_actual);
}

void ActualizaPlayer (fsm_t* this)
{
    int notaActual;

    if (this->user_data->posicion_nota_actual + 1 < this->user_data->p_efecto->num_notas)
    {
        notaActual = this->user_data->posicion_nota_actual++;
        this->user_data->frecuencia_nota_actual = this->user_data->p_efecto->frecuancias[notaActual];
        this->user_data->duracion_nota_actual = this->user_data->p_efecto->duraciones[notaActual];
    }
    else 
    {
        flags_player |= FLAG_PLAYER_END;
    }

    flahs_player &= ~FLAG_NOTA_TIMEOUT;//se baja el flag del time out
}

void FinalEfecto (fsm_t* this)
{
    softToneWrite (PLAYER_PWM_PIN, NO_SONAR);//probar con softStop(int pin)
    flags_player &= ~FLAG_PLAYER_END;//quitamos el flag de final de efecto
}

//------------------------------------------------------
// PROCEDIMIENTOS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void timer_player_duracion_nota_actual_isr (union sigval value)
{
    tmr_stop (tmr_t* tmr_t);//paramos el timer del efecto
    flags_player |= FLAG_NOTA_TIMEOUT;//se activa el flag de tiempo cumplido de la nota
    
}
