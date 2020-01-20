//reads in bmp280 from chip calibration parameters
static void bmp280_cal () {
    i2c_read(0x76, 0x88, regdata, 24);
    digT1 = regdata[0] + 256 * regdata[1];
    digT2 = regdata[2] + 256 * regdata[3];   if (digT2 > 32767)digT2 = digT2 - 65536;
    digT3 = regdata[4] + 256 * regdata[5];   if (digT3 > 32767)digT3 = digT3 - 65536;
    digP1 = regdata[6] + 256 * regdata[7];
    digP2 = regdata[8] + 256 * regdata[9];   if (digP2 > 32767)digP2 = digP2 - 65536;
    digP3 = regdata[10] + 256 * regdata[11]; if (digP3 > 32767)digP3 = digP3 - 65536;
    digP4 = regdata[12] + 256 * regdata[13]; if (digP4 > 32767)digP4 = digP4 - 65536;
    digP5 = regdata[14] + 256 * regdata[15]; if (digP5 > 32767)digP5 = digP5 - 65536;
    digP6 = regdata[16] + 256 * regdata[17]; if (digP6 > 32767)digP6 = digP6 - 65536;
    digP7 = regdata[18] + 256 * regdata[19]; if (digP7 > 32767)digP7 = digP7 - 65536;
    digP8 = regdata[20] + 256 * regdata[21]; if (digP8 > 32767)digP8 = digP8 - 65536;
    digP9 = regdata[22] + 256 * regdata[23]; if (digP9 > 32767)digP9 = digP9 - 65536;
}

//called after tcp_server_task to collect and return data
static void bmp280_getdata () {
       i2c_write (0x76, 0xf4, 0x5f);
       i2c_write (0x76, 0xf5, 0x1c);
       i2c_read(0x76, 0xf7, regdata, 6);
       int adcp = 4096 * regdata[0] + 16*regdata[1] + regdata[2]/16;
       int adct = 4096 * regdata[3] + 16*regdata[4] + regdata[5]/16;
       //calculate temperature
       double var1, var2, ctemp;
       int tfine;
       var1 = (((double)adct)/16384.0 - ((double)digT1)/1024.0)*((double)digT2);
       var2 = ((((double)adct)/131072.0 - ((double)digT1)/8192.0)*
               (((double)adct)/131072.0 - ((double)digT1)/8192.0)*(double)digT3);
       tfine = (int) var1+var2;
       ctemp = (float) tfine/5120.0;
       //calculate pressure
       double p, pres;
       var1 = ((double)tfine/2.0) - 64000.0;
       var2 = var1*var1*((double)digP6)/32768.0;
       var2 = var2 + var1*((double)digP5)*2.0;
       var2 = var2/4 + ((double)digP4)*65536.0;
       var1 = (((double)digP3)*var1*var1/524288.0 + ((double)digP2)*var1)/524288.0;
       var1 = (1 + var1/32768.0)*((double)digP1);
       p = 1048576.0-((double)adcp);
       p = (p - var2/4096.0)*6250.0/var1;
       var1 = ((double)digP9)*p*p/2147483648.0;
       var2 = p*((double)digP8)/32768.0;
       pres = (p + (var1 + var2 + ((double)digP7))/16.0)/100;

       printf ("temp= %6d  ", (int)(100*(32 + 1.8 * ctemp)));
       printf ("pres= %8d\n", (int)(100*pres));

       //construct outstring
       char tmp[100];
       ctemp = 1000 * (32 + 1.8 * ctemp);
       snprintf( outstr, sizeof tmp,"%7d %5d.%03d\t", cnt, (int)(ctemp/1000), ((int)(ctemp)) % 1000);
       pres = 1000 * pres;
       snprintf( tmp, sizeof tmp,"%5d.%03d", (int)(pres/1000),((int)(pres))%1000); strcat (outstr, tmp);  
       strcat ( outstr, "\n\0");
       ++cnt;
}
