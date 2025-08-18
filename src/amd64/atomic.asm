.CODE

PUBLIC snuk_atomic_load_i8, snuk_atomic_store_i8
PUBLIC snuk_atomic_exchange_i8, snuk_atomic_compare_exchange_strong_i8
PUBLIC snuk_atomic_fetch_add_i8, snuk_atomic_fetch_sub_i8
PUBLIC snuk_atomic_fetch_or_i8, snuk_atomic_fetch_xor_i8
PUBLIC snuk_atomic_fetch_and_i8

PUBLIC snuk_atomic_load_i16, snuk_atomic_store_i16
PUBLIC snuk_atomic_exchange_i16, snuk_atomic_compare_exchange_strong_i16
PUBLIC snuk_atomic_fetch_add_i16, snuk_atomic_fetch_sub_i16
PUBLIC snuk_atomic_fetch_or_i16, snuk_atomic_fetch_xor_i16
PUBLIC snuk_atomic_fetch_and_i16

PUBLIC snuk_atomic_load_i32, snuk_atomic_store_i32
PUBLIC snuk_atomic_exchange_i32, snuk_atomic_compare_exchange_strong_i32
PUBLIC snuk_atomic_fetch_add_i32, snuk_atomic_fetch_sub_i32
PUBLIC snuk_atomic_fetch_or_i32, snuk_atomic_fetch_xor_i32
PUBLIC snuk_atomic_fetch_and_i32

PUBLIC snuk_atomic_load_i64, snuk_atomic_store_i64
PUBLIC snuk_atomic_exchange_i64, snuk_atomic_compare_exchange_strong_i64
PUBLIC snuk_atomic_fetch_add_i64, snuk_atomic_fetch_sub_i64
PUBLIC snuk_atomic_fetch_or_i64, snuk_atomic_fetch_xor_i64
PUBLIC snuk_atomic_fetch_and_i64

PUBLIC snuk_atomic_load_u8, snuk_atomic_store_u8
PUBLIC snuk_atomic_exchange_u8, snuk_atomic_compare_exchange_strong_u8
PUBLIC snuk_atomic_fetch_add_u8, snuk_atomic_fetch_sub_u8
PUBLIC snuk_atomic_fetch_or_u8, snuk_atomic_fetch_xor_u8
PUBLIC snuk_atomic_fetch_and_u8

PUBLIC snuk_atomic_load_u16, snuk_atomic_store_u16
PUBLIC snuk_atomic_exchange_u16, snuk_atomic_compare_exchange_strong_u16
PUBLIC snuk_atomic_fetch_add_u16, snuk_atomic_fetch_sub_u16
PUBLIC snuk_atomic_fetch_or_u16, snuk_atomic_fetch_xor_u16
PUBLIC snuk_atomic_fetch_and_u16

PUBLIC snuk_atomic_load_u32, snuk_atomic_store_u32
PUBLIC snuk_atomic_exchange_u32, snuk_atomic_compare_exchange_strong_u32
PUBLIC snuk_atomic_fetch_add_u32, snuk_atomic_fetch_sub_u32
PUBLIC snuk_atomic_fetch_or_u32, snuk_atomic_fetch_xor_u32
PUBLIC snuk_atomic_fetch_and_u32

PUBLIC snuk_atomic_load_u64, snuk_atomic_store_u64
PUBLIC snuk_atomic_exchange_u64, snuk_atomic_compare_exchange_strong_u64
PUBLIC snuk_atomic_fetch_add_u64, snuk_atomic_fetch_sub_u64
PUBLIC snuk_atomic_fetch_or_u64, snuk_atomic_fetch_xor_u64
PUBLIC snuk_atomic_fetch_and_u64

PUBLIC snuk_atomic_flag_test_and_set_explicit
PUBLIC snuk_atomic_flag_clear_explicit
PUBLIC snuk_atomic_flag_load_explicit

PUBLIC snuk_memory_fence

PRE_ATOMIC_LOAD_FENCE MACRO memory_order
	LOCAL no_fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne no_fence
	mfence
no_fence:
ENDM

POST_ATOMIC_LOAD_FENCE MACRO memory_order
	LOCAL fence, done
	cmp memory_order, 1 ; SNUK_MEMORY_ORDER_ACQUIRE
	je fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne done
