@awk -fsdbf.awk detect\%1.ast > %TMP%dc.bat
@call %TMP%dc %TMP%asic.cod
@v %TMP%asic.cod

