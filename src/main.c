/*
 *  ------------------------------------------------------------------------  *
 *  Engineer: James Gruber, Daniel Brathwaite
 *  Design Name: OTTER Raytracer
 *
 *  Description: A raytracing program designed to output graphics to VGA from the OTTER MCU
 *
 *  Additional Comments:
 *
 *  a. Input fractional_bits to functions specifies how many LSB's
 *      are denoted as fractional. Increasing this value will gain precision
 *      at the loss of number bandwidth.
 *
 *  b. Coordinate system used is limited to
 *      Positive x is right
 *      Positive y is forward
 *      Positive z is up
 *  ------------------------------------------------------------------------  *
 */

//#include <stdio.h>

//Image params

const int image_width = 80;
const int image_height = 60;

volatile int * const VG_ADDR = (int *)0x11000120;
volatile int * const VG_COLOR = (int *)0x11000140;

//Camera params

int camx = 0;
int camy = -200;
int camz = 0;
//vec3 camera          = {0    ,-100    ,0};

//vec3 ray_dir         = {0    ,0       ,0};

//Sphere params

//vec3 sphere_position = {0    , 0    , 0};

//int sphere_pow_2 = 5;


/*
    Performs multiplication on two signed values using addition
*/
static int mult(int a, int b){
    if(a == 0 || b == 0){return 0;}

    if(a < 0 && b < 0){
        a = -a;
        b = -b;
    }else if(b < 0){
        int temp = a;
        a = b;
        b = temp;
    }

    /*
        From here on can assume either:
        1. Both numbers are positive
        2. a is negative, and b is positive
    */

    int result = 0;

    while(b > 0){
        if((b&1) == 1){
            result = result + a;
        }
        b = b >> 1;
        a = a << 1;
    }

    return result;
}
/*
 * Fraction bits are the number of result LSB's denoted as fractional
 * Returns (a/b)<<fraction_bits
 * Shifting allows for more precision
 */
static int divide(int a, int b, int fraction_bits){

    if (a < 0 && b < 0){
        int temp = a;
        a = b;
        b = temp;
    }

    int res = 0;
    int negative = 0;

    if (a < 0){
        a = -a;
        negative = 1;
    }else if (b < 0){
        b = -b;
        negative = 1;
    }

    a = a<<fraction_bits;

    int add = 32768;

    while(add > 0) {
        if (mult(b, res+add) <= a) {
            res = res + add;
        }
        add>>=1;
    }

    if(negative == 1){res = -res;}

    return res;
}

/*
 * Assumes a is a positive value
 */
static int square_root(int a){
    int res = 0;

    int add = 16384;
    while(add > 0) {
        if (mult(res + add, res + add) <= a) {
            res = res + add;
        }
        add>>=1;
    }

    return res;
}


void main() {

    int sphere_pow_2 = 5;
    int radius = 1<<sphere_pow_2;

    //Render

    //printf("P3\n80 60\n255\n");
    int movement = 50;
    while(1){
        //printf("P3\n80 60\n255\n");
        for(int j = image_height - 1; j >= 0; --j){   //run for each column
            for(int i = 0; i < image_width; ++i) {     //run for each row
                int horiz = (-(image_width >> 1) + i);
                int vert = (-(image_height >> 1) + j);
                int forward = (image_width);

                //Ray Dir is direction coming out of camera position
                int rdx = horiz;
                int rdy = forward;
                int rdz = vert;

                int ocx = camx - 0;
                int ocy = camy - 0;
                int ocz = camz - 0;

                int a = 0;
                a += mult(rdx, rdx);
                a += mult(rdy, rdy);
                a += mult(rdz, rdz);

                int half_b = 0;
                half_b += mult(ocx, rdx);
                half_b += mult(ocy, rdy);
                half_b += mult(ocz, rdz);

                int radsq = mult(radius, radius);

                int dotoc = 0;
                dotoc += mult(ocx, ocx);
                dotoc += mult(ocy, ocy);
                dotoc += mult(ocz, ocz);

                int c = dotoc - radsq;

                //---------------------------------------------------------------------
                int discriminant = mult(half_b, half_b) - mult(a, c);

                int t;

                if (discriminant > 0) { //hit the sphere
                    int shift = 10;

                    t = divide(-half_b - square_root(discriminant), a, shift);
                } else { //did not hit sphere
                    t = -1;
                }

                int x = i;
                int y = image_height - 1 - j;

                if (t <= 0) {
                    *VG_ADDR = (y << 7) | x;  		// store into the address IO register
                    *VG_COLOR = 155;         // store color val POOP
                    //printf("135 206 235\n");
                }else{

                    int shift = 10;

                    int sc_cam_x = 0;
                    int sc_cam_y = mult(camy, 1024);
                    int sc_cam_z = 0;

                    int sc_dir_x = mult(rdx, t);
                    int sc_dir_y = mult(rdy, t);
                    int sc_dir_z = mult(rdz, t);

                    int r_at_t_x = sc_cam_x + sc_dir_x;
                    int r_at_t_y = sc_cam_y + sc_dir_y;
                    int r_at_t_z = sc_cam_z + sc_dir_z;

                    int sc_sp_x = 0;
                    int sc_sp_y = 0;
                    int sc_sp_z = 0;

                    int n_x = r_at_t_x - 0;
                    int n_y = r_at_t_y - sc_sp_y;
                    int n_z = r_at_t_z - 0;

                    int scaling_factor = sphere_pow_2 + shift;

                    // ERROR OCCURS HERE!!!! -- likely cr, cg, cb
                    int cr = (n_x + (1 << scaling_factor)) >> (scaling_factor - 7);
                    int cg = (n_z + (1 << scaling_factor)) >> (scaling_factor - 7);
                    int cb = (n_y + (1 << scaling_factor)) >> (scaling_factor - 7);

                    int r = (cr>>5)<<5;
                    int g = (cg>>5)<<2;
                    int b = (cb>>6);
                    int write_color = r | g | b;

                    //printf("%d %d %d\n", r, g<<3, b<<6);


                    *VG_ADDR = (y << 7) | x;  		// store into the address IO register
                    *VG_COLOR = write_color;         // store color val POOP
                    //---------------------------------------------------------------------
                }

            }
            //printf("\n");
        }
        int* cameraY = &(camy);
        if(-150 < *cameraY || -500 > *cameraY){
            movement = -movement;
            //printf("balls\n"); tf you comment this for?
        }
        *cameraY += movement;

        //while(1);
        //break;

    }

}