fence:
	lfence
done:
ENDM

PRE_ATOMIC_STORE_FENCE MACRO memory_order
	LOCAL fence, done
	cmp memory_order, 2 ; SNUK_MEMORY_ORDER_RELEASE
	je fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne done
fence:
	sfence
done:
ENDM

POST_ATOMIC_STORE_FENCE MACRO memory_order
	LOCAL no_fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne no_fence
	mfence
no_fence:
ENDM

PRE_ATOMIC_RMW_FENCE MACRO memory_order
	LOCAL store_fence, done
	cmp memory_order, 2 ; SNUK_MEMORY_ORDER_RELEASE
	je store_fence
	cmp memory_order, 3 ; SNUK_MEMORY_ORDER_ACQ_REL
	je store_fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne done
	mfence
	jmp done
store_fence:
	sfence
done:
ENDM

POST_ATOMIC_RMW_FENCE MACRO memory_order
	LOCAL load_fence, done
	cmp memory_order, 1 ; SNUK_MEMORY_ORDER_ACQUIRE
	je load_fence
	cmp memory_order, 3 ; SNUK_MEMORY_ORDER_ACQ_REL
	je load_fence
	cmp memory_order, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne done
	mfence
	jmp done
load_fence:
	lfence
done:
ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; LOAD OPERATIONS ;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, memory_order
snuk_atomic_load_i8 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov al, byte ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_i8 ENDP

; obj, memory_order
snuk_atomic_load_i16 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov ax, word ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_i16 ENDP

; obj, memory_order
snuk_atomic_load_i32 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov eax, dword ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_i32 ENDP

; obj, memory_order
snuk_atomic_load_i64 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov rax, qword ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_i64 ENDP

; obj, memory_order
snuk_atomic_load_u8 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov al, byte ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_u8 ENDP

; obj, memory_order
snuk_atomic_load_u16 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov ax, word ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_u16 ENDP

; obj, memory_order
snuk_atomic_load_u32 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov eax, dword ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_u32 ENDP

; obj, memory_order
snuk_atomic_load_u64 PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov rax, qword ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_load_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; STORE OPERATIONS ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_store_i8 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov byte ptr [rcx], dl
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_i8 ENDP

; obj, value, memory_order
snuk_atomic_store_i16 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov word ptr [rcx], dx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_i16 ENDP

; obj, value, memory_order
snuk_atomic_store_i32 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov dword ptr [rcx], edx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_i32 ENDP

; obj, value, memory_order
snuk_atomic_store_i64 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov qword ptr [rcx], rdx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_i64 ENDP

; obj, value, memory_order
snuk_atomic_store_u8 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov byte ptr [rcx], dl
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_u8 ENDP

; obj, value, memory_order
snuk_atomic_store_u16 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov word ptr [rcx], dx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_u16 ENDP

; obj, value, memory_order
snuk_atomic_store_u32 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov dword ptr [rcx], edx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_u32 ENDP

; obj, value, memory_order
snuk_atomic_store_u64 PROC
	PRE_ATOMIC_STORE_FENCE r8d
	mov qword ptr [rcx], rdx
	POST_ATOMIC_STORE_FENCE r8d
	ret
snuk_atomic_store_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; EXCHANGE OPERATIONS ;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_exchange_i8 PROC
	mov al, dl
	lock xchg al, byte ptr [rcx]
	ret
snuk_atomic_exchange_i8 ENDP

; obj, value, memory_order
snuk_atomic_exchange_i16 PROC
	mov ax, dx
	lock xchg ax, word ptr [rcx]
	ret
snuk_atomic_exchange_i16 ENDP

; obj, value, memory_order
snuk_atomic_exchange_i32 PROC
	mov eax, edx
	lock xchg eax, dword ptr [rcx]
	ret
snuk_atomic_exchange_i32 ENDP

; obj, value, memory_order
snuk_atomic_exchange_i64 PROC
	mov rax, rdx
	lock xchg rax, qword ptr [rcx]
	ret
snuk_atomic_exchange_i64 ENDP

