#include "py/nlr.h"
#include "py/smallint.h"
#include "py/obj.h"

#include "fsl_hwtimer_systick.h"

static hwtimer_t ht;
static uint32_t ht_period;

extern void HWTIMER_SYS_SystickIsrAction(void);

void SysTick_Handler(void)
{
    HWTIMER_SYS_SystickIsrAction();
}

void init_ticks(void)
{
DEBUG_printf("Inititalizing Timer\n");
    HWTIMER_SYS_Init(&ht, &kSystickDevif, 0, NULL);
DEBUG_printf("Setting period\n");
    HWTIMER_SYS_SetPeriod(&ht, 1000);
DEBUG_printf("Period Set\n");
    ht_period = HWTIMER_SYS_GetModulo(&ht);
DEBUG_printf("modulo %d\n", ht_period);
    HWTIMER_SYS_Start(&ht);
DEBUG_printf("Timer started\n");
}

static uint32_t micros(void)
{
    hwtimer_time_t t;
    HWTIMER_SYS_GetTime(&ht, &t);
    return (uint32_t)(t.ticks * 1000) + ((int32_t)(t.subTicks * 1000) / (int32_t)ht_period);
}

static uint32_t millis(void)
{
    return HWTIMER_SYS_GetTicks(&ht);
}

STATIC mp_obj_t time_sleep_us(mp_obj_t usec_in) {
    uint32_t start;
    mp_int_t usec = mp_obj_get_int(usec_in);
    if (usec > 0) {
        start = micros();
        while ((micros() - start) < usec)
            __NOP();
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_us_obj, time_sleep_us);

STATIC mp_obj_t time_sleep_ms(mp_obj_t msec_in) {
    uint32_t start;
    mp_int_t msec = mp_obj_get_int(msec_in);
    if (msec > 0) {
        start = millis();
        while ((millis() - start) < msec) {
            __WFE();
        }
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_ms_obj, time_sleep_ms);

STATIC mp_obj_t time_ticks_ms(void) {
    uint32_t ticks;
    ticks = HWTIMER_SYS_GetTicks(&ht);
    return MP_OBJ_NEW_SMALL_INT(ticks & MP_SMALL_INT_POSITIVE_MASK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_ticks_ms_obj, time_ticks_ms);

STATIC mp_obj_t time_ticks_us(void) {
    return MP_OBJ_NEW_SMALL_INT(micros() & MP_SMALL_INT_POSITIVE_MASK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_ticks_us_obj, time_ticks_us);

STATIC mp_obj_t time_ticks_diff(mp_obj_t start_in, mp_obj_t end_in) {
    // we assume that the arguments come from ticks_xx so are small ints
    uint32_t start = MP_OBJ_SMALL_INT_VALUE(start_in);
    uint32_t end = MP_OBJ_SMALL_INT_VALUE(end_in);
    return MP_OBJ_NEW_SMALL_INT((end - start) & MP_SMALL_INT_POSITIVE_MASK);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(time_ticks_diff_obj, time_ticks_diff);

STATIC const mp_map_elem_t time_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_utime) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep_ms), (mp_obj_t)&time_sleep_ms_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep_us), (mp_obj_t)&time_sleep_us_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks_ms), (mp_obj_t)&time_ticks_ms_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks_us), (mp_obj_t)&time_ticks_us_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks_diff), (mp_obj_t)&time_ticks_diff_obj },
};

STATIC MP_DEFINE_CONST_DICT(time_module_globals, time_module_globals_table);

const mp_obj_module_t mp_module_utime = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&time_module_globals,
};
