.CODE

PUBLIC snuk_utils_pause_instruction, snuk_utils_debug_break

snuk_utils_debug_break PROC
    pause
    ret
snuk_utils_debug_break ENDP

snuk_utils_debug_break PROC
    int 3
    ret
snuk_utils_debug_break ENDP

END
