; This code is meant to serve as a calculator that uses reverse polish notation and includes functions such as: addition, subtraction, multiplication, division, and exponents.
;However, it does not accept negative numbers from the user and division nor exponents have the capability to compute negative numbers. 
;The calculator utilizes a stack to store the operands, and henever the program recieves an appropriate opperator, it pops the last two digits from the stack and applies said operation.
;Once the operation is complete, the program pushes the result back into the stack. 
;However, if there are less tahn two values in the stack when an operator is called, we experience a stack underflow and an "Invalid Expression" is printed.
;"Invalid expression" can be printed if the user inputs a character that is neither an operand nor an operator, and if there is a stack overflow where the user inputs too many operands into the stack.
;If these "Invalid expressions" are not triggered, the program will continue to loop and accept user input untill it recieves "=" or triggers an "Invalid Expression".
;If the user types in an equals, then the program checks if there is only one value in the stack and then goes to print said value in hexadecimal if there is one value.
;However, if there is more than one value in the stack then we once again recieve "Invalid Expression".
; In order to print the value in the stack when equals is called,it takes the value stored at x4000(STACK_START) and stores it in R3 and R5.
;It then takes one hex digit at a time from R3 and transfers it over to R0.
; This is done by left shifting R3 four times and storing the MSB each time into R0.
;Once this happens 4 times, it will check if R0 is a hex value inbetween 0 and 9 or a character from A to F. 
;Once it knows this value, it can apply the appropriate ASCII value offset and print the character.
;This process will repeat 4 times to get the full hex value for the result.

;R0 - this is used to hold the ASCII character that the user inputted and that should be printed to the monitor 
;R1 - this is used as a counter to check if 0, 1, or, 2 negative numbers have been popped in the mulitplication and division subroutine. It also acts as a counter for the loop in the exponent subroutine 
;R2 - this is used to check if a valid "=" has been called. Afterwards it then acts as a counter for the innerloop for PRINT_HEX
;R3 - this is used to hold the address of STACK_START and STACK_END in the push and pop subroutines. It also holds the value of the final result for PRINT_HEX and is the one that is shifted. It also holds the 1st digit that is popped in the stack for each operator subroutine. 
;R4 - this is uesed to hold the address of STACK_TOP in the push and pop subroutines. It also holds the address of STACK_START and acts as a counter for the outerloop in PRINT_HEX. It also holds the 2nd digit that is popped in stack for each operator subroutine. 
;R5 - This is used to check if an "Invalid Expression" needs to be printed. If the value in R5 is 1 then there is an error and it needs to printed. At the end of the program, if Invalid Expression was not called then it holds the final result
;R6 - This is used as a temp register for miscelaneous calcualtions
;R7 - This is used to store the PC when jumping to and returning from subroutines
.ORIG x3000

;your code goes here
			AND R2,R2,#0  ; sets R2 as 0
	
CHARLOOP  	
			GETC 		  ; place user input into r0 as ASCCI value
			OUT			  ; echoes user input to monitor

			LD R6,SPACE_N ; loads the negative ASCII value of "SPACE" into R6 
			ADD R6,R0,R6  ; subtracts the value of R0 by R6 and stores in R6
			BRz CHARLOOP  ; if R0 is "SPACE" then loop back to CHARLOOP

		
	  		JSR EVALUATE    ; jump to EVALUATE
			ADD R5,R5,#0 	; load the condition codes for R5
			BRp INVALID     ; if there is a one in R5 go to invalid
			ADD R2,R2,#0 	; load the condition codes for R2
			BRp PR_VALID    ; if there is a one in R2 then we load the result into r5 and print
			BRnzp CHARLOOP	; unconditionaly return to CHARLOOP
			
INVALID 	LEA R0,END_STR  ; places the starting address of END_STR into R0
			PUTS 			; prints the string starting at memory address in R0
			BRnzp DONE 		; skip to DONE unconditionally

PR_VALID JSR PRINT_HEX      ; jump to PRINT_HEX
		 BRnzp DONE 		; unconditionally skip to halt


DONE 		HALT

