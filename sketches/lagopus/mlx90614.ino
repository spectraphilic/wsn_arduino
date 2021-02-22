#include <Adafruit_MLX90614.h>


Adafruit_MLX90614 mlx;
bool mlx_ok;
double mlx_o, mlx_a;


void mlx_init()
{
    mlx_ok = mlx.begin();
    if (mlx_ok) {
        PRINTLN("MLX90614  OK");
    } else {
        PRINTLN("MLX90614  ERROR");
    }
}

void mlx_measure()
{
    if (mlx_ok) {
        sendResponse("0012"); // 2 values in 1 second
        mlx_o = mlx.readObjectTempC();
        mlx_a = mlx.readAmbientTempC();
        sensor = MLX90614;
    } else {
        sendResponse("0000");
    }
}

void mlx_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f", mlx_o, mlx_a);
}
