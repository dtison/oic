
void fdct (xfrm, xin, wValuesPerRow) 
int FAR   *xfrm;
unsigned char FAR *xin;
WORD      wValuesPerRow;
{
    int   i, j, p;


    #define   ilong int  

    ilong  a,b,c,d,e,f,g,h; 

    ilong      k;
    ilong      k4 [8];

    int       k00, k01, k02, k03, k04, k05, k06, k07, k10, k11, k12, 
              k13, k14, k15, k16, k17, k20, k21, k22, k23, k24, k25, 
              k26, k27, k34, k35, k36, k37;

//  Divided all values by 16

    ilong      c1p4  = 23;
    ilong      c1p8  = 30; 
    ilong      s1p8  = 12;
    ilong      c1p16 = 31;
    ilong      s1p16 =  6;
    ilong      c3p16 = 27;
    ilong      s3p16 = 18;
    ilong      norm  = 32;


    /* 1-dimensional transform on rows */

    p = 0;
    for (i = 0; i < 8; ++i) 
    {

        k00 = xin [p];
        k01 = xin [p + 1];
        k02 = xin [p + 2];
        k03 = xin [p + 3];
        k04 = xin [p + 4];
        k05 = xin [p + 5];
        k06 = xin [p + 6];
        k07 = xin [p + 7];

        k10 = (k00 + k07) << 2;
        k11 = (k01 + k06) << 2;
        k12 = (k02 + k05) << 2;
        k13 = (k03 + k04) << 2;
        k14 = (k03 - k04) << 2;
        k15 = (k02 - k05) << 2;
        k16 = (k01 - k06) << 2;
        k17 = (k00 - k07) << 2;

        k20 = k10 + k13;
        k21 = k11 + k12;
        k22 = k11 - k12;
        k23 = k10 - k13;
        k24 = k14;

        /*  k25  k26 ok  now  */


        a   = (ilong) c1p4 * (ilong) (k16 - k15);

        k25 = (int) (a >> 5);

        a   = (ilong) c1p4 * (ilong) ((k16 + k15) >> 1);

        k26 = (int) (a >> 4);


        k27 = k17;



        /*  Fix for "xfrm [p]"      */

        a = (ilong) ((k20 + k21 - 4096) >> 3);
        a *= c1p4;


        xfrm [p] = (int) (a >> 2);



        a = (ilong) ((k20 - k21) >> 2);
        a *= c1p4;




        xfrm [p + 4] = (int) (a >> 3);





        /*  Fix for "xfrm [p + 2]"    */

        a  = (ilong) ((ilong) c1p8 * (ilong) (k23 >> 2));
        a += (ilong) ((ilong) s1p8 * (ilong) (k22 >> 2));


        xfrm [p + 2] = (int) (a >> 3);  // (5 - shift amt above)



        /*  Fix for "xfrm [p + 6]"    */

        a  = (ilong) ((ilong) s1p8 * (ilong) (k23 >> 2));
        a -= (ilong) ((ilong) c1p8 * (ilong) (k22 >> 2));


        xfrm [p + 6] = (int) (a >> 3);





        /*  k34 .. k37 ok  */

        k34 = k24 + k25;
        k35 = k24 - k25;
        k36 = k27 - k26;
        k37 = k27 + k26;









        /*  Fix for "xfrm [p + 1]"    */

        a  = (ilong) ((ilong) c1p16 * (ilong) (k37 >> 2));
        a += (ilong) ((ilong) s1p16 * (ilong) (k34 >> 2));

        if (a > 32767)
            printf ("xfrm [p + 1] (1) gets greater: %ld\n", a);

        if (a < -32767)
            printf ("xfrm [p + 1] (1) gets less than: %ld \n",a);     

        xfrm [p + 1] = (int) (a >> 3);
  

        /*  Fix for "xfrm [p + 5]"    */

        a  = (ilong) ((ilong) s3p16 * (ilong) (k36 >> 2));
        a += (ilong) ((ilong) c3p16 * (ilong) (k35 >> 2));




        xfrm [p + 5] = (int) (a >> 3);





        /*  Fix for "xfrm [p + 3]"    */

        a  = (ilong) ((ilong) c3p16 * (ilong) (k36 >> 2));
        a -= (ilong) ((ilong) s3p16 * (ilong) (k35 >> 2));



        xfrm [p + 3] = (int) (a >> 3);


        /*  Fix for "xfrm [p + 7]"    */

        a  = (ilong) ((ilong) s1p16 * (ilong) (k37 >> 1));
        a -= (ilong) ((ilong) c3p16 * (ilong) (k34 >> 1));


        xfrm [p + 7] = (int) (a >> 4);



        p += wValuesPerRow;
    }

    /* 1-dimensional transform on columns */

    for (j = 0; j < 8; ++j) 
    {

        k00 = xfrm [j];
        k01 = xfrm [j + (wValuesPerRow)];
        k02 = xfrm [j + (wValuesPerRow * 2)];
        k03 = xfrm [j + (wValuesPerRow * 3)];
        k04 = xfrm [j + (wValuesPerRow * 4)];
        k05 = xfrm [j + (wValuesPerRow * 5)];
        k06 = xfrm [j + (wValuesPerRow * 6)];
        k07 = xfrm [j + (wValuesPerRow * 7)];


        k10 = (k00 + k07) / 2;
        k11 = (k01 + k06) / 2;
        k12 = (k02 + k05) / 2;
        k13 = (k03 + k04) / 2;
        k14 = (k03 - k04) / 2;
        k15 = (k02 - k05) / 2;
        k16 = (k01 - k06) / 2;
        k17 = (k00 - k07) / 2;

        k20 = k10 + k13;
        k21 = k11 + k12;
        k22 = k11 - k12;
        k23 = k10 - k13;
        k24 = k14;

        a   = (ilong) c1p4 * (ilong) ((k16 - k15) >> 1);


        k25 = (int) (a >> 4);


        a   = (ilong) c1p4 * (ilong) ((k16 + k15) >> 2);


        k26 = (int) (a >> 3);


        k27 = k17;





        /*  Fix for "k4 [0]"      */

        a = (ilong) ((k20 + k21) >> 4);
        a *= c1p4;


        k4 [0] = (int) (a >> 1);


        /*  Fix for "k4 [4]"      */

        a = (ilong) ((k20 - k21) >> 2);
        a *= c1p4;


        k4 [4] = (int) (a >> 3);





        /*  Fix for "k4 [2]"      */

        a = (ilong) ((c1p8 * (k23 >> 2)) + (s1p8 * (k22 >> 2)));




        k4 [2] = (int) (a >> 3);


        /*  Fix for "k4 [6]"      */

        a = (ilong) ((s1p8 * (k23 >> 2)) - (c1p8 * (k22 >> 2)));




        k4 [6] = (int) (a >> 3);





        k34 = k24 + k25;
        k35 = k24 - k25;
        k36 = k27 - k26;
        k37 = k27 + k26;


        /*  Fix for "k4 [1]"      */

        a = (ilong) ((c1p16 * (k37 >> 3)) + (s1p16 * (k34 >> 3)));



        k4 [1] = (int) (a >> 2); 



        /*  Fix for "k4 [5]"      */

        a = (ilong) ((s3p16 * (k36 >> 2)) + (c3p16 * (k35 >> 2)));



        k4 [5] = (int) (a >> 3);






        /*  Fix for "k4 [3]"      */

        a = (ilong) ((c3p16 * (k36 >> 2)) - (s3p16 * (k35 >> 2)));



        k4 [3] = (int) (a >> 3);



        /*  Fix for "k4 [7]"      */

        a = (ilong) ((s1p16 * (k37 >> 1)) - (c1p16 * (k34 >> 1)));



        k4 [7] = (int) (a >> 4);





        p = j;
        for (i = 0; i < 8; ++i) 
        {
            if (k4[i] < 0) 
            {
                k = (k4[i] - 4) / 8;
                if (k < -1023)
                    k = -1023;
            } 
            else 
            {
                k = (k4[i] + 4) / 8;
                if (k > 1023)
                    k = 1023;
            }
            xfrm[p] = (int) k;
            p += wValuesPerRow;
        }
    }


}