EQUALS_N 	.FILL xFFC3 ; holds ASCII value for "=" but inverted
SPACE_N	    .FILL xFFE0 ; holds ASCII value for SPACE but inverted
NUMB_CHECK  .FILL xFFC6 ; holds ASCII value for ":" but inverted
MULT_CHECK 	.FILL xFFD6 ; holds ASCII value for "*" but inverted
DIV_CHECK 	.FILL xFFD1 ; holds ASCII value for "/" but inverted
ADD_CHECK 	.FILL xFFD5 ; holds ASCII value for "+" but inverted
MIN_CHECK 	.FILL xFFD3 ; holds ASCII value for "-" but inverted
POW_CHECK 	.FILL xFFA2 ; holds ASCII value for "^" but inverted
NUMB_CONV   .FILL xFFD0 ; holds the inverse value of x30
END_STR 	.STRINGZ "Invalid Expression" 		; this is string that is printed if there is an underflow or there are multiple values in the stack and "=" is called

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;R3- value to print in hexadecimal
PRINT_HEX   
		LD R4,STACK_START ; Loads R4 with the memory address of R4
		LDR R5,R4,#0    ; Loads R5 with the value stored at the memory address held in R4
		LDR R3,R4,#0 	; Loads R3 with the value stored at the memory address held in R3
		LD 	R4,FOUR		; loads R4 with 4 to act as hexCounter (counter for the hexvalue)

OUTLOOP BRz END_HEX  	; if hexCounter reaches zero then return
		AND R0,R0,#0	; resets R0 to 0
		LD 	R2,FOUR 	; loads R2 with 4 to act as binaryCounter (counter for the binary value)

INLOOP  BRz FIN         ; if R2 is zero, go to FIN
		ADD R0,R0,R0 	; left shifts R0
		ADD R3,R3,#0 	; loads condition codes based on R3
		BRzp ZERO  		; checks the MSB of R3
		ADD R0,R0,#1	; adds 1 to R0 if MSB of R3 is 1
ZERO 	ADD R3,R3,R3 	; left shifts R3
		ADD R2,R2,#-1 	; decreases binaryCounter by 1
		BRnzp INLOOP	; unconditionally return to INLOOP

FIN 	AND R6,R6,#0	; resets R6 to 0
		ADD R6,R0,#-9 	; checks if R0 has a value greater than 9
		BRp LETTER 		; if R6 is positive then R0 has a value greater than 9

		LD R6,HEX0		; loads R6 with the ACSII value for 0
		ADD R0,R0,R6 	; adds x30 to R0 in order to get the ASCII value stored in R0
		BRnzp PRINT		; unconditionally go to PRINT

LETTER 
		LD R6,HEXA 		; loads R6 with the ASCII value for A
		ADD R0,R0,R6 	; adds the ASCII value of A to the value in R0
		ADD R0,R0,#-10 	; decreases the content of R0 by 10 in order to get the ASCII value of the original value 

PRINT 	ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		OUT  			; prints out the binary value of R0 as hex
		LD R7, EVAL_SaveR7 ; restores the value of R7
		ADD R4,R4,#-1	; decreases hexCounter by 1
		BRnzp OUTLOOP 	; unconditionally return to OUTLOOP

END_HEX RET

INVERSE 	.FILL xFFE5 ; holds the value of -27
HEXA 		.FILL x41 	; holds the ASCII value for A
HEX0 		.FILL x30 	; holds the ASCII value for 0
FOUR 		.FILL x4 	; holds value of 4

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;R0 - character input from keyboard
;R6 - current numerical output
;
;
EVALUATE
			LD R6,EQUALS_N  ; loads the negative ASCII value of "=" into R6
			ADD R6,R0,R6    ;  subtracts the value of R0 by R6 and stores in R6
			BRnp POW_START 	;  if the value is negative/positve, then we don't have "=" so skip to next check
			BRz EQUAL_START ; if zero we have "=" so go to EQUAL_START

POW_START	LD R6,POW_CHECK ; loads r6 with the negative ASCII value of "^"
			ADD R6,R0,R6  	; subtracts the value of R0 by R6 and stores in R6
			BRnp DIV_START	; if the value is negative/positve, then we don't have an exponent so skip to next check
			ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR EXP            ; jump to EXP
			LD R7, EVAL_SaveR7 ; restores the value of R7
		    BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

DIV_START	LD R6,DIV_CHECK ; loads r6 with the negative ASCII value of "/"
			ADD R6,R0,R6  	; subtracts the value of R0 by R6 and stores in R6
			BRp NUMB_START	; if the value is postive, then we have a number that goes to push
			BRn ADD_START	; if the value is negative, then we don't have a divison so skip to next check
			ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine			
			JSR DIV 		   ; jump to division
			LD R7, EVAL_SaveR7 ; restores the value of R7
		    BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL
	
