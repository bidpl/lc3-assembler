.ORIG x3000
	
START	GETC
	JSR	EVALUATE
	BRzp	START
	HALT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;R3- value to print in hexadecimal

;R4- Loop counter (4 characters)
;R5- Loop couner (4 bits/char)
;R6- Temp register

;R0- IO
PRINT_HEX

;Save Registers
	ST	R0, PRHEX_R0
	ST	R3, PRHEX_R3
	ST	R4, PRHEX_R4
	ST	R5, PRHEX_R5
	ST	R6, PRHEX_R6
	ST	R7, PRHEX_R7

	AND	R4, R4, #0	; Clear R4
	ADD	R4, R4, #4	; Set R4 to 4 (for 4 digits)

BITLOOP	AND	R5, R5, #0	; Clear R4
	ADD	R5, R5, #4	; Set R4 to 4 (4 bits per hex digit)
	AND	R0, R0, #0	; Clear R0

DIGLOOP	; This loop transfers 4 MSB from data to R0 to be printed
	ADD	R0, R0, R0	; Leftshift R0
	ADD	R3, R3, #0	; Update CC bits with number of occurences data
	BRzp	NO_ADD		; if MSB of data 0, keep LSB of R0 0
	ADD	R0, R0, #1	; Set last bit of R0 to 1
NO_ADD	ADD	R3, R3, R3	; Leftshift data
	ADD	R5, R5, #-1	; Decrement bit loop
	BRp	DIGLOOP		; Loop 4 times

	ADD	R0, R0, #-10	; Test of number or letter digit
	BRzp	LETTER

	LD	R6, OFFSET_0	; Load ascii offset for number
	BR	PRINTCH

LETTER	LD	R6, OFFSET_A	; Load ascii offset for letter

PRINTCH	ADD	R0, R0, R6	; Add offset to turn R0 into ascii for correct hex digit
	OUT			; Print it

	ADD	R4, R4, #-1	; Decrement hex digit loop
	BRp	BITLOOP		; Loop 4 times

	LD	R0, PRHEX_R0	; Restore Registers and return
	LD	R3, PRHEX_R3
	LD	R4, PRHEX_R4
	LD	R5, PRHEX_R5
	LD	R6, PRHEX_R6
	LD	R7, PRHEX_R7
	RET
;End PRINT_HEX, start PRINT_HEX data
PRHEX_R0	.BLKW #1
PRHEX_R3	.BLKW #1
PRHEX_R4	.BLKW #1
PRHEX_R5	.BLKW #1
PRHEX_R6	.BLKW #1
PRHEX_R7	.BLKW #1
OFFSET_0	.FILL x003a	;+Ascii value for 0 + 10
OFFSET_A	.FILL x0041	;+Ascii value for A

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;R0 - character input from keyboard
;R6 - current numerical output
;
;R1 - Temp
;R3 - Sub function input 1
;R4 - Sub function input 2
EVALUATE
	ST	R0, EVAL_R0	; Save registers
	ST	R1, EVAL_R1
	ST	R2, EVAL_R2
	ST	R7, EVAL_R7

	OUT

	LD	R1, EVAL_SPACE	; Load additive inverse of space
	ADD	R1, R0, R1	; Add input to it
	BRz	E_DONE		; If space, return

	LD	R1, EVAL_9	; Load additive inverse of 9
	ADD	R1, R0, R1	; Add input to it
	BRp	TRY_EQUAL		; If > than 9, know it's not number
	ADD	R1, R1, #9	; Shift sum over, '0' mapped to #0, '9' to #9
	BRn	TRY_EQUAL

	ADD	R0, R1, #0	; Move R1 to R0 for PUSH subroutine
	JSR	PUSH		; Push operand/number to stack
	BRnzp	E_DONE

