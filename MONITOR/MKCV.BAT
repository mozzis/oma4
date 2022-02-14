cl -AL -Zp -G2s -Oax cvtsrec.c \code\dos16m\msc60l.lib /link /NOE
\code\dos16m\makepm cvtsrec.exe -L -DPMI -Q
\code\dos16m\splice cvtsrec.exe cvtsrec.exp \code\dos16m\loader.exe