; obj, value, memory_order
snuk_atomic_exchange_u8 PROC
	mov al, dl
	lock xchg al, byte ptr [rcx]
	ret
snuk_atomic_exchange_u8 ENDP

; obj, value, memory_order
snuk_atomic_exchange_u16 PROC
	mov ax, dx
	lock xchg ax, word ptr [rcx]
	ret
snuk_atomic_exchange_u16 ENDP

; obj, value, memory_order
snuk_atomic_exchange_u32 PROC
	mov eax, edx
	lock xchg eax, dword ptr [rcx]
	ret
snuk_atomic_exchange_u32 ENDP

; obj, value, memory_order
snuk_atomic_exchange_u64 PROC
	mov rax, rdx
	lock xchg rax, qword ptr [rcx]
	ret
snuk_atomic_exchange_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; COMPARE EXCHANGE OPERATIONS ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_i8 PROC
	mov al, byte ptr [rdx]
	lock cmpxchg byte ptr [rcx], r8b
	mov byte ptr [rdx], al
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_i8 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_i16 PROC
	mov ax, word ptr [rdx]
	lock cmpxchg word ptr [rcx], r8w
	mov word ptr [rdx], ax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_i16 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_i32 PROC
	mov eax, dword ptr [rdx]
	lock cmpxchg dword ptr [rcx], r8d
	mov dword ptr [rdx], eax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_i32 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_i64 PROC
	mov rax, qword ptr [rdx]
	lock cmpxchg qword ptr [rcx], r8
	mov qword ptr [rdx], rax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_i64 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_u8 PROC
	mov al, byte ptr [rdx]
	lock cmpxchg byte ptr [rcx], r8b
	mov byte ptr [rdx], al
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_u8 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_u16 PROC
	mov ax, word ptr [rdx]
	lock cmpxchg word ptr [rcx], r8w
	mov word ptr [rdx], ax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_u16 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_u32 PROC
	mov eax, dword ptr [rdx]
	lock cmpxchg dword ptr [rcx], r8d
	mov dword ptr [rdx], eax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_u32 ENDP

; obj, expect, value, success, fail
snuk_atomic_compare_exchange_strong_u64 PROC
	mov rax, qword ptr [rdx]
	lock cmpxchg qword ptr [rcx], r8
	mov qword ptr [rdx], rax
	jne not_equal
	mov al, 1
	ret
not_equal:
	xor al, al
	ret
snuk_atomic_compare_exchange_strong_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FETCH ADD OPERATIONS ;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_fetch_add_i8 PROC
	mov al, dl
	lock xadd byte ptr [rcx], al
	ret
snuk_atomic_fetch_add_i8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_i16 PROC
	mov ax, dx
	lock xadd word ptr [rcx], ax
	ret
snuk_atomic_fetch_add_i16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_i32 PROC
	mov eax, edx
	lock xadd dword ptr [rcx], eax
	ret
snuk_atomic_fetch_add_i32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_i64 PROC
	mov rax, rdx
	lock xadd qword ptr [rcx], rax
	ret
snuk_atomic_fetch_add_i64 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_u8 PROC
	mov al, dl
	lock xadd byte ptr [rcx], al
	ret
snuk_atomic_fetch_add_u8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_u16 PROC
	mov ax, dx
	lock xadd word ptr [rcx], ax
	ret
snuk_atomic_fetch_add_u16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_u32 PROC
	mov eax, edx
	lock xadd dword ptr [rcx], eax
	ret
snuk_atomic_fetch_add_u32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_add_u64 PROC
	mov rax, rdx
	lock xadd qword ptr [rcx], rax
	ret
snuk_atomic_fetch_add_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FETCH SUB OPERATIONS ;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_fetch_sub_i8 PROC
	mov al, dl
	neg al
	lock xadd byte ptr [rcx], al
	ret
snuk_atomic_fetch_sub_i8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_i16 PROC
	mov ax, dx
	neg ax
	lock xadd word ptr [rcx], ax
	ret
snuk_atomic_fetch_sub_i16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_i32 PROC
	mov eax, edx
	neg eax
	lock xadd dword ptr [rcx], eax
	ret
snuk_atomic_fetch_sub_i32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_i64 PROC
	mov rax, rdx
	neg rax
	lock xadd qword ptr [rcx], rax
	ret
