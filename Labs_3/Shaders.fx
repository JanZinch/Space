//--------------------------------------------------------------------------------------
// File: Shaders.fx
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Переменные константных буферов
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register( t0 );        // Буфер текстуры
SamplerState samLinear : register( s0 );     // Буфер образца


// Буфер с информацией о матрицах
cbuffer ConstantBufferMatrixes : register( b0 )
{
    matrix World;            	// Матрица мира
    matrix View;             	// Матрица вида
    matrix Projection;       	// Матрица проекции
};

 
// Буфер с информацией о свете
cbuffer ConstantBufferLight : register( b1 )
{
    float4 vLightDir[2];    	// Направление источника света
    float4 vLightColor[2];  	// Цвет источника света
    float4 vOutputColor;    	// Активный цвет
	float4 vTime;
};



//--------------------------------------------------------------------------------------
// Структуры
//--------------------------------------------------------------------------------------


struct VS_INPUT                   // Входящие данные вершинного шейдера
{
    float4 Pos : POSITION;        // Позиция по X, Y, Z
    float2 Tex : TEXCOORD0;       // Координаты текстуры по tu, tv
    float3 Norm : NORMAL;         // Нормаль по X, Y, Z
};

 
struct PS_INPUT                   // Входящие данные пиксельного шейдера
{
    float4 Pos : SV_POSITION;     // Позиция пикселя в проекции (экранная)
    float2 Tex : TEXCOORD0;       // Координаты текстуры по tu, tv
    float3 Norm : TEXCOORD1;      // Относительная нормаль пикселя по tu, tv
};

struct PS_INPUT4
{
    float4 Pos : SV_POSITION;
    float4 R : RWORLD;			// Позиция относительно сцены (позиция в виртуальном пространстве)
    float4 R0 : RCAMERA;		// Позиция относительно камеры
	float3 Norm : NORMAL;		// Нормаль к поверхности
				
};

//--------------------------------------------------------------------------------------
// Вершинный шейдер
//--------------------------------------------------------------------------------------

PS_INPUT VS( VS_INPUT input ) 
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul( input.Norm, World );
	output.Pos.y += 1;
    output.Tex = input.Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Пиксельный шейдер для объектов
//--------------------------------------------------------------------------------------

float4 PS( PS_INPUT input) : SV_Target
{

    float4 finalColor = 0;   				// чёрный цвет без освещения
											// складываем освещенность пикселя от всех источников света

    for(int i=0; i<2; i++){

        finalColor += saturate( dot( (float3)vLightDir[i], input.Norm) * vLightColor[i] );

    }

    finalColor *= txDiffuse.Sample( samLinear, input.Tex );
    finalColor.a = 1.0f;
    return finalColor;
}

//--------------------------------------------------------------------------------------
// Пиксельный шейдер для фона
//--------------------------------------------------------------------------------------

float4 PSBkg( PS_INPUT input) : SV_TARGET
{

    float4 finalColor = txDiffuse.Sample( samLinear, input.Tex );
    finalColor.a = 1.0f;
    return finalColor;
}

 
//--------------------------------------------------------------------------------------
// Пиксельный шейдер для источников света
//--------------------------------------------------------------------------------------

float4 PSSolid( PS_INPUT input) : SV_Target
{
    return vOutputColor;
}


//--------------------------------------------------------------------------------------
// Пиксельный шейдер для создания спецэффектов
//--------------------------------------------------------------------------------------

float4 PSEffect( PS_INPUT input) : SV_TARGET
{	
	float angle = atan2(input.Tex.x, input.Tex.y);
	float radius = dot(input.Tex, input.Tex);

	return float4(sin(input.Tex.x + vTime.x), cos(input.Tex.x + vTime.x), sin(input.Tex.y + vTime.x), 0.0f);
}


