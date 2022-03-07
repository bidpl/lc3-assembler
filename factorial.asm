.ORIG x3000
 
            
INNERLOOP   
            ADD   R5,R5,R1            ; Increment sum

.END

; This program calculates X! (X factorial). It can calculate
;   different numbers (4!, 6!, etc.) by changing the value of the first memory
;   location at the bottom of the code. It is currently set up to calculate
;   5!. The program does not account for zero or negative numbers as input.
 
; This program primarily uses registers in the following manner:
; R0 contains 0 (registers contain zero after reset)
; R1 contains multiplication result (6x5 = 30, 30x4 = 120, etc)
; R2 contains -1
; R3 contains counter for outer loop
; R4 contains counter for inner loop
; R5 contains current sum
 