ADD_START	LD R6,ADD_CHECK ; loads r6 with the negative ASCII value of "+"
		    ADD R6,R0,R6  	; subtracts the value of R0 by R6 and stores in R6
			BRnp MULT_START	; if the value is negative/positve, then we have do not have an addition so skip to next check
			ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR PLUS           ; jump to PLUS
			LD R7, EVAL_SaveR7 ; restores the value of R7
		    BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

MULT_START LD R6,MULT_CHECK ; loads r6 with the negative ASCII value of "*"
		   ADD R6,R0,R6  	; subtracts the value of R0 by R6 and stores in R6
	       BRnp MIN_START	; if the value is negative/positive, then we don't have a multiply so skip to next check
		   ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   JSR MUL            ; jump to MUL
		   LD R7, EVAL_SaveR7 ; restores the value of R7
		   BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

MIN_START   LD R6,MIN_CHECK ; loads r6 with the negative ASCII value of "-"
			ADD R6,R0,R6  	; subtracts the value of R0 by R6 and stores in R6
			BRnp NOT_VALID	; if the value is negative/positive, then we don't have a subtract so skip to END_EVAL
			ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR MIN			    ; jump to MIN	
			LD R7, EVAL_SaveR7 ; restores the value of R7
			BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

EQUAL_START AND R5,R5,#0
			LD R3, STACK_START	; loads the memory address that STACK_START points to
			LD R4, STACK_TOP	; loads the memory address that STACK_TOP points to
			ADD R3,R3,#-1 		; subtracts 1 from R3
			NOT R3, R3			; inverts R3
			ADD R3, R3, #1		; adds 1 to make R3 the negative of the original value
			ADD R3, R3, R4		; Subtracts R4 from R3 and places it into R3
			BRz ONE_VAL		    ; if the value is zero then there is only 1 value left in the stack
            ADD R5,R5,#1        ; load a one into R5 to denote an error took place
ONE_VAL     AND R2,R2,#0 		; sets R2 to 0, 
			ADD R2,R2,#1	    ; loads a one into R4 which acts as a check for a valid "=" statement
			BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

NUMB_START  LD R6, NUMB_CONV     ; loads R6 with -x30
			ADD R0,R0,R6 		;converts the ASCII value of the number into its hex value
			ST R7, EVAL_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR PUSH
		   	LD R7, EVAL_SaveR7 ; restores the value of R7
			BRnzp END_EVAL 	   ; skips unconditionally to END_EVAL

NOT_VALID	AND R5,R5,#0 	   ; sets R5 to zero 
			ADD R5,R5,#1 	   ; if none of the previous arguements are triggered and we reach this command. An invald input was called and we now print invalid expression 
		
END_EVAL 	RET                ; return


;your code goes here


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;input R3, R4
;out R0
PLUS	
			ST R3, PUSH_SaveR3	;save R3
			ST R4, PUSH_SaveR4	;save R4
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP          ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_ADD	     ; if R5 is 1 then we immediately skip to the end 
			ADD R3,R0,#0 	 ; stores the value of R0 into R3
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP			 ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_ADD		 ; if R5 is 1 then we immediately skip to the end 
			ADD R4,R0,#0 	 ; stores the value of R0 into R4
			AND R0,R0,#0 	 ; sets R0 to 0
			ADD R0,R4,R3 	 ; adds R4 and R3 and places it into R0
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR PUSH         ;jump to push
		   	LD R7, OP_SaveR7 ; restores the value of R7
			LD R3, POP_SaveR3 ; restore the value of R3
			LD R4, POP_SaveR4 ; restore the value of R4
END_ADD 	RET
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;input R3, R4
;out R0
MIN	
			ST R3, PUSH_SaveR3	;save R3
			ST R4, PUSH_SaveR4	;save R4
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP          ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_MIN	     ; if R5 is 1 then we immediately skip to the end 
			ADD R3,R0,#0 	 ; stores the value of R0 into R3
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP			 ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_MIN	     ; if R5 is 1 then we immediately skip to the end 
			ADD R4,R0,#0 	 ; stores the value of R0 into R4
			NOT R3,R3        ; inverts R3
			ADD R3,R3,#1 	 ; adds 1 to R3 to get the negative value of the original R3 value
			AND R0,R0,#0 	 ; sets R0 to 0
			ADD R0,R4,R3 	 ; adds R4 and R3 and places it into R0
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR PUSH
		   	LD R7, OP_SaveR7 ; restores the value of R7
			LD R3, POP_SaveR3 ; restore the value of R3
			LD R4, POP_SaveR4 ; restore the value of R4
