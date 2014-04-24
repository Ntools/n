#ifdef MSDOS
#define             JDOS            0                       /* PS/55 “ú–{ŒêDOS */
#define            PCDOS           1                       /* PC DOS */
#define             DOSVJ           2                       /* DOS/V “ú–{ŒêÓ°ÄÞ */
#define           DOSVE           3                       /* DOS/V ‰pŒêÓ°ÄÞ */
#define             AXJ                     4                       /* AX “ú–{ŒêÓ°ÄÞ */
#define              AXE                     5                       /* AX ‰pŒêÓ°ÄÞ */
#define                J3100J          6                       /* J3100 “ú–{ŒêÓ°ÄÞ */
#define           J3100E          7                       /* J3100 ‰pŒêÓ°ÄÞ */
#define             DRDOSJ          8                       /* DR-DOS “ú–{ŒêÓ°ÄÞ */
#define          DRDOSE          9                       /* DR-DOS ‰pŒêÓ°ÄÞ */
#define            PC9801          10                      /* PC-9801 */
#define            PC9801H         11                      /* PC-9801 Hireso */

# if (defined __386__)
#  include    <dos.h>

extern union REGS ir, or;

unsigned short getvmd(void);

isMachine(void)
{
# if 0
int machine = 12;
int m;

       if((getvmd() & 0xff00) == 0x0f00) {
             machine = PC9801;
               if (*((unsigned char __far *) MK_FP(0, 0x501)) & 8)             /* Hireso flag */
                       ++machine;                                                                              /* PC9801 Ê²Ú¿Þ */
      }
       else machine = DOSVJ;
   return(machine);
#endif
  return(DOSVJ);
}
# else

unsigned getvmd(void)
{
     __asm {
         mov     ah, 0x0f
                int     0x10
    }
}

drdos(void)
{
  __asm {
         stc
             mov     ax, 0x4412
              int     0x21
            mov     ax, 1
           jnc     drd01
           dec     al
      drd01:
  }
}

dbcschk(int code)
{
    __asm {
         push    ds
              push    si
              mov     ax, 0x6300
              int     0x21
            mov     ax, code
                jc      dbc01
           cmp     word ptr ds:[si], 0
             jz      dbc01
           dec     ax
      dbc01:
          pop     si
              pop     ds
      }
}

ax(void)
{
     __asm {
         mov     ax, 0x5001
              int     0x10
            xor     cx, cx
          and     al, al
          jnz     nax
             mov     cx, AXJ
         cmp     bx,     1
               jz      nax
             mov     cx,     AXE
     nax:
            mov     ax, cx
  }
}

dosv(void)
{
   __asm {
         mov     ax, 0x4900
              int     0x15
            mov     ax, 0
           jc      dv01
            and     bl, bl
          jnz     dv01
            inc     al
      dv01:
   }
}

chkint60(void)
{
       __asm {
         mov     dx, 0x8148              /* Shift JIS code */
            mov     ah, 2
           int     0x60
            xor     ax, ax
          cmp     dx, 0x2129
              jnz     ck801
           inc     ax
      ck801:
  }
}

isMachine(void)
{
int machine = 12;
int m;

       if((getvmd() & 0xff00) == 0x0f00) {
             machine = PC9801;
               if (*((unsigned char __far *)0x00000501) & 8)           /* Hireso flag */
                       ++machine;                                                                              /* PC9801 Ê²Ú¿Þ */
      }
       else if(drdos()) machine = dbcschk(DRDOSE);
     else if((*((unsigned __far *)0x00000180) + *((unsigned __far *)0x00000182)) && chkint60()) {
            machine = J3100J;
               if(*((unsigned char __far *)0x000004d0) == 0) ++machine;
        }
       else if((m = ax())) machine = m;
        else if(dosv()) machine = dbcschk(DOSVE);
       else machine = (*((unsigned __far *)0x000001f4) + *((unsigned __far *)0x000001f6))? JDOS: PCDOS;
        return(machine);
}
# endif
#endif
