; ----------------------------------------------------------------------------
; program iclean_v2.1.pro                              verison 2.1 / 06 oct 05
;
;CREATED: 28 sep 05, Mike Mellon
;
; A baseline program to evaluate image cleaning methods...
;
; ... Reads a channel into IDL memory
; ... attempt some zeroth order calibration (offset, darkcurrent, drift)
;        - apply a reverse-clock-based offset normalization
;        - apply a mask-based fixed pattern normalization
;        - apply a buffer-based offset and drift correction
;        - apply a dark-pixel-based dark current and drift correction
; ... prints some diagnostic statistical information
;
;HISTORY:
; 28 sep 05, Mike, adapted from getchannels amd bits of make summary save...
; 06 sep 05, added reverse clock removal and cleaned up for distribution
;
;TESTED:
; DD MMM YY, name
;
; ----------------------------------------------------------------------------

@NOISE    ; make sure this compiles - it wasn't on it's own for some reason.
Q = 0     ; program diagnostic printing? 0=no 1=yes

; ----------------------------------------------------------------------------
; Start by  reading an image or images.
; ----------------------------------------------------------------------------

FPATH = IMAGE_DIR()      ; find image data directory
FS    = FILESEP()        ; file seperator for this system

TT   = 'Select a channel or group of channels'
FILES = DIALOG_PICKFILE(PATH=FPATH,/READ,FILTER='*.*',TITLE=TT,/MULTIPLE_FILES)
IF FILES[0] EQ '' THEN STOP

NF = N_ELEMENTS(FILES)             ; how many files do we select
FOR JF = 0,NF-1 DO BEGIN           ; look over image files

  FILE = FILES[JF]                 ; select J-th file
  BASENAME = FILEBASENAME(FILE)    ; get some fun file name info
  BASEPATH = FILEDIRNAME(FILE)     ;

  IF Q THEN PRINT,'Reading ',FILE  ; status
  RCHANNEL,FILE,Z,INFO,0           ; read image, silent

  IF Q THEN BEGIN
    PRINT,' ******************************************* '
    PRINT,'TDI  BIN  LINETIME    LINES  COLS'
    PRINT,'---  ---  -----.----  -----  ----'
    F  = '(I3,2X,I3,2X,F8.2,"us",2X,I5,2X,I4)'
    PRINT,INFO.TDI,INFO.BIN,INFO.LINETIME,INFO.NLINE,INFO.NCOL,FORMAT=F
    PRINT,' ******************************************* '
  ENDIF

; ----------------------------------------------------------------------------
; Next, work some numbers to clean the image...
; I attempted to handle bin modes, but it may not be optimal
;
; Note that Z contains the original raw image, step 0 results in Z0,
; step 1 results in Z1, etc...
; ----------------------------------------------------------------------------

  BIN  = INFO.BIN       ; get some commonly used values values
  TDI  = INFO.TDI
  NL   = INFO.NLINE
  NC   = INFO.NCOL
  TRIM = INFO.TRIM

; 0- Reversed offset adjustment
; Determine the relative offset variation from column to column.
; - Average the reversed clocked lines to a column array
; - subtract the array from each column of the image
; - add back the mean reverse clocked value (common offset)

  Z0 = Z           ; define array same size

  LINESTART = 0    ; start at the beginning of reverse clock region
  LINEEND   = 18   ; skip last line to avoid reverse-to-forward transient spike
  AVE_LINES,Z,LINESTART,LINEEND,REV,INOISE
  DC_REV = MEAN(REV)
  FOR I=0L,NL-1 DO BEGIN
    Z0[*,I] = Z[*,I] - REV[*] + DC_REV
  ENDFOR

; 1- Fixed Pattern Correction
; Determine the relative fixed pattern from the mask column to column
; This is similar to cimage except we add back the DC offset to preserve the
; offset and dark current drift.
; - average the mask lines to a column array
; - subtract the array from each column ofthe image
; - add back the mean masked value

  Z1 = Z           ; define array same size

  LINESTART = 20            ; start of mask
  LINEEND   = 20+20/BIN - 1 ; end of mask
  AVE_LINES,Z0,LINESTART,LINEEND,MASK,INOISE
  DC_MASK = MEAN(MASK)
  FOR I=0L,NL-1 DO BEGIN
    Z1[*,I] = Z0[*,I] - MASK[*] + DC_MASK
  ENDFOR


; 2- Drift correction
; Determine the drift in offset and dark current from the buffer and dark columns.
; - Calculate characteristics, offset and dark current vs line number
; - subtract buffer
; - subtract (dark-buffer)*bin to represent dark current

  Z2 = Z           ; define array same size

  STARTCOL = 1                              ; skip first column of buffer
  ENDCOL   = 11                             ; use last column
  AVE_COLS,Z1,STARTCOL,ENDCOL,BARRAY,BNOISE ; compute average buffer line array

  STARTCOL = NC-1 -15 + 4                   ; skip first 5 dark columns
  ENDCOL   = NC-1                           ; use last dark column
  AVE_COLS,Z1,STARTCOL,ENDCOL,DARRAY,DNOISE ; computer average dark line array

  DC = (DARRAY-BARRAY)/BIN                  ; dark current array per image pixel

  FOR I=0L,NL-1 DO BEGIN
    Z2[*,I]=Z1[*,I]-BARRAY[I] - DC[I]*BIN^2 ; correct drift
  ENDFOR

; ----------------------------------------------------------------------------
; Lastly compute some diagnostic statistics to see how we did
; ----------------------------------------------------------------------------

  R0  = 20+(20+TDI)/BIN+30     ; end of ramp + 30 lines
  I0  = 12                     ; start of image area
  I1  = I0 + 1024/BIN -1       ; end of image area

  M = MEAN (Z[I0:I1,R0:*])    ; compute raw image mean
  S = STDEV(Z[I0:I1,R0:*])    ; compute raw image noise
  N = NOISE(Z,INFO,'image',1) ; compute pattern,drift free noise

  M0 = MEAN (Z0[I0:I1,R0:*])    ; compute stage-0 image mean
  S0 = STDEV(Z0[I0:I1,R0:*])    ; compute stage-0 image noise

  M1 = MEAN (Z1[I0:I1,R0:*])    ; compute stage-1 image mean
  S1 = STDEV(Z1[I0:I1,R0:*])    ; compute stage-1 image noise

  M2 = MEAN (Z2[I0:I1,R0:*])    ; compute stage-2 image mean
  S2 = STDEV(Z2[I0:I1,R0:*])    ; compute stage-2 image noise

  IF Q THEN BEGIN
    PRINT,'Raw  :',M,S,N
    PRINT,'Rev  :',M0,S0
    PRINT,'Fixed:',M1,S1
    PRINT,'Drift:',M2,S2
    PRINT,'*****'
  ENDIF

  PRINT,'Image File Name            --    Raw Image Mean, StDev, Noise  ...  ',$
         'Mean & StDev for each Z Step'
  F = '$(A23,"    --    ",F6.1,2X,F5.1,2X,F4.1,3("  ...  ",F6.1,2X,F5.1))'
  PRINT,BASENAME, M,S,N, M0,S0, M1,S1, M2,S2, FORMAT=F

  IF Q THEN PRINT,'*****'

ENDFOR  ; JF file number

END
; ----------------------------------------------------------------------------