END_MIN 	RET
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;input R3, R4
;out R0
MUL	
			ST R3, PUSH_SaveR3	;save R3
			ST R4, PUSH_SaveR4	;save R4
			ST R6, OP_SaveR6	;save R6
			ST R1, OP_SaveR1	;save R1
			AND R1,R1,#0 	 ; sets R1 to 0 and is used as a counter to check if there is a negative
			AND R6,R6,#0     ; sets R6 to 0
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP			 ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_MUL		 ; if R5 is 1 then we immediately skip to the end 
			ADD R0,R0,#0     ; loads condition codes for R0

			BRzp NO_NEGAT1   ; if the value pulled from the Stack is zero/positive then skip to NO_Negat1
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR NEGAT 		 ; if there is a negative jump to NEGAT
			LD R7, OP_SaveR7 ; restores the value of R7

NO_NEGAT1	ADD R3,R0,#0 	 ; stores the value of R0 into R3
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP          ; pop value from stack into R0
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_MUL		 ; if R5 is 1 then we immediately skip to the end 
			ADD R0,R0,#0     ; loads condition code for R0

			BRzp NO_NEGAT2   ; if the value pulled from the Stack is zero/positive then skip to NO_Negat2
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR NEGAT 		 ; if there is a negative jump to NEGAT
			LD R7, OP_SaveR7 ; restores the value of R7

NO_NEGAT2	ADD R4,R0,#0 	 ; stores the value of R0 into R4
			AND R0,R0,#0 	 ; sets R0 to 0
			
MULT_LOOP   ADD R6,R6,R4 	 ; adds R4 to R6 and stores it in R6
			ADD R3,R3,#-1 	 ; decrements R3 by 1
			BRp MULT_LOOP    ; if R3 is still positive then repeat the loop 

			ADD R1,R1,#0     ; loads condition code for R1
			BRz NO_SWITCH    ; if the value in R1 then we don't need to invert the result and skip to NO_SWITCH
			NOT R6,R6 	     ; invert R6
			ADD R6,R6,#1 	 ; add 1 to R6 in order to make it the opposite value

NO_SWITCH	ADD R0,R6,#0 	  ; sets the value of R0 to R6
			ST R7, OP_SaveR7  ; saves the value of R7 so it is not lost in the subroutine
		   	JSR PUSH          ; jump to push the result
		   	LD R7, OP_SaveR7  ; restores the value of R7
			LD R3, POP_SaveR3 ; restore the value of R3
			LD R4, POP_SaveR4 ; restore the value of R4
			LD R6, OP_SaveR6  ; restore the value of R6
			LD R1, OP_SaveR1  ; restore the value of R1

END_MUL 	RET


NEGAT      ; used to check if there is a negative number in the stack for multiply (takes R0/R1 and outputs R1/R0)
			NOT R0,R0     ; invert R0
			ADD R0,R0,#1  ; adds 1 to R0 in order to make it positive
			ADD R1,R1,#0  ; load condition codes for R1
			BRz ONE_NEGAT ; checks if this is the 2nd negative number to multiply
			ADD R1,R1,#-1 ; if there are 2 negative numbers then we set counter to 0
			BRnzp N_END   ; unconditionally return
ONE_NEGAT	ADD R1,R1,#1  ;  if there is 1 negative number then we set counter to 1
 N_END      RET       
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;input R3, R4
;out R0
DIV	
			ST R3, PUSH_SaveR3	;save R3
			ST R4, PUSH_SaveR4	;save R4
			ST R6, OP_SaveR6	;save R6
			ST R1, OP_SaveR1    ;save R1
			AND R1,R1,#0     ; sets R1 to 0
			AND R6,R6,#0     ; sets R6 to 0
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_DIV			 ; if R5 is 1 then we immediately skip to the end 
			ADD R3,R0,#0 	 ; stores the value of R0 into R3
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP			 ; JUMPS to pop
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_DIV		 ; if R5 is 1 then we immediately skip to the end 
			ADD R4,R0,#0 	 ; stores the value of R0 into R4
			AND R0,R0,#0 	 ; sets R0 to 0
			ADD R6,R3,#0     ; sets R6 to R3
			NOT R6,R6        ; inverts R6
			ADD R6,R6,#1     ; adds 1 to R6 to make it the negative value of the value stored in R6
			
