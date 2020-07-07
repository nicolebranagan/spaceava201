char beep;

//  Based off of ChibiSound from https://www.chibiakumas.com/6502/platform3.php#LessonP25

chirp(char bop) {
    beep = bop;

    #asm
    pha
    lda _beep
    ChibiSound:			;%NVPPPPPP	N=Noise  V=Volume  P=Pitch
    pha
        lda #5
        sta $0800		;Channel Select (0)
        
        
        
        stz $0804		;Channel On/Write - Set 'Data Write'
                
        ;Define the wave data
        
        ldy #8			;8x4 bytes of wave data
        lda #%00011111	
    ChibiSoundMoreWaves:
        ldx #4			;Write 4 wave bytes 
    ChibiSoundMoreBytes:		
        sta $0806		;Waveform Data (5 bit)
        dex 
        bne ChibiSoundMoreBytes
        eor #%00011111	;Flip all 5 bits of the wave
        dey
        bne ChibiSoundMoreWaves
    
	pla
	bne ChibiSound_NotSilent
    pla
	bra Done
ChibiSound_NotSilent:
	pha
		and #%01000000	;Volume
		lsr a
		lsr a
		lsr a
		ora #%10010111	;Chanel Op - Set 'Play' & Max Vol
		sta $0804		;Channel On/Write
		
		lda #255
		sta $0801		;Main Amplitude Level
		sta $0805		;LR Volume
	pla
	
	pha
		and #%00111100	;Pitch
		lsr a
		lsr a
		sta $0803		;Frequency H
	pla
	pha
		and #%00000011	;Pitch
		clc
		ror a
		ror a
		ror a
		sta $0802		;Frequency L
	pla
	
	bit #%10000000			;Noise
	beq Chibisound_NoNoise
	
	and #%00111110		;Noise freq (5bits only)
	eor #%00111110
	lsr a
	sec
	ror	a				;Top bit to 1
	bra Chibisound_NoiseRet		
Chibisound_NoNoise:	
	lda #0				;Stop the noise
Chibisound_NoiseRet:
	sta $0807			;Noise Enable
	pla
Done:
#endasm
}