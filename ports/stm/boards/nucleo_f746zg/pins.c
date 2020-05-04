#include "shared-bindings/board/__init__.h"

#define DT_DRV_COMPAT circuitpython_pin

#define CREATE_GLOBALS_TABLE(inst) \
       { MP_ROM_QSTR(MP_QSTR_FOO##inst), MP_ROM_PTR(DT_INST_MP_ROM_PTR(inst)) },

STATIC const mp_rom_map_elem_t board_module_globals_table[] = {
DT_INST_FOREACH(CREATE_GLOBALS_TABLE)
};
MP_DEFINE_CONST_DICT(board_module_globals, board_module_globals_table);
