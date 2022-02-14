# awk script to make debugging batch file for ASIC
# useage: awk -f dbf.awk <asic listing filename> ><batch file name>
#         <batch file name> <list file name>


BEGIN                   { printf("\n@echo off")
                          printf("\nport 301 0")
                        }
$4 == "FCSIFM"          { printf("\necho. > %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 240 >> %%1", $1)
                        }
$4 == "DCSIFM"          { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 26 -d >> %%1", $1)
                        }
$4 == "FCSEFM"          { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 240 >> %%1", $1)
                        }
$4 == "DCSEFM"          { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 8 -d >> %%1", $1)
                        }
$4 == "FCSUR"           { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 40 >> %%1", $1)
                        }
$4 == "RSSMXU"          { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 138 >> %%1", $1)
                        }
$4 == "DCSRATR"         { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 64 -d >> %%1", $1)
                        }
$4 == "FCCUR"           { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 48 >> %%1", $1)
                        }
$4 == "RCSMXU"          { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 138 >> %%1", $1)
                        }
$4 == "RSLICES"         { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 4 >> %%1", $1)
                        }
$4 == "RTRACKS"         { printf("\necho. >> %%1")
                          printf("\necho %s\necho %s >> %%1", $4, $4)
                          printf("\nsmdis 80%s 4 >> %%1", $1)
                        }
END                     {
                        printf("\nport 301 39")
                        }