TRY_EQUAL
	LD	R1, EVAL_EQUAL	; Load additive inverse of '='
	ADD	R1, R1, R0	; Add input to it
	BRnp	TRY_PLUS

				; if '='
	JSR	POP		; POP value to R1 (to hold final answer)
	ADD	R1, R0, #0	; (mv R0 to R1)
	ADD	R5, R5, #0	; Refresh CC bits with R5 (POP fail?)
	BRp	BAD_EQ		; If no value to pop, there was no input
	JSR	POP		; POP second time to make sure it's the only item in stack
	ADD	R5, R5, #0	; Refresh CC bits with R5
	BRnz	BAD_EQ		; We expect R5 to be 1 (underflow), so if it's not, print invalid input

	ADD	R5, R1, #0	; Copy R1 final answer to R5 (output register)
	ADD	R3, R1, #0	; Copy R1 final answer to R3 to be printed
	JSR	PRINT_HEX	; Print it
	HALT

TRY_PLUS
	LD	R1, EVAL_PLUS	; Load additive inverse of PLUS
	ADD	R1, R1, R0	; Add input to it
	BRnp	TRY_MIN

	JSR	GETARGS
	JSR	PLUS		; Add R3 and R4
	JSR	PUSH		; Push it back to stack
	BRnzp	E_DONE

TRY_MIN
	LD	R1, EVAL_MINUS	; Load additive inverse of '-'
	ADD	R1, R1, R0	; Add input to it
	BRnp	TRY_TIMES

	JSR	GETARGS		; POP 2 values from stack + check for underflow
	JSR	MIN		; Subtract R3 and R4
	JSR	PUSH		; PUSH result to stack
	BRnzp	E_DONE

TRY_TIMES
	LD	R1, EVAL_TIMES	; Load additive inverse of '*'
	ADD	R1, R1, R0	; Add input to it
	BRnp	TRY_DIV

	JSR	GETARGS		; POP 2 values from stack + check for underflow
	JSR	MUL		; Multiply them
	JSR	PUSH		; PUSH result to stack
	BRnzp	E_DONE

TRY_DIV	LD	R1, EVAL_DIV	; Load additive inverse of '/'
	ADD	R1, R1, R0	; Add input to it
	BRnp	TRY_EXP

	JSR	GETARGS		; POP 2 values from stack + check for underflow
	JSR	DIV		; Multiply them
	JSR	PUSH		; PUSH result to stack
	BRnzp	E_DONE

TRY_EXP	LD	R1, EVAL_EXP	; Load additive inverse of '/'
	ADD	R1, R1, R0	; Add input to it
	BRnp	NOT_EXP

	JSR	GETARGS		; POP 2 values from stack + check for underflow
	JSR	EXP		; Exponentiate
	JSR	PUSH		; PUSH result to stack
	BRnzp	E_DONE

NOT_EXP	BRnzp	BAD_IN		; If not number, space, '=', '+', '-', '*', '/', or '^', it's invalid

E_DONE	LD	R0, EVAL_R0	; Restore registers and return
	LD	R1, EVAL_R1
	LD	R2, EVAL_R2
	LD	R7, EVAL_R7
	RET

;;;;;;;;;;;;;;;
; BAD INPUT/EQUALS
; IN: Nothing
; OUT: Nothing
; Prints "=Invalid Expression"/"Invalid Expression" and halts program
; Not really a subroutine, it's BR-ed to and has no ret
BAD_IN	LD	R0, ASCIIEQ	; Load ascii value for '='
	OUT			; Print it
BAD_EQ	LEA	R0, INVALID	; Load pointer to "Invalid Expression" string to R0 (i/o)
	PUTS			; Print it
	HALT			; HALT

ASCIIEQ	.FILL		x003D	; Ascii for '='
INVALID	.STRINGZ	"Invalid Expression"