DIV_LOOP    ADD R4,R4,R6 	 ; subtracts R4 by R6 and stores it in R4
			BRn SKIP  		 ; if the previous result is negative, then this division will have a remainder and will go to SKIP in order to preserve the quotient
			ADD R1,R1,#1     ; increments R1 by 1 for every succesful "subtraction"
			ADD R4,R4,#0 	 ; decrements R3 by 1
			BRp DIV_LOOP     ; if R4 is still positive then repeat the loop 

SKIP	    ADD R0,R1,#0 	 ; sets the value of R0 to R1
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR PUSH 		 ; jump to push
		   	LD R7, OP_SaveR7 ; restores the value of R7
			LD R3, POP_SaveR3 ; restore the value of R3
			LD R4, POP_SaveR4 ; restore the value of R4
			LD R6, OP_SaveR6 ; restore the value of R6
			LD R0, OP_SaveR1 ; restore the value of R0
END_DIV 	RET

STACK_END	.FILL x3FF0	;
STACK_START	.FILL x4000	;
STACK_TOP	.FILL x4000	;
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;input R3, R4
;out R0
EXP	
			ST R3, PUSH_SaveR3	;save R3
			ST R4, PUSH_SaveR4	;save R4
			ST R6, OP_SaveR6	;save R6
			ST R1, OP_SaveR1	;save R6
			AND R1,R1,#0     ; sets R1 to 0
			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_EXP		 ; if R5 is 1 then we immediately skip to the end 
			ADD R3,R0,#0 	 ; stores the value of R0 into R3
			

			ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
		   	JSR POP 		 ; jumps to POP
		   	LD R7, OP_SaveR7 ; restores the value of R7
			ADD R5,R5,#0 	 ; loads the condition code for R5
			BRp END_EXP		 ; if R5 is 1 then we immediately skip to the end 

			ADD R3,R3,#0     ; loads the condition codes for R3
			BRp POSI_EXP     ; if the exponent stored in R3 is a positive number go to POSI_EXP, if its zero continue
			AND R0,R0,#0     ; set R0 to 0
			ADD R0,R0,#1     ; set R0 to 1
			BRnzp SKIP_EXP   ; unconditionally skip to SKIP_EXP

POSI_EXP	ADD R4,R0,#0 	 ; stores the value of R0 into R4
			ADD R3,R3,#-1 	 ; decrements R3 since due to the nature of exponents the counter needs to be one less than that of the exponent
			
			
EXP_LOOP	ADD R1,R0,#0 	 ; stores the value of R0 into R1	
			AND R6,R6,#0     ; sets R6 to 0	
M1_LOOP     ADD R6,R6,R4 	 ; adds R4 to R6 and stores it in R6
			ADD R1,R1,#-1 	 ; decrements R3 by 1
			BRp M1_LOOP      ; if R3 is still positive then repeat the loop 
			ADD R4,R6,#0     ; set R4 to R6
			ADD R3,R3,#-1 	 ; decrement R3
			BRp EXP_LOOP 	 ; if R3 is still positive then return to EXP_LOOP

			AND R0,R0,#0 	 ; sets R0 to 0
			ADD R0,R6,#0 	 ; sets the value of R0 to R6
SKIP_EXP	ST R7, OP_SaveR7 ; saves the value of R7 so it is not lost in the subroutine
			JSR PUSH 	     ; jumps to push
		   	LD R7, OP_SaveR7 ; restores the value of R7
			LD R3, POP_SaveR3 ; restore the value of R3
			LD R4, POP_SaveR4 ; restore the value of R4
			LD R6, OP_SaveR6 ; restore the value of R6
END_EXP 	RET


OP_SaveR1	.BLKW #1    ;
POP_SaveR3	.BLKW #1	;
POP_SaveR4	.BLKW #1	;
OP_SaveR6	.BLKW #1    ;
EVAL_SaveR7 .BLKW #1 	;
OP_SaveR7 	.BLKW #1 	;
	
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
	NOT R3, R3			;
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
	NOT R3, R3			;
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

.END


