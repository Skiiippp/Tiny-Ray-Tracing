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

typedef struct vec3{
    int x;
    int y;
    int z;
} vec3;

//Image params

const int image_width = 80;
const int image_height = 60;

volatile int * const VG_ADDR = (int *)0x11000120;
volatile int * const VG_COLOR = (int *)0x11000140;

//Camera params

vec3 camera          = {0    , -100    , 0};

vec3 ray_dir         = { 0 ,    0,     , 0};

//Sphere params

vec3 sphere_position = {0    , 0    , 0};

int sphere_pow_2 = 5;

int divide(int a, int b, int fraction_bits);

// draws a single pixel
static void write_to_vga(int x, int y, vec3 color) {
    //int store_color = ((color.x>>5)<<5) | ((color.y>>5)<<2) | (color.z>>6);
    //*VG_COLOR = 0;//store_color;  // store into the color IO register, which triggers 
    								
	// 8-bit color, RRR,GGG,BB, so R & G must be scaled by 32, B by 64
	char r = divide(color.x, 32, 0) << 5;
	char g = divide(color.y, 32, 0) << 2;
	char b = divide(color.z, 64, 0);
	char color_out = r | g | b;
	
    //ABOUT TO WRITE
	// For debugging purposes
	if(color_out != 0x9b){	// Not background
		color_out = 0xe0;
	}

	*VG_ADDR = (y << 7) | x;  		// store into the address IO register
	*VG_COLOR = color_out;

	
}


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
    Computes the vector result of a + b
*/
static vec3 vec3_sum(vec3 a, vec3 b){
    vec3 c;

    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;

    return c;
}

/*
    Computes the vector result of a - b
*/
static vec3 vec3_diff(vec3 a, vec3 b){
    vec3 c;

    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;

    return c;
}

/*
    Computes the dot product of two vec3 structs
*/
static int vec3_dot(vec3 a, vec3 b){
    int c = 0;

    c += mult(a.x, b.x);
    c += mult(a.y, b.y);
    c += mult(a.z, b.z);

    return c;
}


static vec3 scale_vec3(vec3 a, int scale){

    
    vec3 ret;
    ret.x = mult(a.x, scale);
    ret.y = mult(a.y, scale);

    //vec3 balls = {0,0,0};
    //return balls;

    ret.z = mult(a.z, scale);


    return ret;
}


static vec3 scale_vec3_test(vec3 a, int scale){

    
    vec3 ret;
    ret.x = mult(-1, scale);
    ret.y = mult(80, scale);

    //vec3 balls = {0,0,0};
    //return balls;

    ret.z = mult(27, scale);


    return ret;
}


/*
 * Fraction bits are the number of result LSB's denoted as fractional
 * Returns (a/b)<<fraction_bits
 * Shifting allows for more precision
 */
int divide(int a, int b, int fraction_bits){

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

int hit_sphere(vec3 center, int radius){
    vec3 oc          = vec3_diff(camera, center);
    int a            = vec3_dot(ray_dir, ray_dir);
    int half_b       = vec3_dot(oc, ray_dir);
    int c            = vec3_dot(oc, oc) - mult(radius, radius);
    int discriminant = mult(half_b, half_b) - mult(a, c);

    if(discriminant > 0){ //hit the sphere
        int shift = 10;

        int t = divide(-half_b - square_root(discriminant), a, shift);
        return t;
    }else{ //did not hit sphere
        return -1;
    }
}

vec3 ray_color(){
    int t = hit_sphere(sphere_position, 1<<sphere_pow_2);

    if(t <= 0){
        vec3 background = {135, 206, 235};
        return background;
    } 
    vec3 circle = {255, 0, 0};
    
    int shift = 10;

    vec3 scaled_camera = scale_vec3(camera, 1<<shift);

    vec3 scaled_dir = scale_vec3(ray_dir, t);    // ERROR OCCURS HERE!!!! -- likely accessing nested structs with r.d
    return circle;

    vec3 r_at_t = vec3_sum(scaled_camera, scaled_dir); 

    vec3 n = vec3_diff(r_at_t, scale_vec3(sphere_position, 1<<shift));

    int scaling_factor = sphere_pow_2 + shift;

    int cr = (n.x + (1<<scaling_factor))>>(scaling_factor - 7);
    int cg = (n.y + (1<<scaling_factor))>>(scaling_factor - 7);
    int cb = (n.z + (1<<scaling_factor))>>(scaling_factor - 7);

    vec3 color = {(cr>>5)<<5, (cg>>5)<<5, (cb>>6)<<6};

    return color;
    
}



void main() {

    int sphere_r = 1<<sphere_pow_2;

    //Render

	//printf("P3\n80 60\n255\n");
    int movement = 50;
    while(1){
        for(int j = image_height - 1; j >= 0; --j){   //run for each column
            for(int i = 0; i < image_width; ++i){     //run for each row
                int horiz   = (-(image_width>>1) + i);
                int vert    = (-(image_height>>1) + j);
                int forward = (image_width);

                //Ray Dir is direction coming out of camera position
                ray_dir = {horiz, forward, vert};
                
                vec3 color = ray_color();

                write_to_vga(i, image_height - 1 - j, color);

                //if(color.x != 135)
                //printf("%d %d %d\n", color.x, color.y, color.z);

                /*
                if(color.x == 135 && color.y == 206 && color.z == 235){
                    printf(".");
                } else {
                    printf("*");
                }*/
            
            }
            //printf("\n");
        }
        int* cameraY = &(camera.y);
        if(-150 < *cameraY || -500 > *cameraY){
            movement = -1 * movement;
            //printf("balls\n"); tf you comment this for?
        }
        *cameraY += movement;

        //while(1);
        
    }

}