;;;;;;;;;;;;;;;;
; GET ARGUMENTS
; IN: Nothing
; OUT: R3 (second value off stack), R4 (top value off stack)
; When run, subroutine pops top two values of stack to R3 and R4.
; If stack underflows, print "Invalid Expression" and halts
GETARGS	ST	R5, GETA_R5
	ST	R7, GETA_R7

	JSR	POP
	ADD	R4, R0, #0	; Copy R0 (first POPped value) to R3
	JSR	POP
	ADD	R3, R0, #0	; Copy R0 (second POPped value) to R4

	ADD	R5, R5, #0	; Refresh CC bits with R5 to check for underflow
	BRp	BAD_IN		; If underflow, jump to error, technically bad syntax

	LD	R5, GETA_R5	; Restore registers
	LD	R7, GETA_R7
	RET

GETA_R5	.BLKW #1
GETA_R7	.BLKW #1

EVAL_R0		.BLKW	#1
EVAL_R1		.BLKW	#1
EVAL_R2		.BLKW	#1
EVAL_R7		.BLKW	#1


;your code goes here

EVAL_SPACE	.FILL xFFE0	; Additive inverse of Ascii values
EVAL_EQUAL	.FILL xFFC3
EVAL_PLUS	.FILL xFFD5
EVAL_MINUS	.FILL xFFD3
EVAL_TIMES	.FILL xFFD6
EVAL_DIV	.FILL xFFD1
EVAL_EXP	.FILL xFFA2
EVAL_9		.FILL xFFC7

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;PLUS
;input R3, R4
;out R0
; R0 <- R3+ R4
PLUS	
	ADD R0, R3, R4
	RET
;your code goes here
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; MINUS
;input R3, R4
;out R0
; R0 <- R3 - R4
MIN
	NOT	R4, R4		; Turn R4 into its additive inverse
	ADD	R4, R4, #1
	ADD	R0, R3, R4	; Subtract R4 from from R3 and store into R0
	RET	
;your code goes here
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; MULTIPLY
; input R3, R4
; out R0
; R0 <- R3 * R4
MUL	
;your code goes here
	AND	R0, R0, #0	; Clear R0, will be used to accumulate sum
	ADD	R4, R4, #0	; Refresh CC bits with R4 (2nd input), will be used as loop counter
	BRn	MUL_NEG		; If R4 is negative, negate it and R3. R4 is loop counter so it must be positive/zero

MULLOOP	ADD	R4, R4, #0	; Add R3 to R0 R4 times, Refresh CC bits with R4
	BRz	MUL_END		; If looped R4 times, done
	ADD	R0, R0, R3	; R0 += R3
	ADD	R4, R4, #-1
	BRnzp	MULLOOP	

MUL_NEG	NOT	R3, R3		; Negate R3
	ADD	R3, R3, #1
	NOT	R4, R4		; Negate R4
	ADD	R4, R4, #1	
	BRnzp	MULLOOP		; Go back and start adding/multipling 

MUL_END	RET

	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DIVIDE
; input R3 (divisor), R4 (dividend)
; out R0
; temp R1

; R0 <- R3/R4
DIV
;your code goes here
	ST 	R1, DIV_R1	; Callee-Save R1

	AND	R0, R0, #0	; Clear R0
	ADD	R1, R1, #-1	; Set R1 to -1, used to keep track of negations (-1 = negate answer, 0 = don't)
				; Default is -1 since if dividend is negative, we don't change anything so we need to start assuming negative

	ADD	R3, R3, #0	; Refresh CC bits with R3
	BRzp	D1_POS

	NOT	R3, R3		; If R3 (divisor) is negative, make it positive and flip r1
	ADD	R3, R3, #1
	NOT	R1, R1

D1_POS	ADD	R4, R4, #0	; Refresh CC bits with R4
	BRn	D_LOOP		; We need R4 to be negative, so flip it if positive, do nothing if negative

	NOT	R4, R4		; IF R4 (dividend) is positive, negate it and flip r1
	ADD	R4, R4, #1
	NOT	R1, R1

