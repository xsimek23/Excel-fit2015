/**
* @file gen.cpp
* 
* N-Body Simuluation
* Generate input file for particles simulation
* 
* Format:
*       pos_x pos_y pos_z vel_x vel_y vel_z weight
*/

#include <cstdlib>
#include <cstdio>
#include <cfloat>
#include <ctime>

float randf()
{
	float a = (float)rand() / (float)RAND_MAX;

	if (a == 0.0f)
		a = FLT_MIN;

	return a;
}

int main(int argc, char **argv)
{
    int i;
    int N;
    FILE *fp;

    srand(time(NULL));

    if (argc != 3)
    {
        printf("Usage: gen N <output>\n");
        exit(1);
    }

    N = atoi(argv[1]);

    // print parameters
    printf("N: %d\n", N);

    // write particles to file
    fp = fopen(argv[2], "w");
    if (fp == NULL)
    {
        printf("Can't open file %s!\n", argv[2]);
        exit(1);
    }

    for (i = 0; i < N; i++)
    {
        fprintf(fp, "%10.4f %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f \n",
            randf() * 100.0f,
            randf() * 100.0f,
            randf() * 100.0f,
            randf() * 4.0 - 2.0f,
            randf() * 4.0 - 2.0f,
            randf() * 4.0 - 2.0f,
            randf() * 25000000.0f);
    }

    fclose(fp);

    return 0;
}