snuk_atomic_fetch_sub_i64 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_u8 PROC
	mov al, dl
	neg al
	lock xadd byte ptr [rcx], al
	ret
snuk_atomic_fetch_sub_u8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_u16 PROC
	mov ax, dx
	neg ax
	lock xadd word ptr [rcx], ax
	ret
snuk_atomic_fetch_sub_u16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_u32 PROC
	mov eax, edx
	neg eax
	lock xadd dword ptr [rcx], eax
	ret
snuk_atomic_fetch_sub_u32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_sub_u64 PROC
	mov rax, rdx
	neg rax
	lock xadd qword ptr [rcx], rax
	ret
snuk_atomic_fetch_sub_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FETCH OR OPERATIONS ;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_fetch_or_i8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	or r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_i8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_i16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	or r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_i16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_i32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	or r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_i32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_i64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	or r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_i64 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_u8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	or r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_u8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_u16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	or r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_u16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_u32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	or r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_u32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_or_u64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	or r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_or_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FETCH XOR OPERATIONS ;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_fetch_xor_i8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	xor r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_i8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_i16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	xor r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_i16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_i32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	xor r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_i32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_i64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	xor r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_i64 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_u8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	xor r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_u8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_u16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	xor r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_u16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_u32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	xor r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_u32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_xor_u64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	xor r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_xor_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FETCH AND OPERATIONS ;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, value, memory_order
snuk_atomic_fetch_and_i8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	and r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_i8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_i16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	and r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_i16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_i32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	and r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_i32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_i64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	and r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_i64 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_u8 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov al, byte ptr [rcx]

retry:
	mov r10b, al
	and r10b, dl
	lock cmpxchg byte ptr [rcx], r10b
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_u8 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_u16 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov ax, word ptr [rcx]

retry:
	mov r10w, ax
	and r10w, dx
	lock cmpxchg word ptr [rcx], r10w
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_u16 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_u32 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov eax, dword ptr [rcx]

retry:
	mov r10d, eax
	and r10d, edx
	lock cmpxchg dword ptr [rcx], r10d
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_u32 ENDP

; obj, value, memory_order
snuk_atomic_fetch_and_u64 PROC
	PRE_ATOMIC_RMW_FENCE r8d

	mov rax, qword ptr [rcx]

retry:
	mov r10, rax
	and r10, rdx
	lock cmpxchg qword ptr [rcx], r10
	jne retry

	POST_ATOMIC_RMW_FENCE r8d
	ret
snuk_atomic_fetch_and_u64 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MEMORY FENCE FUNCTION ;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; memory_order of fence
snuk_memory_fence PROC
	cmp ecx, 0 ; SNUK_MEMORY_ORDER_NONE
	je done
	cmp ecx, 1 ; snuk_EMMORY_ORDER_ACQUIRE
	je load_fence
	cmp ecx, 2; SNUK_MEMORY_ORDER_RELEASE
	je store_fence
	cmp ecx, 3 ; SNUK_MEMORY_ORDER_ACQ_REL
	je full_fence
	cmp ecx, 4 ; SNUK_MEMORY_ORDER_TOTAL_ORDER
	jne done
full_fence:
	mfence
	ret
store_fence:
	sfence
	ret
load_fence:
	lfence
done:
	ret
snuk_memory_fence ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; ATOMIC FLAG OPERATIONS ;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; obj, memory_order
snuk_atomic_flag_test_and_set_explicit PROC
	mov al, 1
	lock xchg al, byte ptr [rcx]
	ret
snuk_atomic_flag_test_and_set_explicit ENDP

; obj, memory_order
snuk_atomic_flag_clear_explicit PROC
	PRE_ATOMIC_RMW_FENCE edx
	mov byte ptr [rcx], 0
	POST_ATOMIC_RMW_FENCE edx
	ret
snuk_atomic_flag_clear_explicit ENDP

; obj, memory_order
snuk_atomic_flag_load_explicit PROC
	PRE_ATOMIC_LOAD_FENCE edx
	mov al, byte ptr [rcx]
	POST_ATOMIC_LOAD_FENCE edx
	ret
snuk_atomic_flag_load_explicit ENDP

END