D_LOOP	ADD	R3, R3, R4	; Subtract dividend from divisor
	BRn	DIV_END		; If remainder is negative, division is done (if zero, wtill need to count that subtraction)

	ADD	R0, R0, #1	; Increment R1 if remainder was positive
	BRnzp	D_LOOP		; Repeat division

DIV_END	ADD	R1, R1, #0	; Refresh CC bits with R1
	BRz	ANS_POS		; If answer doesn't need to be negated (R1 == 0), don't flip it
	NOT	R0, R0		; Negate answer
	ADD	R0, R0, #1

ANS_POS	LD	R1, DIV_R1	; Restore Registers
	RET

DIV_R1 .BLKW	#1
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; EXPONENT
; input R3, R4 
; out R0
; tmp R1
; R0 <- R3 ^ R4
EXP
;your code goes here
	ST	R1, EXP_R1	; Callee save register(s)
	
	AND	R0, R0, #0	; Set R0 to 1 (n^0 = 1)
	ADD	R0, R0, #1

EXPLOOP	ADD	R4, R4, #-1	; Decrement R4 and update CC w/ R4
	BRn	EXP_END		; If exponent loop counter is negative, it was 0 before the decrement (so don't multiply again)

	ADD	R1, R0, #0	; Copy current product to R1 (This loop does product = base * product)
	AND	R0, R0, #0	; Clear R0

EXP_MUL	ADD	R1, R1, #-1	; Adds R3 (base) to R0 (New Product) R1 (old product) times
	BRn	EXPLOOP

	ADD	R0, R0, R3	; R0 (New Product) += R3 (base)
	BRnzp	EXP_MUL		; Branch back to EXPonent_MULtiply loop

EXP_END	LD	R1, EXP_R1	; Restore temp register
	RET

EXP_R1	.BLKW	#1
	
;IN:R0, OUT:R5 (0-success, 1-fail/overflow)
;R3: STACK_END R4: STACK_TOP
;
PUSH	
	ST R3, PUSH_SaveR3	;save R3
	ST R4, PUSH_SaveR4	;save R4
	AND R5, R5, #0		;
	LD R3, STACK_END	;
	LD R4, STACk_TOP	;
	ADD R3, R3, #-1		;
	NOT R3, R3		;
	ADD R3, R3, #1		;
	ADD R3, R3, R4		;
	BRz OVERFLOW		;stack is full
	STR R0, R4, #0		;no overflow, store value in the stack
	ADD R4, R4, #-1		;move top of the stack
	ST R4, STACK_TOP	;store top of stack pointer
	BRnzp DONE_PUSH		;
OVERFLOW
	ADD R5, R5, #1		;
DONE_PUSH
	LD R3, PUSH_SaveR3	;
	LD R4, PUSH_SaveR4	;
	RET


PUSH_SaveR3	.BLKW #1	;
PUSH_SaveR4	.BLKW #1	;


;OUT: R0, OUT R5 (0-success, 1-fail/underflow)
;R3 STACK_START R4 STACK_TOP
;
POP	
	ST R3, POP_SaveR3	;save R3
	ST R4, POP_SaveR4	;save R3
	AND R5, R5, #0		;clear R5
	LD R3, STACK_START	;
	LD R4, STACK_TOP	;
	NOT R3, R3		;
	ADD R3, R3, #1		;
	ADD R3, R3, R4		;
	BRz UNDERFLOW		;
	ADD R4, R4, #1		;
	LDR R0, R4, #0		;
	ST R4, STACK_TOP	;
	BRnzp DONE_POP		;
UNDERFLOW
	ADD R5, R5, #1		;
DONE_POP
	LD R3, POP_SaveR3	;
	LD R4, POP_SaveR4	;
	RET


POP_SaveR3	.BLKW #1	;
POP_SaveR4	.BLKW #1	;
STACK_END	.FILL x3FF0	;
STACK_START	.FILL x4000	;
STACK_TOP	.FILL x4000	;

.END


